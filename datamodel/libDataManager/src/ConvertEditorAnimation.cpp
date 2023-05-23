#include "animation_editor/ConvertEditorAnimation.h"
#include "utils/MathUtils.h"


#define PI 3.141592653589793238462643f
static const int rotationSize{4};

double fixRotationValue(double value) {
    double decimal = value - int(value);
    int integer = ((int)value + 360) % 360;
    value = integer + decimal;
    return value;
}

ConvertEditorAnimation::ConvertEditorAnimation(raco::core::CommandInterface *commandInterface, QObject *parent)
    : QObject{parent} ,
      commandInterface_(commandInterface) {
    connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigUpdateGltfAnimation, this, &ConvertEditorAnimation::slotUpdateGltfAnimation);
}

void ConvertEditorAnimation::commandInterface(raco::core::CommandInterface *commandInterface) {
    commandInterface_ = commandInterface;
}

void ConvertEditorAnimation::slotUpdateGltfAnimation(const std::set<raco::core::ValueHandle> &handles, QString fileName) {
    animationChannels_.clear();
    animationNodes_.clear();
	fileName.remove(QRegExp("\\s"));
    std::string animation = fileName.section(".gltf", 0, 0).toStdString();
    std::set<std::string> animationIDs;

    for (const auto &valueHandle : handles) {
        int outPutIndex{0};
        for (int i{static_cast<int>(valueHandle.size()-1)}; i >= 0; i--) {
            raco::core::ValueHandle tempHandle = valueHandle[i];
            if (tempHandle.getPropName().compare(GLTF_ANIMATION_OUTPUTS) == 0) {
                if (tempHandle.type() == raco::core::PrimitiveType::Table) {
                    for (int j{0}; j < tempHandle.size(); ++j) {
                        raco::core::ValueHandle aniHandle = tempHandle[j];

                        // get link node object ID map by handle; key-path value-object ID
                        auto linkEnds = raco::core::Queries::getLinksConnectedToProperty(*commandInterface_->project(), aniHandle, true, false);
                        std::map<std::string, std::string> sortedLinkEnds;
                        for (const auto& linkEnd : linkEnds) {
                            auto linkDesc = linkEnd->descriptor();
                            sortedLinkEnds[linkDesc.end.getFullPropertyPath()] = linkDesc.end.object()->objectID();
                        }
                        if (sortedLinkEnds.size() > 0) {
                            auto it = sortedLinkEnds.begin();
                            animationNodes_.push_back(it->first);
                            outPutIndex++;
                        }
                    }
                }
            }
            if (tempHandle.getPropName().compare(GLTF_ANIMATION_CHANNELS) == 0) {
                if (outPutIndex != 0) {
                    if (tempHandle.type() == raco::core::PrimitiveType::Table) {
                        for (int j{0}; j < tempHandle.size(); ++j) {
                            raco::core::ValueHandle aniHandle = tempHandle[j];
                            // get animation channel data
                            if (aniHandle.type() == raco::core::PrimitiveType::Ref) {
                                aniHandle = aniHandle.asRef();
                                raco::user_types::AnimationChannel *aniChannel = dynamic_cast<raco::user_types::AnimationChannel *>(aniHandle.rootObject().get());
                                animationChannels_.push_back(aniChannel);
                            }
                        }
                    }
                }
            }
        }
        std::string animationID = valueHandle[0].asString();
        animationIDs.emplace(animationID);
    }

    updateGltfAnimation(animation);
    Q_EMIT raco::signal::signalProxy::GetInstance().sigDeleteAniamtionNode(animationIDs);
}

void ConvertEditorAnimation::updateGltfAnimation(std::string animation) {
    if (animation.empty()) {
        curAnimation_ = "";
        return;
    }
    curAnimation_ = QString::fromStdString(animation);
    if (animationChannels_.size() == animationNodes_.size()) {
        // create animation
        raco::guiData::animationDataManager::GetInstance().InsertAmimation(animation);
        raco::guiData::animationDataManager::GetInstance().SetActiveAnimation(animation);
        Q_EMIT raco::signal::signalProxy::GetInstance().sigUpdateActiveAnimation_From_AnimationLogic(curAnimation_);
        Q_EMIT raco::signal::signalProxy::GetInstance().sigInitAnimationView();
        //
        for (int i{0}; i < animationChannels_.size(); ++i) {
            raco::user_types::AnimationChannel *aniChannel = animationChannels_.at(i);
            std::string path = animationNodes_.at(i);
            QString qstrNode = QString::fromStdString(path).section("/", -1);
            std::string node = qstrNode.split(".").at(0).toStdString();
            std::string property = qstrNode.split(".").at(1).toStdString();

            raco::guiData::NodeData *nodeData = raco::guiData::NodeDataManager::GetInstance().searchNodeByName(node);
            if (nodeData && aniChannel) {
                std::vector<float> keyFrames = aniChannel->currentSamplerData_.get()->input;
                std::vector<std::vector<float>> propertyData = aniChannel->currentSamplerData_.get()->output;
                raco::core::MeshAnimationInterpolation interpolation = aniChannel->currentSamplerData_.get()->interpolation;

                // insert curves
                updateOneGltfCurve(nodeData, keyFrames, propertyData, interpolation, property, node);
            }
        }
    }
    Q_EMIT raco::signal::signalProxy::GetInstance().sigInitCurveView();
    Q_EMIT raco::signal::signalProxy::GetInstance().sigRepaintTimeAixs_From_CurveUI();
}


std::array<double, 3> XYZEul_FromHMatrix(float M[][4]) {
	std::array<double, 3> ea;
	int i = 0, j = 1, k = 2;
    double cy = sqrtf(M[i][i] * M[i][i] + M[j][i] * M[j][i]);
	if (cy > 16 * FLT_EPSILON) {
		ea[0] = atan2f(M[k][j], M[k][k]);
		ea[1] = atan2f(-M[k][i], cy);
		ea[2] = atan2f(M[j][i], M[i][i]);
	} else {
		ea[0] = atan2f(-M[j][k], M[j][j]);
		ea[1] = atan2f(-M[k][i], cy);
		ea[2] = 0;
	}

	ea[0] = ea[0] * 180.0 / PI;
	ea[1] = ea[1] * 180.0 / PI;
	ea[2] = ea[2] * 180.0 / PI;

	return ea;
}

std::array<double, 3> Eul_FromQuat(float x, float y, float z, float w) {
	float M[4][4];
	int X = 0,Y = 1,Z = 2,W = 3;
	float Nq = x * x + y * y + z * z + w * w;
	float s = (Nq > 0.f) ? (2.f / Nq) : 0.f;
	float xs = x * s, ys = y * s, zs = z * s;
	float wx = w * xs, wy = w * ys, wz = w * zs;
	float xx = x * xs, xy = x * ys, xz = x * zs;
	float yy = y * ys, yz = y * zs, zz = z * zs;
	M[X][X] = 1.f - (yy + zz);
	M[X][Y] = xy - wz;
	M[X][Z] = xz + wy;
    M[Y][X] = xy + wz;
	M[Y][Y] = 1.f - (xx + zz);
	M[Y][Z] = yz - wx;
	M[Z][X] = xz - wy;
	M[Z][Y] = yz + wx;
	M[Z][Z] = 1.f - (xx + yy);
	M[W][X] = M[W][Y] = M[W][Z] = M[X][W] = M[Y][W] = M[Z][W] = 0.f;
	M[W][W] = 1.f;
	return (XYZEul_FromHMatrix(M));
}


void ConvertEditorAnimation::updateOneGltfCurve(raco::guiData::NodeData *nodeData, std::vector<float> keyFrames, std::vector<std::vector<float>> propertyData, raco::core::MeshAnimationInterpolation interpolation, std::string property, std::string node)
{
    auto insertBindingItem = [=](std::string prop, std::string curve)->bool {
        std::string animation = curAnimation_.toStdString();
        std::map<std::string, std::string> bindingMap;
        if (nodeData->NodeExtendRef().curveBindingRef().getPropCurve(animation, bindingMap)) {
            auto it = bindingMap.find(prop);
            if (it != bindingMap.end()) {
                return false;
            }
            nodeData->NodeExtendRef().curveBindingRef().insertBindingDataItem(animation, prop, curve);
        } else {
            std::map<std::string, std::string> bindingMap;
            bindingMap.emplace(prop, curve);
            nodeData->NodeExtendRef().curveBindingRef().insertAnimation(animation, bindingMap);
        }
        return true;
    };

    if (keyFrames.empty()) {
        return;
    }

    std::string propX = property + PROP_X;
    std::string propY = property + PROP_Y;
    std::string propZ = property + PROP_Z;
    std::string curveX = curAnimation_.toStdString() + SYMBOL_UNDERLINE + node + SYMBOL_POINT + propX;
    std::string curveY = curAnimation_.toStdString() + SYMBOL_UNDERLINE + node + SYMBOL_POINT + propY;
    std::string curveZ = curAnimation_.toStdString() + SYMBOL_UNDERLINE + node + SYMBOL_POINT + propZ;

    float lastX{0},lastY{0},lastZ{0};
    float lastEulerX{0},lastEulerY{0},lastEulerZ{0};
    int invalidIndexX{0},invalidIndexY{0},invalidIndexZ{0};

    for (int i{0}; i < keyFrames.size(); ++i) {
        int keyFrame = qRound(keyFrames.at(i) * 24);
        std::vector<float> data = propertyData.at(i);
        bool isValid{false};
        if (i + 1 < keyFrames.size()) {
            std::vector<float> nextData = propertyData.at(i + 1);
            if (nextData != data) {
                isValid = true;
            }
        }

        // calculate rotation property data
        if (data.size() == rotationSize) {
            auto rotation = Eul_FromQuat(data[ROTATION_X], data[ROTATION_Y], data[ROTATION_Z], data[ROTATION_W]);
            auto eulerRotation = raco::utils::math::eulerAngle(lastEulerX, lastEulerY, lastEulerZ, rotation[ROTATION_X], rotation[ROTATION_Y], rotation[ROTATION_Z]);

            lastEulerX = eulerRotation[ROTATION_X];
            lastEulerY = eulerRotation[ROTATION_Y];
            lastEulerZ = eulerRotation[ROTATION_Z];

            insertBindingItem(propX, curveX);
            insertBindingItem(propY, curveY);
            insertBindingItem(propZ, curveZ);
            insertCurve(keyFrame, eulerRotation[ROTATION_X], curveX, interpolation);
            insertCurve(keyFrame, eulerRotation[ROTATION_Y], curveY, interpolation);
            insertCurve(keyFrame, eulerRotation[ROTATION_Z], curveZ, interpolation);
            continue;
        }

        // insert translation/scale property data
        if ((data[ROTATION_X] != lastX || isValid) || i == 0) {
            insertBindingItem(propX, curveX);
            insertCurve(keyFrame, data[ROTATION_X], curveX, interpolation);
            lastX = data[ROTATION_X];
            invalidIndexX++;
        }
        if ((data[ROTATION_Y] != lastY || isValid) || i == 0) {
            insertBindingItem(propY, curveY);
            insertCurve(keyFrame, data[ROTATION_Y], curveY, interpolation);
            lastY = data[ROTATION_Y];
            invalidIndexY++;
        }
        if ((data[ROTATION_Z] != lastZ || isValid) || i == 0) {
            insertBindingItem(propZ, curveZ);
            insertCurve(keyFrame, data[ROTATION_Z], curveZ, interpolation);
            lastZ = data[ROTATION_Z];
            invalidIndexZ++;
        }
    }
    // handle invalid property data
    if (invalidIndexX == 1) {
        nodeData->NodeExtendRef().curveBindingRef().deleteBindingDataItem(curAnimation_.toStdString(), propX, curveX);
        if (raco::guiData::CurveManager::GetInstance().getCurve(curveX)) {
            raco::guiData::CurveManager::GetInstance().takeCurve(curveX);
        }
    }
    if (invalidIndexY == 1) {
        nodeData->NodeExtendRef().curveBindingRef().deleteBindingDataItem(curAnimation_.toStdString(), propY, curveY);
        if (raco::guiData::CurveManager::GetInstance().getCurve(curveY)) {
            raco::guiData::CurveManager::GetInstance().takeCurve(curveY);
        }
    }
    if (invalidIndexZ == 1) {
        nodeData->NodeExtendRef().curveBindingRef().deleteBindingDataItem(curAnimation_.toStdString(), propZ, curveZ);
        if (raco::guiData::CurveManager::GetInstance().getCurve(curveZ)) {
            raco::guiData::CurveManager::GetInstance().takeCurve(curveZ);
        }
    }
}

bool ConvertEditorAnimation::insertCurve(int keyFrame, float data, std::string curve, raco::core::MeshAnimationInterpolation interpolation) {
    raco::guiData::Curve *curveData = raco::guiData::CurveManager::GetInstance().getCurve(curve);
    if (curveData == nullptr) {
        curveData = new raco::guiData::Curve();
    }
    curveData->setCurveName(curve);
    curveData->setDataType(raco::guiData::EDataType::Type_FLOAT);
    raco::guiData::Point *point = new raco::guiData::Point;
    point->setKeyFrame(keyFrame);
    point->setDataValue(static_cast<double>(data));

    raco::guiData::EInterPolationType type = raco::guiData::EInterPolationType::LINER;

    switch(interpolation) {
    case raco::core::MeshAnimationInterpolation::Linear:{
        type = raco::guiData::EInterPolationType::LINER;
        break;
    }
    case raco::core::MeshAnimationInterpolation::Step:{
        type = raco::guiData::EInterPolationType::STEP;
        break;
    }
    case raco::core::MeshAnimationInterpolation::CubicSpline: {
        type = raco::guiData::EInterPolationType::BESIER_SPLINE;
        break;
    }
    }
    point->setInterPolationType(type);
    curveData->insertPoint(point);
    raco::guiData::CurveManager::GetInstance().addCurve(curveData);

    return true;
}
