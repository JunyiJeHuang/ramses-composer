#include "node_logic/NodeLogic.h"
#include "PropertyData/PropertyType.h"
#include "time_axis/TimeAxisCommon.h"
#include "VisualCurveData/VisualCurvePosManager.h"
#include "RenderData/RenderDataManager.h"
#include <QDebug>

namespace raco::node_logic {
NodeLogic::NodeLogic(raco::core::CommandInterface *commandInterface, QObject *parent)
    : QObject{parent}, commandInterface_{commandInterface} {
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateKeyFram_From_AnimationLogic, this, &NodeLogic::slotUpdateKeyFrame);
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateActiveAnimation_From_AnimationLogic, this, &NodeLogic::slotUpdateActiveAnimation);
    connect(&signalProxy::GetInstance(), &signalProxy::sigResetAllData_From_MainWindow, this, &NodeLogic::slotResetNodeData,Qt::DirectConnection);
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateNodeProp_From_ObjectView, this, &NodeLogic::slotUpdateNodeProperty);
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateMeshNodeTransProperty, this, &NodeLogic::slotUpdateMeshNodeTranslation);
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateMeshNodeRotationProperty, this, &NodeLogic::slotUpdateMeshNodeRotation);
    connect(&signalProxy::GetInstance(), &signalProxy::sigUpdateMeshNodeScalingProperty, this, &NodeLogic::slotUpdateMeshNodeScaling);
}

void NodeLogic::setCommandInterface(core::CommandInterface *commandInterface) {
    commandInterface_ = commandInterface;
}

void NodeLogic::analyzing(NodeData *pNode) {
	if (!pNode )
		return;
    handleMapMutex_.lock();
	if (pNode->objectID() != "" && pNode->objectID() != "objectID") {
		auto it = nodeObjectIDHandleReMap_.find(pNode->objectID());
		if (it != nodeObjectIDHandleReMap_.end()) {
			raco::core::ValueHandle valueHandle = nodeObjectIDHandleReMap_.find(pNode->objectID())->second;
			initBasicProperty(valueHandle, pNode);
		}
	}
    handleMapMutex_.unlock();

	for (auto it = pNode->childMapRef().begin(); it != pNode->childMapRef().end(); ++it) {
        analyzing(&(it->second));
    }
}

bool NodeLogic::getValueHanlde(std::string property, core::ValueHandle &valueHandle) {

    QString qstrPropety = QString::fromStdString(property);
    QStringList list = qstrPropety.split(".");
	if (!list.isEmpty() && list.contains("uniforms")) {
		property = "materials.material." + property;
		qstrPropety = QString::fromStdString(property);
		list = qstrPropety.split(".");
	}

    auto func = [&](core::ValueHandle &tempHandle, std::string tempProp)->bool {
          if (tempHandle.hasProperty(tempProp)) {
              tempHandle = tempHandle.get(tempProp);
          } else {
              return false;
          }
          return tempHandle.isProperty();
    };

    bool bValid = false;
    if (valueHandle.isObject()) {
        for (int i{0}; i < list.size(); i++) {
            QString str = list[i];
			bValid = func(valueHandle, str.toStdString());
			if (!bValid) {
                i++;
				if (i >= list.size())
                    break;
                str += "." + list[i];
				bValid = func(valueHandle, str.toStdString());
				if (!bValid)
                    break;
            }
        }
    }
	return bValid;
}

void NodeLogic::setProperty(core::ValueHandle handle, std::string property, float value) {
    if (getValueHanlde(property, handle) && commandInterface_) {
        commandInterface_->set(handle, value, false);
    }
}

std::map<std::string, core::ValueHandle> &NodeLogic::getNodeNameHandleReMap() {
    QMutexLocker locker(&handleMapMutex_);
    return nodeObjectIDHandleReMap_;
}

void NodeLogic::analyzeRenderHandles(const std::map<std::string, core::ValueHandle> &handles) {
    RenderDataManager::GetInstance().clear();
    for (const auto &it : handles) {
        std::string objectID = it.first;
        raco::core::ValueHandle handle = it.second;
        if (handle.rootObject()->isType<raco::user_types::RenderPass>()) {
            analyzeRenderPass(objectID, handle);
        } else if (handle.rootObject()->isType<raco::user_types::RenderTarget>()) {
            analyzeRenderTarget(objectID, handle);
        } else if (handle.rootObject()->isType<raco::user_types::RenderLayer>()) {
            analyzeRenderLayer(objectID, handle);
        } else if (handle.rootObject()->isType<raco::user_types::RenderBuffer>()) {
            analyzeRenderBuffer(objectID, handle);
        }
    }
}

void NodeLogic::analyzeRenderPass(const std::string &id, const core::ValueHandle &handle) {
    RenderPass renderPass;
    for (int i = 0; i < handle.size(); i++) {
        core::ValueHandle tempHandle = handle[i];
        QString property = QString::fromStdString(tempHandle.getPropName());
        if (property.compare("objectName") == 0) {
            std::string name = tempHandle.asString();
            renderPass.setObjectName(name);
        } else if (property.compare("target") == 0) {
            raco::core::ValueHandle targetHandle = tempHandle.asRef();
            if (targetHandle) {
                std::string target = targetHandle[0].asString();
                renderPass.setRenderTarget(target);
            }
        } else if (property.compare("camera") == 0) {
            raco::core::ValueHandle cameraHandle = tempHandle.asRef();
            if (cameraHandle && cameraHandle.hasProperty("objectName")) {
                std::string camera = cameraHandle.get("objectName").asString();
                renderPass.setCamera(camera);
            }
        } else if (property.contains("layer")) {
            raco::core::ValueHandle layerHandle = tempHandle.asRef();
            if (layerHandle) {
                std::string layer = layerHandle[0].asString();
                renderPass.addRenderLayer(layer);
            }
        } else if (property.compare("enabled") == 0) {
            bool enabled = tempHandle.asBool();
            renderPass.setEnabled(enabled);
        } else if (property.compare("renderOrder") == 0) {
            int order = tempHandle.asInt();
            renderPass.setRenderOrder(order);
        } else if (property.compare("clearColor") == 0) {
            raco::core::Vec4f clearColo = tempHandle.asVec4f();
            renderPass.setClearColor(clearColo.x.asDouble(), clearColo.y.asDouble(), clearColo.z.asDouble(), clearColo.w.asDouble());
        } else if (property.compare("enableClearColor") == 0) {
            bool enable = tempHandle.asBool();
            renderPass.setEnableClearColor(enable);
        } else if (property.compare("enableClearDepth") == 0) {
            bool enable = tempHandle.asBool();
            renderPass.setEnableClearDepth(enable);
        } else if (property.compare("enableClearStencil") == 0) {
            bool enable = tempHandle.asBool();
            renderPass.setEnableClearStencil(enable);
        }
    }
    RenderDataManager::GetInstance().addRenderPass(id, renderPass);
}

void NodeLogic::analyzeRenderTarget(const std::string &id, const core::ValueHandle &handle) {
    RenderTarget renderTarget;
    for (int i = 0; i < handle.size(); i++) {
        core::ValueHandle tempHandle = handle[i];
        QString property = QString::fromStdString(tempHandle.getPropName());
        if (property.compare("objectName") == 0) {
            std::string name = tempHandle.asString();
            renderTarget.setObjectName(name);
        } else if (property.contains("buffer")) {
            raco::core::ValueHandle bufferHandle = tempHandle.asRef();
            if (bufferHandle) {
                std::string buffer = bufferHandle[0].asString();
                renderTarget.addRenderBuffer(buffer);
            }
        }
    }
    RenderDataManager::GetInstance().addRenderTarget(id, renderTarget);
}

void NodeLogic::analyzeRenderLayer(const std::string &id, const core::ValueHandle &handle) {
    RenderLayer renderLayer;
    for (int i = 0; i < handle.size(); i++) {
        core::ValueHandle tempHandle = handle[i];
        QString property = QString::fromStdString(tempHandle.getPropName());
        qDebug() << property;
        if (property.compare("objectName") == 0) {
            std::string name = tempHandle.asString();
            renderLayer.setObjectName(name);
        } else if (property.compare("materialFilterMode") == 0) {
            int mode = tempHandle.asInt();
            renderLayer.setMaterialFilterMode(mode);
        } else if (property.compare("sortOrder") == 0) {
            int order = tempHandle.asInt();
            renderLayer.setRenderOrder(order);
        } else if (property.compare("renderableTags") == 0) {
            for (int j = 0; j < tempHandle.size(); j++) {
                raco::core::ValueHandle tagHandle = tempHandle[j];
                if (tagHandle) {
                    std::string tag = tagHandle.getPropName();
                    renderLayer.addRenderTag(tag);
                }
            }
        } else if (property.compare("materialFilterTags") == 0) {
            for (int j = 0; j < tempHandle.size(); j++) {
                raco::core::ValueHandle tagHandle = tempHandle[j];
                if (tagHandle) {
                    std::string tag = tagHandle.getPropName();
                    renderLayer.addMaterialFilterTag(tag);
                }
            }
        }
    }
    RenderDataManager::GetInstance().addRenderLayer(id, renderLayer);
}

void NodeLogic::analyzeRenderBuffer(const std::string &id, const core::ValueHandle &handle) {
    RenderBuffer renderBuffer;
    for (int i = 0; i < handle.size(); i++) {
        core::ValueHandle tempHandle = handle[i];
        QString property = QString::fromStdString(tempHandle.getPropName());
        if (property.compare("objectName") == 0) {
            std::string name = tempHandle.asString();
            renderBuffer.setObjectName(name);
        } else if (property.compare("wrapUMode") == 0) {
            int mode = tempHandle.asInt();
            renderBuffer.setUWrapMode((WrapMode)mode);
        } else if (property.compare("wrapVMode") == 0) {
            int mode = tempHandle.asInt();
            renderBuffer.setVWrapMode((WrapMode)mode);
        } else if (property.contains("minSamplingMethod")) {
            int method = tempHandle.asInt();
            renderBuffer.setMinSamplingMethod((Filter)method);
        } else if (property.compare("magSamplingMethod") == 0) {
            int method = tempHandle.asInt();
            renderBuffer.setMagSamplingMethod((Filter)method);
        } else if (property.compare("anisotropy") == 0) {
            int level = tempHandle.asInt();
            renderBuffer.setAnisotropyLevel(level);
        } else if (property.contains("width")) {
            int width = tempHandle.asInt();
            renderBuffer.setWidth(width);
        } else if (property.compare("height") == 0) {
            int height = tempHandle.asInt();
            renderBuffer.setHeight(height);
        } else if (property.compare("format") == 0) {
            int format = tempHandle.asInt();
            renderBuffer.setFormat((FORMAT)format);
        }
    }
    RenderDataManager::GetInstance().addRenderBuffer(id, renderBuffer);
}

void NodeLogic::setNodeNameHandleReMap(std::map<std::string, core::ValueHandle> nodeNameHandleReMap) {
    QMutexLocker locker(&handleMapMutex_);
	nodeObjectIDHandleReMap_.clear();
    nodeObjectIDHandleReMap_ = std::move(nodeNameHandleReMap);
}

bool NodeLogic::getHandleFromObjectID(const std::string &objectID, core::ValueHandle &handle) {
    QMutexLocker locker(&handleMapMutex_);
    auto it = nodeObjectIDHandleReMap_.find(objectID);
    if (it == nodeObjectIDHandleReMap_.end()) {
        return false;
    }
    handle = it->second;
    return true;
}

bool NodeLogic::hasHandleFromObjectID(const std::string &objectID) {
    QMutexLocker locker(&handleMapMutex_);
    auto it = nodeObjectIDHandleReMap_.find(objectID);
    if (it == nodeObjectIDHandleReMap_.end()) {
        return false;
    }
    return true;
}

void NodeLogic::analyzeHandle() {
	raco::guiData::NodeDataManager &nodeManager = NodeDataManager::GetInstance();

    analyzing(&(nodeManager.root()));
}

void NodeLogic::initBasicProperty(raco::core::ValueHandle valueHandle, NodeData *node) {
	QString strObjId, strPropName;

	for (int i{0}; i < valueHandle.size(); i++) {
		if (!valueHandle[i].isObject()) {
			raco::core::ValueHandle tempHandle = valueHandle[i];
			QString str = QString::fromStdString(tempHandle.getPropName());
			if (QString::fromStdString(tempHandle.getPropName()).compare("objectID") == 0) {
				strObjId = QString::fromStdString(tempHandle.asString());
				strPropName = QString::fromStdString(tempHandle.getPropertyPath());
			}

			if (QString::fromStdString(tempHandle.getPropName()).compare("translation") == 0) {
				Vec3 trans;
				trans.x = tempHandle.get("x").asDouble();
				trans.y = tempHandle.get("y").asDouble();
				trans.z = tempHandle.get("z").asDouble();
				node->insertSystemData("translation", trans);
			} else if (QString::fromStdString(tempHandle.getPropName()).compare("rotation") == 0) {
				Vec3 rota;
				rota.x = tempHandle.get("x").asDouble();
				rota.y = tempHandle.get("y").asDouble();
				rota.z = tempHandle.get("z").asDouble();
				node->insertSystemData("rotation", rota);

            } else if (QString::fromStdString(tempHandle.getPropName()).compare("scaling") == 0) {
				Vec3 scal;
				scal.x = tempHandle.get("x").asDouble();
				scal.y = tempHandle.get("y").asDouble();
				scal.z = tempHandle.get("z").asDouble();
                node->insertSystemData("scaling", scal);
            } else if (QString::fromStdString(tempHandle.getPropName()).compare("tags") == 0) {
                for (int j = 0; j < tempHandle.size(); j++) {
                    raco::core::ValueHandle tagHandle = tempHandle[j];
                    std::string tag = tagHandle.asString();
                    node->addTag(tag);
                }
            }
            initBasicProperty(valueHandle[i], node);
		}
    }
}

void NodeLogic::slotUpdateActiveAnimation(QString animation) {
    curAnimation_ = animation;
}

void NodeLogic::preOrderReverse(NodeData *pNode, const int &keyFrame, const std::string &sampleProperty) {
    if (!pNode)
        return;

    if (pNode->getBindingySize() != 0) {
        std::map<std::string, std::string> bindingDataMap;
        pNode->NodeExtendRef().curveBindingRef().getPropCurve(sampleProperty, bindingDataMap);

        // TODO SETPROPERTY
        setPropertyByCurveBinding(pNode->objectID(), bindingDataMap, keyFrame);
        raco::signal::signalProxy::GetInstance().sigUpdateMeshModelMatrix(pNode->objectID());
    }
    for (auto it = pNode->childMapRef().begin(); it != pNode->childMapRef().end(); ++it) {
        preOrderReverse(&(it->second), keyFrame, sampleProperty);
    }
}

void NodeLogic::setPropertyByCurveBinding(const std::string &objecID, const std::map<std::string, std::string> &map, const int &keyFrame) {
    QMutexLocker locker(&handleMapMutex_);
    auto iter = nodeObjectIDHandleReMap_.find(objecID);
    if (iter != nodeObjectIDHandleReMap_.end()) {
        for (const auto &bindingIt : map) {
            if (CurveManager::GetInstance().getCurve(bindingIt.second)) {
                double value{0};
                EInterPolationType type;
                if (CurveManager::GetInstance().getPointType(bindingIt.second, keyFrame, type)) {
                    if (getKeyValue(bindingIt.second, type, keyFrame, value)) {
                        setProperty(iter->second, bindingIt.first, value);
                    }
                }
            }
        }
    }
}

void NodeLogic::delNodeBindingByCurveName(std::string curveName) {
	NodeDataManager::GetInstance().delCurveBindingByName(curveName);
	Q_EMIT sig_initCurveBindingWidget__NodePro();
}

bool NodeLogic::getKeyValue(std::string curve, EInterPolationType type, int keyFrame, double &value) {
    int curX = VisualCurvePosManager::GetInstance().getCurX();
    int curY = VisualCurvePosManager::GetInstance().getCurY();
    double eachFrameWidth = VisualCurvePosManager::GetInstance().getEachFrameWidth();
    double eachValueWidth = VisualCurvePosManager::GetInstance().getEachValueWidth();

    switch (type) {
    case EInterPolationType::LINER: {
        CurveManager::GetInstance().getCurveValue(curve, keyFrame, type, value);
        return true;
    }
    case EInterPolationType::BESIER_SPLINE: {
        if (CurveManager::GetInstance().hasCurve(curve)) {
            Curve *curveData = CurveManager::GetInstance().getCurve(curve);
            std::list<Point *> pointList = curveData->getPointList();
            for (auto it = pointList.begin(); it != pointList.end(); it++) {
                auto itTemp = it;
                itTemp++;
                if (itTemp != pointList.end()) {
                    Point *point1 = *it;
                    Point *point2 = *itTemp;
                    if (point1->getKeyFrame() < keyFrame && point2->getKeyFrame() > keyFrame) {
                        QList<QPointF> srcPoints, destPoints;
                        srcPoints.push_back(QPointF(point1->getKeyFrame() * eachFrameWidth, std::any_cast<double>(point1->getDataValue()) * eachValueWidth));
                        srcPoints.push_back(QPointF(point1->getRightKeyFrame() * eachFrameWidth, std::any_cast<double>(point1->getRightData()) * eachValueWidth));
                        srcPoints.push_back(QPointF(point2->getLeftKeyFrame() * eachFrameWidth, std::any_cast<double>(point2->getLeftData()) * eachValueWidth));
                        srcPoints.push_back(QPointF(point2->getKeyFrame() * eachFrameWidth, std::any_cast<double>(point2->getDataValue()) * eachValueWidth));

                        time_axis::createNBezierCurve(srcPoints, destPoints, 0.001);

                        // point border value
                        double dIndex = (1000.0 / (point2->getKeyFrame() - point1->getKeyFrame()) * (keyFrame - point1->getKeyFrame())) - 1;
                        int iIndex = dIndex;
                        if (iIndex >= destPoints.size() - 1) {
                            iIndex = destPoints.size() - 1;
                            time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex], value);
                            return true;
                        }
                        if (iIndex < 0) {
                            iIndex = 0;
                        }

                        // point value
                        double offset = dIndex - iIndex;
                        double lastValue, nextValue;
                        time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex], lastValue);
                        time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex + 1], nextValue);
                        value = (nextValue - lastValue) * offset + lastValue;
                        return true;
                    }
                }
            }
            return CurveManager::GetInstance().getCurveValue(curve, keyFrame, EInterPolationType::LINER, value);
        }
        break;
    }
    case EInterPolationType::HERMIT_SPLINE: {
        if (CurveManager::GetInstance().hasCurve(curve)) {
            Curve *curveData = CurveManager::GetInstance().getCurve(curve);
            std::list<Point *> pointList = curveData->getPointList();
            for (auto it = pointList.begin(); it != pointList.end(); it++) {
                auto itTemp = it;
                itTemp++;
                if (itTemp != pointList.end()) {
                    Point *point1 = *it;
                    Point *point2 = *itTemp;
                    if (point1->getKeyFrame() < keyFrame && point2->getKeyFrame() > keyFrame) {
                        QList<QPointF> srcPoints, destPoints;

                        srcPoints.push_back(QPointF(point1->getKeyFrame() * eachFrameWidth, std::any_cast<double>(point1->getDataValue()) * eachValueWidth));
                        srcPoints.push_back(QPointF(point2->getKeyFrame() * eachFrameWidth, std::any_cast<double>(point2->getDataValue()) * eachValueWidth));
                        srcPoints.push_back(QPointF((point1->getRightKeyFrame() - point1->getKeyFrame()) * eachFrameWidth,
                                                    (std::any_cast<double>(point1->getRightData()) - std::any_cast<double>(point1->getDataValue())) * eachValueWidth));
                        srcPoints.push_back(QPointF((point2->getKeyFrame() - point2->getLeftKeyFrame()) * eachFrameWidth,
                                                    (std::any_cast<double>(point2->getLeftData()) - std::any_cast<double>(point2->getDataValue())) * eachValueWidth));

                        time_axis::createHermiteCurve(srcPoints, destPoints, 0.001);

                        // point border value
                        double dIndex = (1000.0 / (point2->getKeyFrame() - point1->getKeyFrame()) * (keyFrame - point1->getKeyFrame())) - 1;
                        int iIndex = dIndex;
                        if (iIndex >= destPoints.size() - 1) {
                            iIndex = destPoints.size() - 1;
                            time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex], value);
                            return true;
                        }
                        if (iIndex < 0) {
                            iIndex = 0;
                        }

                        // point value
                        double offset = dIndex - iIndex;
                        double lastValue, nextValue;
                        time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex], lastValue);
                        time_axis::point2Value(eachFrameWidth, eachValueWidth, destPoints[iIndex + 1], nextValue);
                        value = (nextValue - lastValue) * offset + lastValue;
                        return true;
                    }
                }
            }
            return CurveManager::GetInstance().getCurveValue(curve, keyFrame, EInterPolationType::LINER, value);
        }
        break;
    }
    case EInterPolationType::STEP: {
        CurveManager::GetInstance().getCurveValue(curve, keyFrame, type, value);
        return true;
    }
    }

    return false;
}

void NodeLogic::slotUpdateKeyFrame(int keyFrame) {
    preOrderReverse(&NodeDataManager::GetInstance().root(), keyFrame, curAnimation_.toStdString());
}

void NodeLogic::slotResetNodeData() {
    NodeDataManager::GetInstance().clearNodeData();
}

void NodeLogic::slotUpdateNodeProperty(const std::string &objectID, const core::ValueHandle &handle) {
    handleMapMutex_.lock();
    if (objectID != "" && objectID != "objectID") {
        NodeData *pNode = NodeDataManager::GetInstance().searchNodeByID(objectID);
        pNode->clearTags();
        initBasicProperty(handle, pNode);
    }
    handleMapMutex_.unlock();
}

void NodeLogic::slotUpdateMeshNodeTranslation(const std::string &objectID, const double &transX, const double &transY, const double &transZ) {
    raco::core::ValueHandle handle;
    if (getHandleFromObjectID(objectID, handle)) {

        raco::core::ValueHandle tempHandle = handle;
        if (getValueHanlde("translation.x", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + transX;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("translation.y", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + transY;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("translation.z", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + transZ;
            commandInterface_->set(tempHandle, value, false);
        }
    }
}

void NodeLogic::slotUpdateMeshNodeRotation(const std::string &objectID, const double &rotatX, const double &rotatY, const double &rotatZ) {
    raco::core::ValueHandle handle;
    if (getHandleFromObjectID(objectID, handle)) {
        raco::core::ValueHandle tempHandle = handle;
        if (getValueHanlde("rotation.x", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + rotatX;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("rotation.y", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + rotatY;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("rotation.z", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + rotatZ;
            commandInterface_->set(tempHandle, value, false);
        }
    }
}

void NodeLogic::slotUpdateMeshNodeScaling(const std::string &objectID, const double &scaling) {
    raco::core::ValueHandle handle;
    if (getHandleFromObjectID(objectID, handle)) {
        raco::core::ValueHandle tempHandle = handle;
        if (getValueHanlde("scaling.x", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + scaling;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("scaling.y", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + scaling;
            commandInterface_->set(tempHandle, value, false);
        }
        tempHandle = handle;
        if (getValueHanlde("scaling.z", tempHandle) && commandInterface_) {
            double value = tempHandle.asDouble() + scaling;
            commandInterface_->set(tempHandle, value, false);
        }
    }
}

}
