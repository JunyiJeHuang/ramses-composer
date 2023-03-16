/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "common_widgets/MeshAssetImportDialog.h"
#include "style/Icons.h"
#include "user_types/Animation.h"
#include "user_types/Mesh.h"
#include "user_types/MeshNode.h"
#include "user_types/Node.h"
#include "user_types/Skin.h"

#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

namespace raco::common_widgets {

MeshAssetImportDialog::MeshAssetImportDialog(raco::core::MeshScenegraph& sceneGraph, int projectFeatureLevel, QWidget* parent)
	: sceneGraph_{sceneGraph},
	  nodeTreeList_{sceneGraph_.nodes.size()},
	  meshTreeList_{sceneGraph_.meshes.size()},
	  animTreeList_{sceneGraph_.animations.size()} {
	setWindowTitle("Import External Assets");

	widget_ = new QTreeWidget(this);
	widget_->setHeaderItem(new QTreeWidgetItem({"Name", "Type"}));
	widget_->setColumnWidth(0, width() / 3);
	widget_->setAlternatingRowColors(true);
	connect(widget_, &QTreeWidget::itemChanged, [this](QTreeWidgetItem* item) {
		widget_->blockSignals(true);

		if (item->checkState(0) == Qt::Unchecked) {
			iterateThroughChildren(item, [](auto* item) {
				item->setCheckState(0, Qt::Unchecked);
			});
		} else if (item->checkState(0) == Qt::Checked) {
			for (auto parent = item->parent(); parent; parent = parent->parent()) {
				// Skip top-level items.
				if (parent->parent()) {
					parent->setCheckState(0, Qt::Checked);
				}
			}

			iterateThroughChildren(item, [](auto* item) {
				item->setCheckState(0, Qt::Checked);
			});
		}

		widget_->blockSignals(false);
	});

	selectAllButton_ = new QPushButton("Check All", this);
	connect(selectAllButton_, &QPushButton::clicked, [this]() {
		widget_->blockSignals(true);
		checkAll(Qt::Checked);
		widget_->blockSignals(false);
	});

	deselectAllButton_ = new QPushButton("Uncheck All", this);
	connect(deselectAllButton_, &QPushButton::clicked, [this]() {
		widget_->blockSignals(true);
		checkAll(Qt::Unchecked);
		widget_->blockSignals(false);
	});

	massSelectButtonLayout_ = new QHBoxLayout(nullptr);
	massSelectButtonLayout_->addWidget(selectAllButton_);
	massSelectButtonLayout_->addWidget(deselectAllButton_);
	massSelectButtonLayout_->addStretch();

//	yAxesUpButton_ = new QRadioButton(this);
//	zAxesUpButton_ = new QRadioButton(this);
//	yAxesUpButton_->setChecked(true);
//    QButtonGroup *axesButtonGroup = new QButtonGroup(this);
//    axesButtonGroup->addButton(yAxesUpButton_);
//    axesButtonGroup->addButton(zAxesUpButton_);
//    axesDirectionButtonLayout_ = new QHBoxLayout(nullptr);
//    axesDirectionButtonLayout_->addWidget(new QLabel("Assets Axes Direction:"));
//    axesDirectionButtonLayout_->addStretch();
//	axesDirectionButtonLayout_->addWidget(yAxesUpButton_);
//    axesDirectionButtonLayout_->addWidget(new QLabel("+Y up, +Z forward"));
//    axesDirectionButtonLayout_->addStretch();
//	axesDirectionButtonLayout_->addWidget(zAxesUpButton_);
//    axesDirectionButtonLayout_->addWidget(new QLabel("+Z up, -Y forward"));
//    axesDirectionButtonLayout_->addStretch();

    animationNodeButton_ = new QRadioButton(this);
    animationEditorButton_ = new QRadioButton(this);
    QButtonGroup *animationButtonGroup = new QButtonGroup(this);
    animationButtonGroup->addButton(animationNodeButton_);
    animationButtonGroup->addButton(animationEditorButton_);
    animationEditorButton_->setChecked(true);
    animationModeButtonLayout_ = new QHBoxLayout(nullptr);
    animationModeButtonLayout_->addWidget(new QLabel("Animation Mode:"));
    animationModeButtonLayout_->addStretch();
    animationModeButtonLayout_->addWidget(animationEditorButton_);
    animationModeButtonLayout_->addWidget(new QLabel("Animation Editor"));
    animationModeButtonLayout_->addStretch();
    animationModeButtonLayout_->addWidget(animationNodeButton_);
    animationModeButtonLayout_->addWidget(new QLabel("Animation Node"));
    animationModeButtonLayout_->addStretch();

    selButtonLayout_ = new QVBoxLayout(nullptr);
//    selButtonLayout_->addLayout(axesDirectionButtonLayout_);
    selButtonLayout_->addLayout(animationModeButtonLayout_);

	dialogButtonBox_ = new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this};
	dialogButtonBox_->button(QDialogButtonBox::Ok)->setText("Import");
	connect(dialogButtonBox_, &QDialogButtonBox::rejected, this, &MeshAssetImportDialog::reject);
	connect(dialogButtonBox_, &QDialogButtonBox::accepted, [this]() {
		applyChangesToScenegraph();
		Q_EMIT accept();
	});

	layout_ = new QGridLayout{this};
	layout_->setRowStretch(0, 0);
	layout_->setRowStretch(1, 1);
    layout_->setRowStretch(2, 1);
    layout_->setRowStretch(3, 1);

	layout_->addWidget(new QLabel("Check/Uncheck the external assets that you would like to import into the scene.", 0));
	layout_->addWidget(widget_, 1, 0);
	layout_->addLayout(massSelectButtonLayout_, 2, 0);
    layout_->addLayout(selButtonLayout_, 3, 0);
    layout_->addWidget(dialogButtonBox_, 4, 0);

	auto sceneGraphRootItem = new QTreeWidgetItem({"Scene Graph", ""});
	widget_->addTopLevelItem(sceneGraphRootItem);

	auto resourcesRootItem = new QTreeWidgetItem({"Resources", ""});
	widget_->addTopLevelItem(resourcesRootItem);

	for (auto i = 0; i < sceneGraph_.nodes.size(); ++i) {
		auto& node = sceneGraph_.nodes[i].value();

		auto& item = nodeTreeList_[i] = new QTreeWidgetItem({QString::fromStdString(node.name), QString::fromStdString((node.subMeshIndices.size() == 1) ? raco::user_types::MeshNode::typeDescription.typeName : raco::user_types::Node::typeDescription.typeName)});
		widgetItemToSubmeshIndexMap_[item] = &node.subMeshIndices;
		item->setIcon(0, node.subMeshIndices.size() == 1 ? raco::style::Icons::instance().typeMesh : raco::style::Icons::instance().typeNode);
		item->setCheckState(0, Qt::CheckState::Checked);
		if (node.parentIndex == raco::core::MeshScenegraphNode::NO_PARENT) {
			sceneGraphRootItem->addChild(item);
		}

		// Handle edge case of a node containing multiple primitives:
		// We currently split those into separate MeshNodes under a singular, separate Node
		if (node.subMeshIndices.size() > 1) {
			auto primitiveParent = new QTreeWidgetItem({QString::fromStdString(fmt::format("{}_meshnodes", node.name)), QString::fromStdString(raco::user_types::Node::typeDescription.typeName)});
			primitiveParent->setIcon(0, raco::style::Icons::instance().typeNode);
			primitiveParent->setCheckState(0, Qt::CheckState::Checked);
			item->addChild(primitiveParent);
			// first element in primitive tree list is the aforementioned singular Node (primitive parent)
			nodeToPrimitiveTreeList_[i].emplace_back(primitiveParent);

			for (auto primitiveIndex = 0; primitiveIndex < node.subMeshIndices.size(); ++primitiveIndex) {
				auto* primitive = new QTreeWidgetItem({QString::fromStdString(fmt::format("{}_meshnode_{}", node.name, primitiveIndex)), QString::fromStdString(raco::user_types::MeshNode::typeDescription.typeName)});
				primitive->setIcon(0, raco::style::Icons::instance().typeMesh);
				primitive->setCheckState(0, Qt::CheckState::Checked);
				nodeToPrimitiveTreeList_[i].emplace_back(primitive);
				primitiveToMeshIndexMap_[primitive] = *node.subMeshIndices[primitiveIndex];
				primitiveParent->addChild(primitive);
			}
		}
		for (auto& index : node.subMeshIndices) {
			meshNodeIndexReferencedByNodes_[*index].emplace(i);
		}
	}

	for (auto i = 0; i < sceneGraph_.nodes.size(); ++i) {
		auto& node = sceneGraph_.nodes[i].value();
		if (node.parentIndex != raco::core::MeshScenegraphNode::NO_PARENT) {
			auto* child = nodeTreeList_[i];
			nodeTreeList_[node.parentIndex]->addChild(child);
		}
	}

	for (auto i = 0; i < sceneGraph_.meshes.size(); ++i) {
		auto& mesh = sceneGraph_.meshes[i].value();
		auto item = meshTreeList_[i] = new QTreeWidgetItem({QString::fromStdString(mesh), QString::fromStdString(raco::user_types::Mesh::typeDescription.typeName)});
		item->setIcon(0, raco::style::Icons::instance().typeMesh);
		item->setCheckState(0, Qt::CheckState::Checked);
		resourcesRootItem->addChild(item);
	}

	for (auto animIndex = 0; animIndex < sceneGraph_.animations.size(); ++animIndex) {
		auto& anim = sceneGraph_.animations[animIndex].value();
		auto animItem = animTreeList_[animIndex] = new QTreeWidgetItem({QString::fromStdString(anim.name), QString::fromStdString(raco::user_types::Animation::typeDescription.typeName)});
		animItem->setIcon(0, raco::style::Icons::instance().typeAnimation);
		animItem->setCheckState(0, Qt::CheckState::Checked);
		resourcesRootItem->addChild(animItem);

		for (auto samplerIndex = 0; samplerIndex < sceneGraph_.animationSamplers[animIndex].size(); ++samplerIndex) {
			auto& sampler = sceneGraph_.animationSamplers[animIndex][samplerIndex];
			auto sampItem = new QTreeWidgetItem({QString::fromStdString(*sampler), QString::fromStdString(raco::user_types::AnimationChannel::typeDescription.typeName)});
			animSamplerItemMap_[animIndex].emplace_back(sampItem);
			sampItem->setIcon(0, raco::style::Icons::instance().typeAnimationChannel);
			sampItem->setCheckState(0, Qt::CheckState::Checked);
			resourcesRootItem->addChild(sampItem);
		}
	}

	if (projectFeatureLevel >= user_types::Skin::typeDescription.featureLevel) {
		for (auto index = 0; index < sceneGraph_.skins.size(); index++) {
			const auto& skin = sceneGraph_.skins[index].value();
			auto item = new QTreeWidgetItem({QString::fromStdString(skin.name), QString::fromStdString(raco::user_types::Skin::typeDescription.typeName)});
			skinTreeList_.emplace_back(item);
			item->setCheckState(0, Qt::CheckState::Checked);
			resourcesRootItem->addChild(item);
		}
	}
	sceneGraphRootItem->setExpanded(true);
	resourcesRootItem->setExpanded(true);
}

// Set check state to all but top-level items.
void MeshAssetImportDialog::checkAll(Qt::CheckState state) {
	for (auto topLevelIndex = 0; topLevelIndex < widget_->topLevelItemCount(); ++topLevelIndex) {
		auto topLevelItem = widget_->topLevelItem(topLevelIndex);
		iterateThroughChildren(topLevelItem, [&](auto* item) {
			item->setCheckState(0, state);
		});
	}
}

void MeshAssetImportDialog::iterateThroughChildren(QTreeWidgetItem* item, const std::function<void(QTreeWidgetItem*)>& func) {
	for (auto childIndex = 0; childIndex < item->childCount(); ++childIndex) {
		auto* child = item->child(childIndex);
		func(child);
		iterateThroughChildren(child, func);
	}
}

void MeshAssetImportDialog::applyChangesToScenegraph() {
	for (auto i = 0; i < meshTreeList_.size(); ++i) {
		if (meshTreeList_[i]->checkState(0) == Qt::Unchecked) {
			sceneGraph_.meshes[i].reset();
			for (const auto& nodeReferencingMesh : meshNodeIndexReferencedByNodes_[i]) {
				auto& submeshIndices = sceneGraph_.nodes[nodeReferencingMesh]->subMeshIndices;
				std::replace(submeshIndices.begin(), submeshIndices.end(), i, -1);
			}
		}
	}

	for (auto i = 0; i < nodeTreeList_.size(); ++i) {
		if (nodeTreeList_[i]->checkState(0) == Qt::Unchecked) {
			sceneGraph_.nodes[i].reset();
			nodeToPrimitiveTreeList_.erase(i);
		}
	}

	for (auto i = 0; i < animTreeList_.size(); ++i) {
		if (animTreeList_[i]->checkState(0) == Qt::Unchecked) {
			sceneGraph_.animations[i].reset();
		}
	}

	for (const auto& [animIndex, samplerItems] : animSamplerItemMap_) {
		for (auto samplerIndex = 0; samplerIndex < samplerItems.size(); ++samplerIndex) {
			if (samplerItems[samplerIndex]->checkState(0) == Qt::Unchecked) {
				sceneGraph_.animationSamplers[animIndex][samplerIndex].reset();
			}
		}
	}

	for (const auto& [nodeIndex, primitiveItems] : nodeToPrimitiveTreeList_) {
		auto primitiveParent = primitiveItems.front();
		auto& subMeshIndices = sceneGraph_.nodes[nodeIndex]->subMeshIndices;
		if (primitiveParent->checkState(0) == Qt::Unchecked) {
			subMeshIndices.clear();
		} else {
			for (auto primIndex = 1; primIndex < subMeshIndices.size() + 1; ++primIndex) {
				if (primitiveItems[primIndex]->checkState(0) == Qt::Unchecked) {
					subMeshIndices[primIndex - 1].reset();
				}
			}
		}
	}

	for (auto index = 0; index < skinTreeList_.size(); index++) {
		if (skinTreeList_[index]->checkState(0) == Qt::Unchecked) {
			sceneGraph_.skins[index].reset();
		}
	}
}

}  // namespace raco::common_widgets
