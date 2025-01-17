﻿#ifndef VISUALCURVEINFOWIDGET_H
#define VISUALCURVEINFOWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLayout>
#include <QStackedWidget>
#include <QSpacerItem>
#include <QFormLayout>
#include "common_editors/DoubleEditor.h"
#include "common_editors/Int64Editor.h"
#include "core/CommandInterface.h"
#include "time_axis/TimeAxisCommon.h"

using namespace raco::time_axis;
using namespace raco::common_editors;
namespace raco::visualCurve {
class VisualCurveInfoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VisualCurveInfoWidget(QWidget *parent, raco::core::CommandInterface* commandInterface);

    void initVisualCurveKeyWidget();
    void initVisualCurveCursorWidget();
    void setKeyWidgetVisible();
    void setCursorWidgetVisible();

public Q_SLOTS:
    void slotSwitchInterpolationMode(int index);
    void slotKeyFrameChanged(int value);
    void slotKeyValueChanged(double value);
    void slotLeftKeyFrameChanged(double value);
    void slotLeftKeyValueChanged(double value);
    void slotRightKeyFrameChanged(double value);
    void slotRightKeyValueChanged(double value);
    void slotLeftTangentChanged(double value);
    void slotRightTangentChanged(double value);
    void slotSwitchHandleType(int index);

    void slotKeyFrameFinished();
    void slotKeyValueFinished();
    void slotLeftKeyFrameFinished();
    void slotLeftKeyValueFinished();
    void slotRightKeyFrameFinished();
    void slotRightKeyValueFinished();
    void slotLeftTangentFinished();
    void slotRightTangentFinished();

    void slotCursorShow(bool checked);
    void slotCursorXChanged(int value);
    void slotCursorYChanged(double value);
    void slotCurveScaleChanged(double value);
    void slotCursorXFinished();
    void slotCursorYFinished();
    void slotCurveScaleFinished();

    void slotUpdateSelKey();
    void slotUpdateCursorX();
    void slotRefreshWidget();

signals:
    void sigRefreshCursorX();
    void sigRefreshVisualCurve();
    void sigUpdateCurvePoints();
    void sigSwitchCurveType(int type);
    void sigUpdateCursorX(int cursorX);
private:
    void switchCurveType(int type, bool isShowLeftPoint = false);
    void updateSelKey();
    void updateCursorX();
    void recaculateWorkerLeftPoint(QPair<QPointF, QPointF> &workerPoint, SKeyPoint keyPoint);
    void recaculateWorkerRightPoint(QPair<QPointF, QPointF> &workerPoint, SKeyPoint keyPoint);
    void pushState2UndoStack(std::string description);
private:
    QWidget *visualCurveKeyWidget_{nullptr};
    QWidget *visualCurveCursorWidget_{nullptr};
    QStackedWidget *centreWidget_{nullptr};

    QComboBox *interpolationComboBox_{nullptr};
    Int64Editor *keyFrameSpinBox_{nullptr};
    DoubleEditor *keyValueSpinBox_{nullptr};

    QLabel *leftValueLabel_{nullptr};
    QLabel *rightValueLabel_{nullptr};
    QLabel *leftFrameLabel_{nullptr};
    QLabel *rightFrameLabel_{nullptr};
    QLabel *leftTangentLabel_{nullptr};
    QLabel *rightTangentLabel_{nullptr};
    QLabel *handleTypeLabel_{nullptr};
    DoubleEditor *leftFrameSpinBox_{nullptr};
    DoubleEditor *leftTangentSpinBox_{nullptr};
    DoubleEditor *leftValueSpinBox_{nullptr};
    DoubleEditor *rightFrameSpinBox_{nullptr};
    DoubleEditor *rightTangentSpinBox_{nullptr};
    DoubleEditor *rightValueSpinBox_{nullptr};

    QGridLayout *cursorLayout_{nullptr};
    QCheckBox *showCursorCheckBox_{nullptr};
    Int64Editor *cursorXSpinBox_{nullptr};
    DoubleEditor *cursorYSpinBox_{nullptr};
    DoubleEditor *curveScaleSpinBox_{nullptr};
    QComboBox *handleTypeComboBox_{nullptr};
    raco::core::CommandInterface *commandInterface_;
};
}

#endif // VISUALCURVEINFOWIDGET_H
