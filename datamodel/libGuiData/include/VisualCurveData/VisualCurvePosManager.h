#ifndef VISUALCURVEDATAMANAGER_H
#define VISUALCURVEDATAMANAGER_H

#include <QMap>
#include <QList>
#include "core/ChangeBase.h"
#include "core/StructCommon.h"

namespace raco::guiData {
using namespace raco::time_axis;
class VisualCurvePosManager  {
public:
    static VisualCurvePosManager& GetInstance();
    VisualCurvePosManager(const VisualCurvePosManager&) = delete;
    VisualCurvePosManager& operator=(const VisualCurvePosManager&) = delete;

    STRUCT_VISUAL_CURVE_POS convertDataStruct();
    void merge(QVariant data);

    void setKeyBoardType(KEY_BOARD_TYPE type);
    KEY_BOARD_TYPE getKeyBoardType();

    double getCenterLinePos();
    void setCenterLinePos(double value);

    double getCurveScale();
    void setCurveScale(double value);

    int getCurFrame();
    void setCurFrame(int frame);

    int getCurX();
    int getCurY();
    double getEachFrameWidth();
    double getEachValueWidth();

    void setCurPos(int x, int y);
    void setWidth(double frameWidth, double valueWidth);

    void setCursorShow(bool show);
    bool getCursorShow();

    void insertSameKeyPoint(int index, SAME_KEY_TYPE type);
    QMap<int, SAME_KEY_TYPE> getSameKeyPointsInfo();
    void clearSameKeyPoints();

    SAME_KEY_TYPE getSameKeyType();
    void setSameKeyType(SAME_KEY_TYPE type);

    int getSameKeyPointIndex();
    void setSameKeyPointIndex(int index);

    MOUSE_PRESS_ACTION getPressAction();
    void setPressAction(MOUSE_PRESS_ACTION action);

    void setGroupCurve(std::string curve);
    bool isGroupCurve(std::string curve);

    void insertHidenCurve(std::string curve);
    void deleteHidenCurve(std::string curve);
    bool hasHidenCurve(std::string curve);
    void clearHidenCurve();

    void resetCurrentPointInfo();
    QPair<std::string, int> getCurrentPointInfo();

    void setCurrentPointInfo(std::string curve, int index);
    void setCurrentPointInfo(std::string curve);
    void setCurrentPointInfo(int index);

    void clearKeyPointMap();
    QMap<std::string, QList<SKeyPoint>> getKeyPointMap();
    void setKeyPointMap(QMap<std::string, QList<SKeyPoint>> map);
    void addKeyPointList(std::string curve, QList<SKeyPoint> points);
    void deleteKeyPointList(std::string curve);
    bool getCurKeyPoint(SKeyPoint &point);
    bool insertKeyPoint(int index, std::string curve, SKeyPoint keyPoint);
    bool getKeyPoint(std::string curve, int index, SKeyPoint &point);
    void replaceCurKeyPoint(SKeyPoint point);
    bool getCurKeyPointList(QList<SKeyPoint> &pointList);
    bool getKeyPointList(std::string curve, QList<SKeyPoint> &points);
    void swapCurKeyPointList(QList<SKeyPoint> pointList);

    void addMultiSelPoint(int index);
    bool hasMultiSelPoint(int index);
    void delMultiSelPoint(int index);
    QList<int> getMultiSelPoints();
    void clearMultiSelPoints();
private:
    VisualCurvePosManager();
private:
    KEY_BOARD_TYPE keyBoardType_{KEY_BOARD_TYPE::POINT_MOVE};
    int curFrame_{0};
    int curX_{0};
    int curY_{0};
    double centerLinePos_{0.0f};
    double eachFrameWidth_{0.0f};
    double eachValueWidth_{0.0f};
    double curveScale_{1.0f};
    bool cursorShow_{true};
    QMap<int, SAME_KEY_TYPE> sameKeyPointsInfo_;
    SAME_KEY_TYPE sameKeyType_{SAME_NONE};
    int sameKeyPointIndex_{0};
    MOUSE_PRESS_ACTION pressAction_;
    QPair<std::string, int> curPointInfo_;
    QMap<std::string, QList<SKeyPoint>> keyPointMap_;
    QList<std::string> hidenCurveList_;
    QList<int> multiSelPoints_;
    std::string groupCurve_;
};
}

#endif // VISUALCURVEDATAMANAGER_H
