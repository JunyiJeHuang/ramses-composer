/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "property_browser/editors/RefEditor.h"

#include "common_widgets/ObjectSearchView.h"
#include "common_widgets/PropertyBrowserButton.h"
#include "core/Context.h"
#include "core/Project.h"
#include "core/Queries.h"
#include "property_browser/PropertyBrowserEditorPopup.h"
#include "property_browser/PropertyBrowserItem.h"
#include "property_browser/PropertyBrowserLayouts.h"
#include "property_browser/PropertyBrowserModel.h"
#include "property_browser/controls/MouseWheelGuard.h"
#include "style/Colors.h"
#include "style/Icons.h"
#include "NodeData/nodeManager.h"

#include <QApplication>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>

namespace raco::property_browser {

using namespace raco::style;

class RefEditorPopup : public PropertyBrowserEditorPopup {
public:
	class RefSearchView : public ObjectSearchView {
	public:
		RefSearchView(const raco::property_browser::PropertyBrowserRef::RefItems& refs, components::SDataChangeDispatcher dispatcher, core::Project* project, const core::ValueHandle& obj, QWidget* parent)
			: ObjectSearchView(dispatcher, project, obj, parent), refs_(refs) {
			rebuild();
			updateSelection();
		}

	protected:
		void rebuild() noexcept override {
			model_.clear();

			for (auto i = 1; i < refs_.size(); ++i) {
				auto ref = refs_[i];
				auto obj = project_->getInstanceByID(ref.objId.toStdString());
				auto* item = new raco::common_widgets::ObjectSearchViewItem{ref.objName, obj};
				model_.appendRow(item);
			}
		}

	const raco::property_browser::PropertyBrowserRef::RefItems &refs_;
	};


	RefEditorPopup(PropertyBrowserItem* item, QWidget* anchor) : PropertyBrowserEditorPopup{item, anchor, new RefSearchView(item->refItem()->items(), item->dispatcher(), item->project(), item->valueHandle(), anchor)} {
		bool hasRef = (item->refItem()->currentIndex() != PropertyBrowserRef::EMPTY_REF_INDEX);

		if (hasRef) {
			currentRelation_.setReadOnly(true);
			currentRelation_.setText(item->refItem()->items().at(item->refItem()->currentIndex()).objName);
			deleteButton_.setFlat(true);
			deleteButton_.setIcon(Icons::instance().remove);
		} else {
			currentRelation_.setVisible(false);
			deleteButton_.setVisible(false);
		}
	}

protected:
	void establishObjectRelation() override {
		item_->refItem()->setIndex((list_->selection().row() + 1));
	}

	void removeObjectRelation() override {
		item_->refItem()->setIndex(PropertyBrowserRef::EMPTY_REF_INDEX);
	}
};


RefEditor::RefEditor(
	PropertyBrowserItem* item,
	QWidget* parent)
	: PropertyEditor(item, parent),
	  ref_{item->refItem()} {
	auto* layout{new raco::common_widgets::NoContentMarginsLayout<QHBoxLayout>{this}};

	changeRefButton_ = new raco::common_widgets::PropertyBrowserButton(raco::style::Icons::instance().openInNew, "", this);
	QObject::connect(changeRefButton_, &QPushButton::clicked, [this]() {
		new RefEditorPopup(item_, changeRefButton_);
	});

	layout->addWidget(changeRefButton_);

	currentRef_ = new QLineEdit{this};
	currentRef_->setReadOnly(true);
	currentRef_->setText(ref_->items().at(ref_->currentIndex()).objName);
	currentRef_->setToolTip(ref_->items().at(ref_->currentIndex()).tooltipText);
	currentRef_->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
	emptyReference_ = (ref_->currentIndex() == PropertyBrowserRef::EMPTY_REF_INDEX);
	QObject::connect(currentRef_, &QLineEdit::customContextMenuRequested, this, &RefEditor::createCustomContextMenu);
	layout->addWidget(currentRef_);

	goToRefObjectButton_ = new raco::common_widgets::PropertyBrowserButton(raco::style::Icons::instance().goTo, "", this);

	QObject::connect(goToRefObjectButton_, &QPushButton::clicked, [this, item]() {
		item->model()->Q_EMIT objectSelectionRequested(ref_->items().at(ref_->currentIndex()).objId);
	});

	layout->addWidget(goToRefObjectButton_);


	QObject::connect(ref_, &PropertyBrowserRef::indexChanged, [this](auto index) {
		emptyReference_ = (index == PropertyBrowserRef::EMPTY_REF_INDEX);
		goToRefObjectButton_->setDisabled(emptyReference_);
		currentRef_->setText(ref_->items().at(ref_->currentIndex()).objName);
		currentRef_->setToolTip(ref_->items().at(ref_->currentIndex()).tooltipText);
	});

	QObject::connect(ref_, &PropertyBrowserRef::indexChanged, comboBox_, &QComboBox::setCurrentIndex);
	QObject::connect(ref_, &PropertyBrowserRef::itemsChanged, this, &RefEditor::updateItems);
	QObject::connect(comboBox_, qOverload<int>(&QComboBox::activated), ref_, &PropertyBrowserRef::setIndex);
	QObject::connect(comboBox_, qOverload<int>(&QComboBox::currentIndexChanged), [this, item](auto index) {
		emptyReference_ = (index == PropertyBrowserRef::EMPTY_REF_INDEX);
		goToRefObjectButton_->setDisabled(emptyReference_);
		if (item->displayName() == "material") {
			//setNodeDataMaterialChanged(index);
		}

		comboBox_->setToolTip(comboBox_->itemData(index, Qt::ToolTipRole).toString());
	});

	// Override the enabled behaviour of the parent class, so that the goto button can remain enabled even though the rest of the control gets disabled.
	setEnabled(true);
	currentRef_->setEnabled(item->editable());
	changeRefButton_->setEnabled(item->editable());
	goToRefObjectButton_->setEnabled(!emptyReference_);
	QObject::disconnect(item, &PropertyBrowserItem::editableChanged, this, &QWidget::setEnabled);
	QObject::connect(item, &PropertyBrowserItem::editableChanged, this, [this]() {
		currentRef_->setEnabled(item_->editable());
		changeRefButton_->setEnabled(item_->editable());
		goToRefObjectButton_->setEnabled(!emptyReference_);
	});
}

void RefEditor::setNodeDataMaterialChanged(int index) {
	raco::guiData::NodeData* pNode = raco::guiData::NodeDataManager::GetInstance().getActiveNode();

	if ((index > 0) && pNode->getMaterialName() != ref_->items().at(index).objName.toStdString()) {  // is't empty
		pNode->uniformClear();
		pNode->setMaterialName(ref_->items().at(index).objName.toStdString());
		pNode->setMaterialIsChanged(true);
	}
}

void RefEditor::updateItems(const PropertyBrowserRef::ComboBoxItems& items) {
	QObject::disconnect(comboBox_, qOverload<int>(&QComboBox::activated), ref_, &PropertyBrowserRef::setIndex);
	comboBox_->clear();
	for (const auto& comboItem : items) {
		comboBox_->addItem(comboItem.objName, comboItem.objId);
		comboBox_->setItemData(comboBox_->count() - 1, comboItem.tooltipText, Qt::ToolTipRole);
	}
	comboBox_->setCurrentIndex(ref_->currentIndex());
	QObject::connect(comboBox_, qOverload<int>(&QComboBox::activated), ref_, &PropertyBrowserRef::setIndex);
}

bool RefEditor::unexpectedEmptyReference() const noexcept {
	return emptyReference_ && !item_->valueHandle().query<core::ExpectEmptyReference>();
}

void RefEditor::createCustomContextMenu(const QPoint& p) {
	if (!item_->editable()) {
		return;
	}

	auto deserialization{raco::serialization::deserializeObjects(RaCoClipboard::get())};
	if (!deserialization) {
		return;
	}

	auto copiedObjects = raco::core::BaseContext::getTopLevelObjectsFromDeserializedObjects(*deserialization, item_->project());

	if (copiedObjects.empty()) {
		return;
	}

	const auto& refItems = ref_->items();
	std::vector<std::pair<PropertyBrowserRef::RefItem, int>> validRefTargets;
	for (auto refIndex = 0; refIndex < refItems.size(); ++refIndex) {
		const auto& item = refItems[refIndex];
		for (const auto& copiedObj : copiedObjects) {
			if (copiedObj->objectID() == item.objId.toStdString()) {
				validRefTargets.emplace_back(item, refIndex);
				break;
			}
		}
	}

	if (validRefTargets.empty()) {
		return;
	}

	auto* lineEditMenu = new QMenu(this);

	if (validRefTargets.size() == 1) {
		lineEditMenu->setToolTipsVisible(true);
		auto* setRefAction = lineEditMenu->addAction(QString("Set Reference to Copied Object %1").arg(validRefTargets.front().first.objName), [this, &validRefTargets]() {
			ref_->setIndex(validRefTargets.front().second);
		});
		setRefAction->setToolTip(validRefTargets.front().first.tooltipText);
	} else if (validRefTargets.size() > 1) {
		auto setReferenceMenu = lineEditMenu->addMenu("Set Reference to Copied Objects...");
		setReferenceMenu->setToolTipsVisible(true);

		for (const auto& validRefTarget : validRefTargets) {
			auto* setRefAction = setReferenceMenu->addAction(validRefTarget.first.objName, [this, &validRefTarget]() {
				ref_->setIndex(validRefTarget.second);
			});
			setRefAction->setToolTip(validRefTarget.first.tooltipText);
		}
	}

	lineEditMenu->exec(currentRef_->mapToGlobal(p));
}

}  // namespace raco::property_browser
