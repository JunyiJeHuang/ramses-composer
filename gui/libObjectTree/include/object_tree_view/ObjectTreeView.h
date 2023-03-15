/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "MeshData/MeshDataManager.h"
#include "core/EditorObject.h"
#include "node_logic/NodeLogic.h"
#include "signal/SignalProxy.h"
#include <QAbstractItemModel>
#include <QTreeView>
#include <unordered_set>


namespace raco::object_tree::model {
class ObjectTreeViewDefaultSortFilterProxyModel;
class ObjectTreeViewDefaultModel;
}

namespace raco::object_tree::view {

class ObjectTreeView : public QTreeView {
	Q_OBJECT

	using ValueHandle = core::ValueHandle;
	using EditorObject = core::EditorObject;
	using SEditorObject = core::SEditorObject;

public:
	ObjectTreeView(const QString &viewTitle, raco::object_tree::model::ObjectTreeViewDefaultModel *viewModel, raco::object_tree::model::ObjectTreeViewDefaultSortFilterProxyModel *sortFilterProxyModel = nullptr, QWidget *parent = nullptr);

	std::set<ValueHandle> getSelectedHandles() const;
	std::vector<SEditorObject> getSortedSelectedEditorObjects() const;
	QString getViewTitle() const;

	void getOnehandle(QModelIndex index, NodeData *parent, raco::guiData::NodeDataManager &nodeDataManager, std::map<std::string, core::ValueHandle> &NodeNameHandleReMap);
    void getOneMeshHandle(QModelIndex index);
    bool getOneMeshData(ValueHandle valueHandle, raco::guiData::MeshData &meshData);
    bool getOneMaterialHandle(ValueHandle &valueHandle);
    void getOneMaterials(QModelIndex index, std::map<std::string, core::ValueHandle> &materialHandleMap);
    std::map<std::string, core::ValueHandle> updateNodeTree();
	std::map<std::string, core::ValueHandle> updateResource();
    std::map<std::string, core::ValueHandle> updateMaterial();
	std::map<std::string, core::ValueHandle> updateTexture();
    void updateMeshData();
    int attriElementSize(raco::guiData::VertexAttribDataType type);
	void convertGltfAnimation(QString fileName);
    bool getAnimationHandle(QModelIndex index, core::ValueHandle valueHandle, std::set<raco::core::ValueHandle> &handles);

	void requestNewNode(EditorObject::TypeDescriptor nodeType, const std::string &nodeName, const QModelIndex &parent);
	void showContextMenu(const QPoint &p);
	bool canCopyAtIndices(const QModelIndexList &indices);
	bool canPasteIntoIndex(const QModelIndex &index, bool asExtref);
	bool canProgrammaticallyGoToObject();
	bool containsObject(const QString &objectID) const;
	void setFilterKeyColumn(int column);
	void filterObjects(const QString &filterString);

	bool hasProxyModel() const;

	QModelIndexList getSelectedIndices(bool sorted = false) const;
	QModelIndex getSelectedInsertionTargetIndex() const;
	void collapseRecusively(const QModelIndex &index);

	void createBMWMaterial(const std::vector<MaterialData> &materialArr, const QModelIndex &parent);

Q_SIGNALS:
	void dockSelectionFocusRequested(ObjectTreeView *focusTree);
	void newNodeRequested(EditorObject::TypeDescriptor nodeType, const std::string &nodeName, const QModelIndex &parent);
	void newObjectTreeItemsSelected(const std::set<ValueHandle> &handles);
    void externalObjectSelected();
    void setMaterialResHandles(const std::map<std::string, core::ValueHandle>& map);
	void setTextureResHandles(const std::map<std::string, core::ValueHandle> &map);
    void updateNodeHandles(const QString &title, const std::map<std::string, core::ValueHandle> &map);

public Q_SLOTS:
	void resetSelection();
	void copyObjects();
	void pasteObjects(const QModelIndex &index, bool asExtRef = false);
	void cutObjects();
	void deleteObjects();
    void globalOpreations();
	void duplicateObjects();

	void selectObject(const QString &objectID);
	void expandAllParentsOfObject(const QString &objectID);
	void expanded(const QModelIndex &index);
	void collapsed(const QModelIndex &index);
    void getMaterialResHandles();
	void getTextureResHandles();
    void fillMeshData();
    void selectActiveObject();
    // void deleteAnimationHandle(std::string id);
	void importBMWAssets(NodeData *nodeData, const std::vector<MaterialData>& maetrials);
    void deleteAnimationHandle(std::set<std::string> ids);
	void importBMWAssets(NodeData *nodeData, const std::vector<MaterialData>& maetrials);
	
protected:
	static inline auto SELECTION_MODE = QItemSelectionModel::Select | QItemSelectionModel::Rows;

	raco::object_tree::model::ObjectTreeViewDefaultModel *treeModel_;
	raco::object_tree::model::ObjectTreeViewDefaultSortFilterProxyModel *proxyModel_;
	QString viewTitle_;
	std::unordered_set<std::string> expandedItemIDs_;
	std::unordered_set<std::string> selectedItemIDs_;

	virtual QMenu* createCustomContextMenu(const QPoint &p);

	void dragMoveEvent(QDragMoveEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;

	std::vector<SEditorObject> indicesToSEditorObjects(const QModelIndexList &index) const;
	core::SEditorObject indexToSEditorObject(const QModelIndex &index) const;
	std::string indexToTreeNodeID(const QModelIndex &index) const;
	QModelIndex indexFromTreeNodeID(const std::string &id) const;

protected Q_SLOTS:
	void restoreItemExpansionStates();
	void restoreItemSelectionStates();
	void expandAllParentsOfObject(const QModelIndex &index);
};

}
