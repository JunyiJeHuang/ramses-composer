#ifndef CURVEDATA_H
#define CURVEDATA_H

#include <list>
#include <mutex>
#include <string>
#include <any>
#include <map>
#include <regex>

namespace raco::guiData {

enum EDataType{
    Type_FLOAT,
    Type_VEC2,
    Type_VEC3,
    Type_VEC4
};

enum EInterPolationType{
    LINER,
    HERMIT_SPLINE,
    BESIER_SPLINE,
    STEP
};

class Point
{
public:
    Point(int keyFrame = 0);

    //
    void setKeyFrame(const int& keyFrame);
    //
    int getKeyFrame();
    //
    void setInterPolationType(const EInterPolationType& interPolationType);
    //
    EInterPolationType getInterPolationType();
    //
    void setDataValue(const std::any& value);
    //
    std::any getDataValue();
    //
    void setLeftTagent(const std::any& value);
    //
    std::any getLeftTagent();
    //
    void setRightTagent(const std::any& value);
    //
    std::any getRightTagent();
    //
    void setLeftData(const std::any &value);
    //
    std::any getLeftData();
    //
    void setLeftKeyFrame(const double keyFrame);
    //
    double getLeftKeyFrame();
    //
    void setRightData(const std::any &value);
    //
    std::any getRightData();
    //
    void setRightKeyFrame(const double keyFrame);
    //
    double getRightKeyFrame();

private:
    int keyFrame_;
    EInterPolationType interPolationType_{LINER};
    std::any data_{0.0}; //
    std::any leftTagent_{0.0};
    std::any rightTagent_{0.0};
    std::any leftData_{(double)INT_MIN};
    double leftKeyFrame_{INT_MIN};
    std::any rightData_{(double)INT_MIN};
    double rightKeyFrame_{INT_MIN};
};

class Curve
{
public:
    Curve();
    ~Curve();

    // Curve Name
    void setCurveName(const std::string& curveName);
    // Curve Name
    std::string getCurveName();
    //
    void setDataType(const EDataType& dataType);
    //
    EDataType getDataType();
    //
    bool insertPoint(Point* point);
    //
    bool insertSamePoint(Point* point);
    //
    bool delPoint(int keyFrame);
    bool takePoint(int keyFrame);
    bool delSamePoint(int keyFrame);
    //
    std::list<Point*> getPointList();
    //
    Point* getPoint(int keyFrame);
    //
    bool sortPoint();
    //
    bool getDataValue(int curFrame, double &value);
    //
    bool getStepValue(int curFrame, double &value);
    //
    bool getPointType(int curFrame, EInterPolationType &type);
    //
    double calculateLinerValue(Point* firstPoint, Point* secondPoint, double curFrame);
    //
    bool modifyPointKeyFrame(const int& keyFrame, const int& modifyKeyFrame);
    //
    void clear();

private:
    std::string curveName_;
    EDataType dataType_{Type_FLOAT};
    std::list<Point*> pointList_;
};
}

#endif // CURVEDATA_H
