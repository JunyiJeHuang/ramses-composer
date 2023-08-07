#ifndef GLTFANIMATIONMANAGER_H
#define GLTFANIMATIONMANAGER_H

#include <QObject>
#include "signal/SignalProxy.h"
#include "user_types/Animation.h"
#include "user_types/AnimationChannel.h"
#include "core/Queries.h"
#include "NodeData/nodeManager.h"
#include "AnimationData/animationData.h"
#include "CurveData/CurveManager.h"

#define SYMBOL_POINT        std::string(".")
#define SYMBOL_UNDERLINE    std::string("_")
#define PROP_X              std::string(".x")
#define PROP_Y              std::string(".y")
#define PROP_Z              std::string(".z")
#define GLTF_OBJECT_NAME            std::string("objectName")
#define GLTF_ANIMATION_CHANNELS     std::string("animationChannels")
#define GLTF_ANIMATION_OUTPUTS      std::string("outputs")
#define ROTATION_X          0
#define ROTATION_Y          1
#define ROTATION_Z          2
#define ROTATION_W          3

using namespace raco::guiData;

class ConvertEditorAnimation : public QObject {
    Q_OBJECT
public:
    explicit ConvertEditorAnimation(raco::core::CommandInterface* commandInterface, QObject *parent = nullptr);
    void commandInterface(raco::core::CommandInterface* commandInterface);
public Q_SLOTS:
    void slotUpdateGltfAnimation(const std::set<raco::core::ValueHandle> &handles, QString name, bool filter);
private:
    void updateGltfAnimation(std::string animation, bool filter);
    void preOrderReverse(NodeData *pNode, const std::string &animation, std::list<Curve *> curveList);
    void updateOneGltfCurve(raco::guiData::NodeData *nodeData, std::vector<float> keyFrames, std::vector<std::vector<float>> propertyData, raco::core::MeshAnimationInterpolation interpolation, std::string property, std::string node);
    bool insertCurve(int keyFrame, float data, std::string curve, raco::core::MeshAnimationInterpolation interpolation);
private:
    raco::core::CommandInterface* commandInterface_{nullptr};
    std::vector<raco::user_types::AnimationChannel *> animationChannels_;
    std::vector<std::string> animationNodes_;
    QString curAnimation_;
};

#endif // GLTFANIMATIONMANAGER_H
