#ifndef TIMEAXISMAINWINDOW_H
#define TIMEAXISMAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QToolBar>
#include <QMessageBox>
#include <QMenu>
#include "components/DataChangeDispatcher.h"
#include "core/CommandInterface.h"
#include "qstackedwidget.h"
#include "time_axis/TimeAxisWidget.h"
#include "time_axis/TimeAxisScrollArea.h"
#include "time_axis/AnimationEditorView.h"
#include "signal/SignalProxy.h"
#include "visual_curve/VisualCurveScrollArea.h"
#include "visual_curve/VisualCurveWidget.h"
#include "visual_curve/VisualCurveNodeTreeView.h"
#include "visual_curve/VisualCurveInfoWidget.h"
#include <QItemSelectionModel>
#include <QDockWidget>

using namespace raco::signal;
using namespace raco::visualCurve;
namespace Ui {
class TimeAxisMainWindow;
}

namespace raco::time_axis {

enum CURVE_TYPE_ENUM {
    TIME_AXIS,
    VISUAL_CURVE
};

class TimeAxisMainWindow final : public QWidget {
    Q_OBJECT
public:
    TimeAxisMainWindow(raco::components::SDataChangeDispatcher dispatcher,
                       raco::core::CommandInterface* commandInterface,
                       QWidget* parent = nullptr);

public Q_SLOTS:
    void slotInitAnimationMgr();
    //
    void slotInsertCurve(QString property, QString curve, QVariant value);
    void slotRefreshTimeAxis();
    void slotRefreshTimeAxisAfterUndo();
    void slotInitCurves();
    void slotSwitchNode(raco::core::ValueHandle &handle);
    void startAnimation();
    void slotUpdateAnimation();
    void slotUpdateAnimationKey(QString oldKey, QString newKey);
    void slotResetAnimation();
    void slotSwitchCurveWidget();
    void slotPressKey();
    void slotSwitchVisualCurveInfoWidget();
private Q_SLOTS:
    void slotTreeMenu(const QPoint &pos);
    void slotLoad();
    void slotCopy();
    void slotPaste();
    void slotDelete();
    void slotProperty();
    void slotCreateNew();
    void slotCurrentRowChanged(const QModelIndex &index);
    void slotItemChanged(QStandardItem *item);
    void slotStartTimeFinished();
    void slotEndTimeFinished();
private:
    bool initTitle(QWidget *parent);
    bool initTree(QWidget *parent);
	bool initAnimationMenu();
    void loadAnimation();

private:
    TimeAxisWidget *timeAxisWidget_;
    VisualCurveWidget *visualCurveWidget_;
    TimeAxisScrollArea *timeAxisScrollArea_;
    VisualCurveScrollArea *visualCurveScrollArea_;
    QStackedWidget *stackedWidget_;
    QVBoxLayout *vBoxLayout_;
    QHBoxLayout *hBoxLayout_;
    QHBoxLayout *hTitleLayout_;
    raco::core::CommandInterface *commandInterface_;

    QWidget *titleWidget_;
    AnimationEditorView *editorView_;
    QPushButton *startBtn_;
    QPushButton *nextBtn_;
    QPushButton *previousBtn_;
    bool animationStarted_{false};

    QStackedWidget *leftStackedWidget_;
    VisualCurveNodeTreeView *visualCurveNodeTreeView_{nullptr};
    VisualCurveInfoWidget *visualCurveInfoWidget_{nullptr};

	QMenu m_Menu;
    QAction *m_pLoad{nullptr};
    QAction *m_pCopy{nullptr};
    QAction *m_pDelete{nullptr};
    QAction *m_pProperty{nullptr};
    QAction *pasteAction_{nullptr};
private:
    int UUID_{1};
    QStandardItemModel* model_{nullptr};
    QString curItemName_;
    QString copyItemName_;
    QMap<QString, QStandardItem*> itemMap_;
    Int64Editor *lineBegin_{nullptr};
    Int64Editor *lineEnd_{nullptr};
    KeyFrameManager *keyFrameMgr_{nullptr};
    CURVE_TYPE_ENUM curCurveType_ = CURVE_TYPE_ENUM::VISUAL_CURVE;
    DragPushButton *button_{nullptr}; //时间轴滑动条
};
}

#endif // TIMEAXISMAINWINDOW_H
