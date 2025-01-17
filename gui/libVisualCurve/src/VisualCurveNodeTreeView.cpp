#include "visual_curve/VisualCurveNodeTreeView.h"
#include "core/Undo.h"
#include "VisualCurveData/VisualCurvePosManager.h"

namespace raco::visualCurve {

TreeModel::TreeModel(QWidget *parent) {

}

void TreeModel::setFolderDataMgr(FolderDataManager *mgr) {
    folderDataMgr_ = mgr;
}

bool TreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    auto curveFromItem = [=](QStandardItem *item)->QString {
        QString curve = item->text();
        while(item->parent()) {
            item = item->parent();
            QString tempStr = item->text() + "|";
            curve.insert(0, tempStr);
        }
        return curve;
    };

    QVector<qint64> vector;
    QByteArray array = data->data("test");
    QDataStream stream(&array, QIODevice::ReadOnly);
    stream >> vector;
    QModelIndex tempParent = parent;
    QStandardItem *parentItem = itemFromIndex(parent);
    std::string destNode = parentItem ? parentItem->text().toStdString() : "default";

    for (auto p : qAsConst(vector)) {
        QModelIndex* index = (QModelIndex*)p;
        if (index) {
            QStandardItem *item = itemFromIndex(*index);
            std::string srcCurvePath = curveFromItem(item).toStdString();
            if (folderDataMgr_->isCurve(srcCurvePath)) {
                Folder *srcFolder{nullptr};
                STRUCT_CURVE_PROP *srcCurveProp{nullptr};
                if (folderDataMgr_->curveFromPath(srcCurvePath, &srcFolder, &srcCurveProp)) {
                    if (parentItem) {
                        // curve move to node
                        std::string destCurvePath = curveFromItem(parentItem).toStdString();
                        if (!moveCurveToNode(srcFolder, srcCurveProp, srcCurvePath, destCurvePath)) {
                            return false;
                        }
                        move(index->parent(), index->row(), tempParent, parentItem->rowCount());
                    } else {
                        // curve move to default node
                        if (srcFolder == folderDataMgr_->getRootFolder()) {
                            return false;
                        }
                        moveCurveToDefaultNode(srcFolder, srcCurveProp, srcCurvePath);
                        move(index->parent(), index->row(), tempParent, rowCount());
                    }
                }
            } else {
                Folder *srcFolder{nullptr};
                if (folderDataMgr_->folderFromPath(srcCurvePath, &srcFolder)) {
                    if (parentItem) {
                        // move folder to node
                        std::string destCurvePath = curveFromItem(parentItem).toStdString();
                        if (!moveFolderToNode(srcFolder, srcCurvePath, destCurvePath)) {
                            return false;
                        }
                        move(index->parent(), index->row(), tempParent, parentItem->rowCount());
                    } else {
                        // move folder to default node
                        if (srcFolder == folderDataMgr_->getRootFolder() || folderDataMgr_->getRootFolder()->hasFolder(srcFolder->getFolderName())) {
                            return false;
                        }
                        moveFolderToDefaultNode(srcFolder, srcCurvePath);
                        move(index->parent(), index->row(), tempParent, rowCount());
                    }
                }
            }
            delete index;
        }
    }
    Q_EMIT moveRowFinished(destNode);
    Q_EMIT signal::signalProxy::GetInstance().sigCheckCurveBindingValid_From_CurveUI();
    return true;
}

Qt::DropActions TreeModel::supportedDropActions() const {
    return Qt::MoveAction;
}

QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData* mimeData = QAbstractItemModel::mimeData(indexes);

    auto sortInsertData = [&](QVector<qint64> &vec, QModelIndex* p) {
        for (int i{0}; i < vec.size(); ++i) {
            QModelIndex *tIndex = (QModelIndex*)vec.at(i);
            if (tIndex->row() < p->row()) {
                vec.insert(i, (qint64)p);
                return;
            }
        }
        vec.push_back((qint64)p);
    };

    QVector<qint64> vector;
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    for (int i = 0; i < indexes.count(); i++) {
        QModelIndex index = indexes[i];
        QModelIndex* p = new QModelIndex(index);
        QStandardItem *item = itemFromIndex(*p);
        if (item->text().compare("") != 0) {
            sortInsertData(vector, p);
        }
    }
    stream << vector;
    mimeData->setData(QString("test"), array);
    return mimeData;
}

bool TreeModel::move(QModelIndex source, int sourceRow, QModelIndex &dest, int destRow) {
    if (sourceRow < 0 || destRow < 0) {
        return false;
    }
    if(!beginMoveRows(source, sourceRow, sourceRow, dest, destRow)){
        return false;
    }

    beginInsertRows(dest, destRow, destRow);
    if (dest.isValid()) {
        QStandardItem *destItem = itemFromIndex(dest);
        int to = dest.row();
        QStandardItem *sourceItem = itemFromIndex(source);
        QList<QStandardItem *> itemList;
        if (sourceItem) {
            itemList = sourceItem->takeRow(sourceRow);
        } else {
            itemList = takeRow(sourceRow);
            if (sourceRow < to) {
                to -= 1;
                dest = item(to)->index();
            }
        }

        blockSignals(true);
        if(destItem) {
            destItem->insertRow(destRow, itemList);
        }
//        endInsertRows();
        blockSignals(false);
    } else {
        QStandardItem *sourceItem = itemFromIndex(source);
        QList<QStandardItem *> itemList;
        if (sourceItem) {
            itemList = sourceItem->takeRow(sourceRow);
        } else {
            itemList = takeRow(sourceRow);
        }

        blockSignals(true);
//        beginInsertRows(dest, destRow, destRow);
        insertRow(destRow, itemList);
//        endInsertRows();
        blockSignals(false);
    }
    endMoveRows();
    return true;
}

void TreeModel::swapCurve(std::string oldCurve, std::string newCurve) {
    QList<SKeyPoint> keyPoints;
    VisualCurvePosManager::GetInstance().getKeyPointList(oldCurve, keyPoints);
    VisualCurvePosManager::GetInstance().deleteKeyPointList(oldCurve);
    VisualCurvePosManager::GetInstance().addKeyPointList(newCurve, keyPoints);
}

void TreeModel::setFolderPath(Folder *folder, std::string path) {
    if(!folder)
        return ;

    for (const auto &it : folder->getCurveList()) {
        std::string curveName;
        folderDataMgr_->pathFromCurve(it->curve_, folder, curveName);
        std::string oldCurvePath = path + "|" + it->curve_;
        if (CurveManager::GetInstance().getCurve(oldCurvePath)) {
            Curve *tempCurve = CurveManager::GetInstance().getCurve(oldCurvePath);
            tempCurve->setCurveName(curveName);
            swapCurve(oldCurvePath, curveName);
        }
    }

    std::list<Folder *> folderList = folder->getFolderList();
    for (auto it = folderList.begin(); it != folderList.end(); ++it) {
        setFolderPath(*it, std::string(path + "|" + (*it)->getFolderName()));
    }
}

bool TreeModel::moveCurveToNode(Folder *srcFolder, STRUCT_CURVE_PROP *srcCurveProp, std::string srcCurvePath, std::string destCurvePath) {
    Folder *destFolder{nullptr};
    if (!folderDataMgr_->isCurve(destCurvePath)) {
        if (folderDataMgr_->folderFromPath(destCurvePath, &destFolder)) {
            if (!destFolder->hasCurve(srcCurveProp->curve_)) {
                srcFolder->takeCurve(srcCurveProp->curve_);
                destFolder->insertCurve(srcCurveProp);
                if (CurveManager::GetInstance().getCurve(srcCurvePath)) {
                    destCurvePath = destCurvePath + "|" + srcCurveProp->curve_;
                    Curve *curve = CurveManager::GetInstance().getCurve(srcCurvePath);
                    curve->setCurveName(destCurvePath);
                    swapCurve(srcCurvePath, destCurvePath);
                }
                return true;
            }
        }
    }
    return false;
}

bool TreeModel::moveCurveToDefaultNode(Folder *srcFolder, STRUCT_CURVE_PROP *srcCurveProp, std::string srcCurvePath) {
    srcFolder->takeCurve(srcCurveProp->curve_);
    folderDataMgr_->getRootFolder()->insertCurve(srcCurveProp);
    if (CurveManager::GetInstance().getCurve(srcCurvePath)) {
        Curve *curve = CurveManager::GetInstance().getCurve(srcCurvePath);
        curve->setCurveName(srcCurveProp->curve_);
        swapCurve(srcCurvePath, srcCurveProp->curve_);
    }
    return true;
}

bool TreeModel::moveFolderToNode(Folder *srcFolder, std::string srcCurvePath, std::string destCurvePath) {
    if (srcCurvePath.empty()) {
        return false;
    }
    if (srcFolder->parent()) {
        srcFolder = srcFolder->parent()->takeFolder(srcFolder->getFolderName());
    } else {
        srcFolder = srcFolder->takeFolder(srcCurvePath);
    }

    Folder *destFolder{nullptr};
    STRUCT_CURVE_PROP *destCurveProp{nullptr};
    if (!folderDataMgr_->isCurve(destCurvePath)) {
        if (folderDataMgr_->folderFromPath(destCurvePath, &destFolder)) {
            destFolder->insertFolder(srcFolder);
            setFolderPath(srcFolder, srcCurvePath);
            return true;
        }
    }
    return false;
}

bool TreeModel::moveFolderToDefaultNode(Folder *srcFolder, std::string srcCurvePath) {
    if (srcFolder->parent()) {
        srcFolder->parent()->takeFolder(srcFolder->getFolderName());
    } else {
        srcFolder->takeFolder(srcCurvePath);
    }
    folderDataMgr_->getRootFolder()->insertFolder(srcFolder);
    setFolderPath(srcFolder, srcCurvePath);
    return true;
}

VisualCurveNodeTreeView::VisualCurveNodeTreeView(QWidget *parent, core::CommandInterface *commandInterface)
    : QWidget{parent},
    commandInterface_(commandInterface) {
    visualCurveTreeView_ = new QTreeView(this);
    visualCurveTreeView_->setAlternatingRowColors(true);
    model_ = new TreeModel(visualCurveTreeView_);
    model_->setColumnCount(2);
    visualCurveTreeView_->setModel(model_);
    visualCurveTreeView_->setHeaderHidden(true);
    visualCurveTreeView_->header()->resizeSection(0, 240);
    visualCurveTreeView_->header()->resizeSection(1, 30);

    visualCurveTreeView_->setDragEnabled(true);
    visualCurveTreeView_->setAcceptDrops(true);
    visualCurveTreeView_->setDragDropMode(QAbstractItemView::InternalMove);
    visualCurveTreeView_->setDropIndicatorShown(true);
    visualCurveTreeView_->setDragDropOverwriteMode(true);
    visualCurveTreeView_->setDefaultDropAction(Qt::DropAction::MoveAction);
    visualCurveTreeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);

    folderDataMgr_ = &FolderDataManager::GetInstance();
    model_->setFolderDataMgr(folderDataMgr_);

    visibleButton_ = new ButtonDelegate(visualCurveTreeView_);
    visualCurveTreeView_->setItemDelegateForColumn(1, visibleButton_);
    visibleButton_->setFolderManager(folderDataMgr_);
    visibleButton_->setModel(model_);

    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->addWidget(visualCurveTreeView_);
    vBoxLayout->setMargin(0);
    this->setLayout(vBoxLayout);
    QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigSwitchVisualCurve, this, &VisualCurveNodeTreeView::slotRefrenceBindingCurve);
    QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigInsertCurve_To_VisualCurve, this, &VisualCurveNodeTreeView::slotInsertCurve);
    QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigRepaintAfterUndoOpreation, this, &VisualCurveNodeTreeView::slotRefreshWidget);

    menu_ = new QMenu{this};
    visibleMenu_ = new QMenu{"Set Visible", this};
    onAct_ = new QAction("ON");
    offAct_ = new QAction("OFF");
    visibleMenu_->addAction(onAct_);
    visibleMenu_->addAction(offAct_);
    createFolder_ = new QAction("Create Node");
    delete_ = new QAction("Delete");
    createCurve_ = new QAction("Create Curve");
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &VisualCurveNodeTreeView::customContextMenuRequested, this, &VisualCurveNodeTreeView::slotShowContextMenu);
    connect(createFolder_, &QAction::triggered, this, &VisualCurveNodeTreeView::slotCreateFolder);
    connect(delete_, &QAction::triggered, this, &VisualCurveNodeTreeView::slotDelete);
    connect(createCurve_, &QAction::triggered, this, &VisualCurveNodeTreeView::slotCreateCurve);
    connect(onAct_, &QAction::triggered, this, &VisualCurveNodeTreeView::slotSetVisibleOn);
    connect(offAct_, &QAction::triggered, this, &VisualCurveNodeTreeView::slotSetVisibleOff);
    connect(model_, &QStandardItemModel::itemChanged, this, &VisualCurveNodeTreeView::slotItemChanged);
    connect(model_, &TreeModel::moveRowFinished, this, &VisualCurveNodeTreeView::slotModelMoved);
    connect(visualCurveTreeView_, &QTreeView::pressed, this, &VisualCurveNodeTreeView::slotCurrentRowChanged);
    connect(visibleButton_, &ButtonDelegate::clicked, this, &VisualCurveNodeTreeView::slotButtonDelegateClicked);
}

VisualCurveNodeTreeView::~VisualCurveNodeTreeView() {

}

void VisualCurveNodeTreeView::initCurves() {
    folderDataMgr_->clear();
    model_->removeRows(0, model_->rowCount());

    for (auto curve : CurveManager::GetInstance().getCurveList()) {
        std::string curvePath = curve->getCurveName();
        QStringList list = QString::fromStdString(curvePath).split("|");
        QString curveName = list.takeLast();

        Folder *folder = folderDataMgr_->getRootFolder();
        QStandardItem *item{nullptr};
        for (const QString &node : list) {
            if (folder->hasFolder(node.toStdString())) {
                if (item) {
                    for (int i{0}; i < item->rowCount(); i++) {
                        if (node.compare(item->child(i)->text()) == 0) {
                            item = item->child(i);
                            break;
                        }
                    }
                } else {
                    for (int i{0}; i < model_->rowCount(); i++) {
                        if (node.compare(model_->item(i)->text()) == 0) {
                            item = model_->item(i);
                            break;
                        }
                    }
                }
                folder = folder->getFolder(node.toStdString());
            } else {
                QStandardItem *nodeItem = new QStandardItem(node);
                if (item) {
                    item->appendRow(nodeItem);
                } else {
                    model_->appendRow(nodeItem);
                }
                item = nodeItem;
                item->setColumnCount(2);
                folder->insertFolder(node.toStdString());
                folder = folder->getFolder(node.toStdString());
            }
        }

        QStandardItem *curveItem = new QStandardItem(curveName);
        folder->insertCurve(curveName.toStdString());
        if (item) {
            item->appendRow(curveItem);
        } else {
            model_->appendRow(curveItem);
        }
    }
}

void VisualCurveNodeTreeView::switchCurSelCurve(std::string curve) {
    QStringList list = QString::fromStdString(curve).split("|");
    QString root = list.takeFirst();
    QStandardItem *item{nullptr};
    for (int i{0}; i < model_->rowCount(); i++) {
        if (model_->item(i)->text() == root) {
            item = model_->item(i);
        }
    }
    if (!item) {
        return;
    }

    for (const QString &node : list) {
        if (!itemFromPath(&item, node)) {
            return;
        }
    }
    visualCurveTreeView_->setCurrentIndex(item->index());
    slotCurrentRowChanged(item->index());
}

void VisualCurveNodeTreeView::cancelSelCurve() {
    visualCurveTreeView_->setCurrentIndex(QModelIndex());
}

void VisualCurveNodeTreeView::setFolderVisible(Folder *folder, bool visible) {
    if(!folder)
        return ;

    folder->setVisible(visible);
    for (const auto &it : folder->getCurveList()) {
        it->visible_ = visible;
        std::string curve;
        folderDataMgr_->pathFromCurve(it->curve_, folder, curve);
        if (visible) {
            VisualCurvePosManager::GetInstance().deleteHidenCurve(curve);
        } else {
            VisualCurvePosManager::GetInstance().insertHidenCurve(curve);
        }
    }

    std::list<Folder *> folderList = folder->getFolderList();
    for (auto it = folderList.begin(); it != folderList.end(); ++it) {
        setFolderVisible(*it, visible);
    }
}

void VisualCurveNodeTreeView::slotInsertCurve(QString property, QString curve, QVariant value) {
    QStandardItem *curveItem = new QStandardItem(curve);
    model_->appendRow(curveItem);
    folderDataMgr_->getRootFolder()->insertCurve(curve.toStdString());
}

void VisualCurveNodeTreeView::slotRefrenceBindingCurve(std::string smapleProp, std::string prop, std::string curve) {
    switchCurSelCurve(curve);
}

void VisualCurveNodeTreeView::slotShowContextMenu(const QPoint &p) {
    QModelIndex curIndex = visualCurveTreeView_->indexAt(p);
    QModelIndex index = curIndex.sibling(curIndex.row(), 0);
    QStandardItem* item = model_->itemFromIndex(index);

    menu_->clear();
    if (item) {
        std::string curve = curveFromItem(item).toStdString();
        menu_->addAction(delete_);
        menu_->addAction(createFolder_);
        menu_->addAction(createCurve_);
        menu_->addMenu(visibleMenu_);
    } else {
        visualCurveTreeView_->setCurrentIndex(QModelIndex());
        menu_->addAction(createFolder_);
        menu_->addAction(createCurve_);
    }
    menu_->exec(mapToGlobal(p));
}

void VisualCurveNodeTreeView::slotCreateFolder() {
    QModelIndex index = visualCurveTreeView_->currentIndex();
    if (index.isValid()) {
        QStandardItem *item = model_->itemFromIndex(index);
        std::string curve = curveFromItem(item).toStdString();

        if (folderDataMgr_->isCurve(curve)) {
            Folder *folder{nullptr};
            STRUCT_CURVE_PROP *curveProp{nullptr};
            if (folderDataMgr_->curveFromPath(curve, &folder, &curveProp)) {
                std::string defaultFolder = folder->createDefaultFolder();
                QStandardItem *folderItem = new QStandardItem(QString::fromStdString(defaultFolder));
                folder->insertFolder(defaultFolder);
                if (item->parent()) {
                    item->parent()->appendRow(folderItem);
                } else {
                    model_->appendRow(folderItem);
                }
            }
        } else {
            Folder *folder{nullptr};
            if (folderDataMgr_->folderFromPath(curve, &folder)) {
                std::string defaultFolder = folder->createDefaultFolder();
                QStandardItem *folderItem = new QStandardItem(QString::fromStdString(defaultFolder));
                folder->insertFolder(defaultFolder);
                item->appendRow(folderItem);
            }
        }
    } else {
        std::string defaultFolder = folderDataMgr_->getRootFolder()->createDefaultFolder();
        QStandardItem *folderItem = new QStandardItem(QString::fromStdString(defaultFolder));
        folderDataMgr_->getRootFolder()->insertFolder(defaultFolder);
        model_->appendRow(folderItem);
    }
}

void VisualCurveNodeTreeView::slotCreateCurve() {
    QModelIndex selected = visualCurveTreeView_->currentIndex();
    if (selected.isValid()) {
        QStandardItem *item = model_->itemFromIndex(selected);
        std::string curve = curveFromItem(item).toStdString();

        if (folderDataMgr_->isCurve(curve)) {
            Folder *folder{nullptr};
            STRUCT_CURVE_PROP *curveProp{nullptr};
            if (folderDataMgr_->curveFromPath(curve, &folder, &curveProp)) {
                std::string createCurve = folder->createDefaultCurve();
                folder->insertCurve(createCurve);
                QStandardItem *curveItem = new QStandardItem(QString::fromStdString(createCurve));
                if (item->parent()) {
                    item->parent()->appendRow(curveItem);
                } else {
                    model_->appendRow(curveItem);
                }

                Curve *tempCurve = new Curve;
                tempCurve->setCurveName(createCurve);
                CurveManager::GetInstance().addCurve(tempCurve);
                pushState2UndoStack(fmt::format("create curve: '{}'", createCurve));
            }
        } else {
            Folder *folder{nullptr};
            if (folderDataMgr_->folderFromPath(curve, &folder)) {
                std::string createCurve = folder->createDefaultCurve();
                folder->insertCurve(createCurve);
                QStandardItem *curveItem = new QStandardItem(QString::fromStdString(createCurve));
                item->appendRow(curveItem);

                Curve *tempCurve = new Curve;
                tempCurve->setCurveName(createCurve);
                CurveManager::GetInstance().addCurve(tempCurve);
                pushState2UndoStack(fmt::format("create curve: '{}'", createCurve));
            }
        }
    } else {
        std::string createCurve = folderDataMgr_->getRootFolder()->createDefaultCurve();
        folderDataMgr_->getRootFolder()->insertCurve(createCurve);
        QStandardItem *curveItem = new QStandardItem(QString::fromStdString(createCurve));
        model_->appendRow(curveItem);

        Curve *tempCurve = new Curve;
        tempCurve->setCurveName(createCurve);
        CurveManager::GetInstance().addCurve(tempCurve);
        pushState2UndoStack(fmt::format("create curve: '{}'", createCurve));
    }
}

void VisualCurveNodeTreeView::slotSetVisibleOn() {
    std::string info;
    QModelIndexList selectedIndexs = visualCurveTreeView_->selectionModel()->selectedRows();
    std::sort(selectedIndexs.begin(), selectedIndexs.end(), [this](QModelIndex index1, QModelIndex index2) {
        QStandardItem *item1 = model_->itemFromIndex(index1);
        QStandardItem *item2 = model_->itemFromIndex(index2);
        return item1->row() > item2->row();
    });
    for (auto selected : selectedIndexs) {
        QStandardItem *item = model_->itemFromIndex(selected);
        if (item) {
            std::string itemName = curveFromItem(item).toStdString();
            if (!itemName.empty()) {
                if (folderDataMgr_->isCurve(itemName)) {
                    Folder *folder{nullptr};
                    STRUCT_CURVE_PROP *curveProp{nullptr};
                    if (folderDataMgr_->curveFromPath(itemName, &folder, &curveProp)) {
                        curveProp->visible_ = true;
                        VisualCurvePosManager::GetInstance().deleteHidenCurve(itemName);
                    }
                }
                info += itemName + ";";
            }
        }
    }
    visibleButton_->updateWidget();
    Q_EMIT sigRefreshVisualCurve();
    pushState2UndoStack(fmt::format("set visible on: '{}'", info));
}

void VisualCurveNodeTreeView::slotSetVisibleOff() {
    std::string info;
    QModelIndexList selectedIndexs = visualCurveTreeView_->selectionModel()->selectedRows();
    std::sort(selectedIndexs.begin(), selectedIndexs.end(), [this](QModelIndex index1, QModelIndex index2) {
        QStandardItem *item1 = model_->itemFromIndex(index1);
        QStandardItem *item2 = model_->itemFromIndex(index2);
        return item1->row() > item2->row();
    });
    for (auto selected : selectedIndexs) {
        QStandardItem *item = model_->itemFromIndex(selected);
        if (item) {
            std::string itemName = curveFromItem(item).toStdString();
            if (!itemName.empty()) {
                if (folderDataMgr_->isCurve(itemName)) {
                    Folder *folder{nullptr};
                    STRUCT_CURVE_PROP *curveProp{nullptr};
                    if (folderDataMgr_->curveFromPath(itemName, &folder, &curveProp)) {
                        curveProp->visible_ = false;
                        VisualCurvePosManager::GetInstance().insertHidenCurve(itemName);
                    }
                }
                info += itemName + ";";
            }
        }
    }
    visibleButton_->updateWidget();
    Q_EMIT sigRefreshVisualCurve();
    pushState2UndoStack(fmt::format("set visible on: '{}'", info));
}

void VisualCurveNodeTreeView::slotDelete() {
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Ramses Composer",
        tr("Delete?\n"),
        QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
        QMessageBox::Yes);
    if (resBtn == QMessageBox::No) {
        return;
    }

    std::string info;
    QModelIndexList selectedIndexs = visualCurveTreeView_->selectionModel()->selectedRows();
    std::sort(selectedIndexs.begin(), selectedIndexs.end(), [this](QModelIndex index1, QModelIndex index2) {
        QStandardItem *item1 = model_->itemFromIndex(index1);
        QStandardItem *item2 = model_->itemFromIndex(index2);
        return item1->row() > item2->row();
    });
    for (auto selected : selectedIndexs) {
        QStandardItem *item = model_->itemFromIndex(selected);
        if (item) {
            std::string itemName = curveFromItem(item).toStdString();
            if (!itemName.empty()) {
                if (folderDataMgr_->isCurve(itemName)) {
                    deleteCurve(item);
                } else {
                    deleteFolder(item);
                }
                info += itemName + ";";
            }
        }
    }
    for (int i{0}; i < selectedIndexs.size(); i++) {
        QModelIndex selected = selectedIndexs.at(i);
        QStandardItem *item = model_->itemFromIndex(selected);
        model_->removeRow(selected.row(), selected.parent());
    }
    Q_EMIT sigRefreshVisualCurve();
    pushState2UndoStack(fmt::format("delete curves/nodes: '{}'", info));
}

void VisualCurveNodeTreeView::slotItemChanged(QStandardItem *item) {
    std::string curve = item->text().toStdString();
    std::string curvePath = selNode_;
    Folder *folder{nullptr};
    STRUCT_CURVE_PROP *curveProp{nullptr};
    if (folderDataMgr_->isCurve(curvePath)) {
        if (folderDataMgr_->curveFromPath(curvePath, &folder, &curveProp)) {
            curveProp->curve_ = curve;
            std::string newCurve;
            folderDataMgr_->pathFromCurve(curve, folder, newCurve);
            swapCurve(curvePath, newCurve);
            Q_EMIT signal::signalProxy::GetInstance().sigCheckCurveBindingValid_From_CurveUI();
            pushState2UndoStack(fmt::format("'{}' curve name chang to '{}'", curvePath, curve));
        }
    } else {
        if (folderDataMgr_->folderFromPath(curvePath, &folder)) {
            folder->setFolderName(curve);
            setFolderPath(folder, curvePath);
            Q_EMIT signal::signalProxy::GetInstance().sigCheckCurveBindingValid_From_CurveUI();
            pushState2UndoStack(fmt::format("'{}' folder name chang to '{}'", curvePath, curve));
        }
    }
}

void VisualCurveNodeTreeView::slotCurrentRowChanged(const QModelIndex &index) {
    QStandardItem *item = model_->itemFromIndex(index);
    std::string curvePath = curveFromItem(item).toStdString();
    selNode_ = curvePath;

    std::string curCurve = VisualCurvePosManager::GetInstance().getCurrentPointInfo().first;
    if (folderDataMgr_->isCurve(curvePath)) {
        if (curCurve != curvePath) {
            VisualCurvePosManager::GetInstance().setCurrentPointInfo(curvePath, 0);
            VisualCurvePosManager::GetInstance().setPressAction(MOUSE_PRESS_ACTION::MOUSE_PRESS_KEY);
            Q_EMIT sigSwitchVisualCurveInfoWidget();
            Q_EMIT sigRefreshVisualCurve();
        }
        return;
    }
    VisualCurvePosManager::GetInstance().setCurrentPointInfo(std::string(), 0);
    VisualCurvePosManager::GetInstance().setPressAction(MOUSE_PRESS_ACTION::MOUSE_PRESS_NONE);
    Q_EMIT sigRefreshVisualCurve();
}

void VisualCurveNodeTreeView::slotButtonDelegateClicked(const QModelIndex &index) {
    QStandardItem *item = model_->itemFromIndex(index.siblingAtColumn(0));
    std::string curvePath = curveFromItem(item).toStdString();

    if (folderDataMgr_->isCurve(curvePath)) {
        Folder *folder{nullptr};
        STRUCT_CURVE_PROP *curveProp{nullptr};
        if (folderDataMgr_->curveFromPath(curvePath, &folder, &curveProp)) {
            curveProp->visible_ = !curveProp->visible_;
            if (curveProp->visible_) {
                VisualCurvePosManager::GetInstance().deleteHidenCurve(curvePath);
            } else {
                VisualCurvePosManager::GetInstance().insertHidenCurve(curvePath);
            }
            std::string str = curveProp->visible_ ? "show" : "hide";
            pushState2UndoStack(fmt::format("set '{}' curve '{}'", curvePath, str));
        }
    } else {
        Folder *folder{nullptr};
        if (folderDataMgr_->folderFromPath(curvePath, &folder)) {
            bool visible = !folder->isVisible();
            setFolderVisible(folder, visible);
            std::string str = visible ? "show" : "hide";
            pushState2UndoStack(fmt::format("set '{}' folder '{}'", curvePath, str));
        }
    }
    Q_EMIT sigRefreshVisualCurve();
}

void VisualCurveNodeTreeView::slotDeleteCurveFromVisualCurve(std::string curve) {
    QStringList list = QString::fromStdString(curve).split("|");
    QString root = list.takeFirst();
    QStandardItem *item{nullptr};
    for (int i{0}; i < model_->rowCount(); i++) {
        if (model_->item(i)->text() == root) {
            item = model_->item(i);
        }
    }
    if (!item) {
        return;
    }

    for (const QString &node : list) {
        if (!itemFromPath(&item, node)) {
            return;
        }
    }
    QModelIndex index = item->index();
    model_->removeRow(index.row(), index.parent());

    Folder *folder{nullptr};
    STRUCT_CURVE_PROP *curveProp{nullptr};
    if (folderDataMgr_->isCurve(curve)) {
        if (folderDataMgr_->curveFromPath(curve, &folder, &curveProp)) {
            folder->deleteCurve(curveProp->curve_);
        }
        Q_EMIT sigRefreshVisualCurve();
    }
}

void VisualCurveNodeTreeView::slotModelMoved(std::string dest) {
    pushState2UndoStack(fmt::format("curves move to '{}'", dest));
}

void VisualCurveNodeTreeView::slotRefreshWidget() {
    model_->removeRows(0, model_->rowCount());
    Folder *folder = folderDataMgr_->getRootFolder();

    for (auto curve : folder->getCurveList()) {
        QStandardItem *curveItem = new QStandardItem(QString::fromStdString(curve->curve_));
        model_->appendRow(curveItem);
    }
    for (auto childFolder : folder->getFolderList()) {
        QStandardItem *nodeItem = new QStandardItem(QString::fromStdString(childFolder->getFolderName()));
        model_->appendRow(nodeItem);
        initItemFromFolder(nodeItem, childFolder);
    }
}

void VisualCurveNodeTreeView::searchCurve(NodeData *pNode, std::string &property, std::string curve, std::string sampleProp) {
    if(!pNode)
        return ;

    std::map<std::string, std::string> bindingMap;
    if (pNode->NodeExtendRef().curveBindingRef().getPropCurve(sampleProp, bindingMap)) {
        for (const auto &it : bindingMap) {
            if (it.second.compare(curve)) {
                property = it.first;
                return;
            }
        }
    }

    for (auto it = pNode->childMapRef().begin(); it != pNode->childMapRef().end(); ++it) {
        searchCurve(&(it->second), property, curve, sampleProp);
    }
}

bool VisualCurveNodeTreeView::itemFromPath(QStandardItem **item, QString node) {
    QStandardItem *tempItem = *item;
    for (int i{0}; i < tempItem->rowCount(); i++) {
        QStandardItem *childItem = tempItem->child(i);
        if (childItem->text() == node) {
            *item = childItem;
            return true;
        }
    }
    return false;
}

QString VisualCurveNodeTreeView::curveFromItem(QStandardItem *item) {
    QString curve = item->text();
    while(item->parent()) {
        item = item->parent();
        QString tempStr = item->text() + "|";
        curve.insert(0, tempStr);
    }
    return curve;
}

void VisualCurveNodeTreeView::pushState2UndoStack(std::string description) {
    raco::core::UndoState undoState;
    undoState.saveCurrentUndoState();
    commandInterface_->undoStack().push(description, undoState);
}

void VisualCurveNodeTreeView::initItemFromFolder(QStandardItem *item, Folder *folder) {
    for (auto curve : folder->getCurveList()) {
        QStandardItem *curveItem = new QStandardItem(QString::fromStdString(curve->curve_));
        item->appendRow(curveItem);
    }
    for (auto childFolder : folder->getFolderList()) {
        QStandardItem *nodeItem = new QStandardItem(QString::fromStdString(childFolder->getFolderName()));
        item->appendRow(nodeItem);
        initItemFromFolder(nodeItem, childFolder);
    }
}

void VisualCurveNodeTreeView::swapCurve(std::string oldCurve, std::string newCurve) {
    QList<SKeyPoint> keyPoints;
    VisualCurvePosManager::GetInstance().getKeyPointList(oldCurve, keyPoints);
    VisualCurvePosManager::GetInstance().deleteKeyPointList(oldCurve);
    VisualCurvePosManager::GetInstance().addKeyPointList(newCurve, keyPoints);
}

void VisualCurveNodeTreeView::setFolderPath(Folder *folder, std::string path) {
    if(!folder)
        return ;

    for (const auto &it : folder->getCurveList()) {
        std::string curveName;
        folderDataMgr_->pathFromCurve(it->curve_, folder, curveName);
        std::string oldCurvePath = path + "|" + it->curve_;
        if (CurveManager::GetInstance().getCurve(oldCurvePath)) {
            Curve *tempCurve = CurveManager::GetInstance().getCurve(oldCurvePath);
            tempCurve->setCurveName(curveName);
            swapCurve(oldCurvePath, curveName);
        }
    }

    std::list<Folder *> folderList = folder->getFolderList();
    for (auto it = folderList.begin(); it != folderList.end(); ++it) {
        setFolderPath(*it, std::string(path + "|" + (*it)->getFolderName()));
    }
}

void VisualCurveNodeTreeView::deleteCurve(QStandardItem *item) {
    QModelIndex index = item->index();
    if (item) {
        std::string curve = item->text().toStdString();
        std::string curvePath = curveFromItem(item).toStdString();

        Folder *folder{nullptr};
        STRUCT_CURVE_PROP *curveProp{nullptr};
        if (folderDataMgr_->isCurve(curvePath)) {
            if (folderDataMgr_->curveFromPath(curvePath, &folder, &curveProp)) {
                folder->deleteCurve(curve);
                CurveManager::GetInstance().takeCurve(curvePath);
                VisualCurvePosManager::GetInstance().deleteKeyPointList(curvePath);
            }
        }
    }
}

bool VisualCurveNodeTreeView::sortIndex(const QModelIndex &index1, const QModelIndex &index2) {
//    QModelIndexList indexList;
//    for (auto selected : list) {
//        QStandardItem *item = model_->itemFromIndex(selected);
//        if (indexList.empty()) {
//            indexList.append(selected);
//        } else {
//            bool temp{false};
//            for (int i{0}; i < indexList.size(); i++) {
//                QStandardItem *tempItem = model_->itemFromIndex(indexList[i]);
//                if (item->row() > tempItem->row()) {
//                    indexList.insert(i, selected);
//                    temp = true;
//                    break;
//                }
//            }
//            if (!temp) {
//                indexList.push_front(selected);
//            }
//        }
//    }
//    return indexList;
    QStandardItem *item1 = model_->itemFromIndex(index1);
    QStandardItem *item2 = model_->itemFromIndex(index2);
    return item1->row() < item2->row();
}

void VisualCurveNodeTreeView::deleteFolder(QStandardItem *item) {
    QModelIndex index = item->index();
    std::string folderName = item->text().toStdString();
    if (item) {
        std::string curve = curveFromItem(item).toStdString();
        if (!folderDataMgr_->isCurve(curve)) {
            Folder *folder{nullptr};
            if (folderDataMgr_->folderFromPath(curve, &folder)) {
                for (auto it : folder->getCurveList()) {
                    std::string curveName;
                    folderDataMgr_->pathFromCurve(it->curve_, folder, curveName);
                    CurveManager::GetInstance().takeCurve(curveName);
                    VisualCurvePosManager::GetInstance().deleteKeyPointList(curveName);
                }
                folder->parent()->deleteFolder(folderName);
            }
        }
    }
}
}
