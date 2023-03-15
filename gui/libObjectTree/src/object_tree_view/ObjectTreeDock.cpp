/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "object_tree_view/ObjectTreeDock.h"

#include "common_widgets/NoContentMarginsLayout.h"
#include "core/Context.h"
#include "object_tree_view_model/ObjectTreeViewDefaultModel.h"
#include "object_tree_view/ObjectTreeView.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndexList>
#include <QSortFilterProxyModel>

namespace raco::object_tree::view {

ObjectTreeDock::ObjectTreeDock(const char *dockTitle, QWidget *parent)
	: CDockWidget(dockTitle, parent), treeViewStack_(nullptr) {
	treeDockContent_ = new QWidget(parent);
	setWidget(treeDockContent_);
	setAttribute(Qt::WA_DeleteOnClose);
	setFeature(ads::CDockWidget::DockWidgetDeleteOnClose, true);

	treeDockLayout_ = new raco::common_widgets::NoContentMarginsLayout<QVBoxLayout>(treeDockContent_);
	treeDockContent_->setLayout(treeDockLayout_);

	filterLineEdit_ = new QLineEdit(this);
	filterLineEdit_->setPlaceholderText("Filter Objects...");
	filterByComboBox_ = new QComboBox(this);
	filterByComboBox_->addItem("Filter by Name", QVariant(raco::object_tree::model::ObjectTreeViewDefaultModel::ColumnIndex::COLUMNINDEX_NAME));
	filterByComboBox_->addItem("Filter by Type", QVariant(raco::object_tree::model::ObjectTreeViewDefaultModel::ColumnIndex::COLUMNINDEX_TYPE));
	filterByComboBox_->addItem("Filter by Object ID", QVariant(raco::object_tree::model::ObjectTreeViewDefaultModel::ColumnIndex::COLUMNINDEX_ID));
	filterByComboBox_->addItem("Filter by User Tag", QVariant(raco::object_tree::model::ObjectTreeViewDefaultModel::ColumnIndex::COLUMNINDEX_USERTAGS));

	auto treeDockSettingsWidget = new QWidget(treeDockContent_);
	treeDockSettingsLayout_ = new raco::common_widgets::NoContentMarginsLayout<QHBoxLayout>(treeDockSettingsWidget);
	treeDockSettingsLayout_->setContentsMargins(2, 3, 2, 0);
	treeDockSettingsLayout_->addWidget(filterLineEdit_);
	treeDockSettingsLayout_->addWidget(filterByComboBox_);

	treeDockLayout_->addWidget(treeDockSettingsWidget);

	treeViewStack_ = new QStackedWidget(treeDockContent_);
	treeDockLayout_->addWidget(treeViewStack_);


	connect(filterLineEdit_, &QLineEdit::textChanged, [this]() {
		filterTreeViewObjects();
	});

	connect(filterByComboBox_, &QComboBox::currentTextChanged, [this]() {
		auto currentIndex = filterByComboBox_->currentData().toInt();

		getActiveTreeView()->setFilterKeyColumn(currentIndex);

		filterTreeViewObjects();
	});


}

raco::object_tree::view::ObjectTreeDock::~ObjectTreeDock() {
	Q_EMIT dockClosed(this);
}

void ObjectTreeDock::setTreeView(ObjectTreeView *treeView) {
	const auto treeViewTitle = treeView->getViewTitle();

	filterLineEdit_->setVisible(treeView->hasProxyModel());
	filterByComboBox_->setVisible(treeView->hasProxyModel());

    connect(this, &ObjectTreeDock::selectObject, treeView, &ObjectTreeView::selectObject);

	connect(treeView, &ObjectTreeView::newObjectTreeItemsSelected, [this](const auto &handles) {
		Q_EMIT newObjectTreeItemsSelected(handles, this);
	});
	connect(treeView, &ObjectTreeView::externalObjectSelected, [this]() {
		Q_EMIT externalObjectSelected(this);
	});
	connect(treeView, &ObjectTreeView::dockSelectionFocusRequested, [this]() {
		Q_EMIT dockSelectionFocusRequested(this);
	});

	treeViewStack_->removeWidget(treeViewStack_->currentWidget());
	treeViewStack_->addWidget(treeView);
}

ObjectTreeView *ObjectTreeDock::getActiveTreeView() const {
	return dynamic_cast<ObjectTreeView *>(treeViewStack_->currentWidget());
}

void ObjectTreeDock::resetSelection() {
	getActiveTreeView()->resetSelection();
}

void ObjectTreeDock::filterTreeViewObjects() {
	getActiveTreeView()->filterObjects(filterLineEdit_->text());
}

}  // namespace raco::object_tree::view
