#ifndef NODEDATAPRO_H
#define NODEDATAPRO_H

#include <QObject>
#include <set>

#include "core/CommandInterface.h"
#include "property_browser/PropertyBrowserItem.h"
#include "NodeData/NodeManager.h"
#include "CurveData/CurveManager.h"
#include "signal/SignalProxy.h"
#include "user_types/RenderPass.h"
#include <QDebug>
#include <QMutex>

using namespace raco::signal;
using namespace raco::guiData;

namespace raco::node_logic {
class PropertyBrowserItem;

class NodeLogic : public QObject {
	Q_OBJECT
public:
    explicit NodeLogic(raco::core::CommandInterface* commandInterface,
                       QObject *parent = nullptr);

    void setCommandInterface(raco::core::CommandInterface* commandInterface);
    void analyzeHandle();

	void initBasicProperty(raco::core::ValueHandle valueHandle, NodeData *pNode);
    void analyzing(NodeData *pNode);

    bool getValueHanlde(std::string property, core::ValueHandle &valueHandle);

    void setProperty(core::ValueHandle handle, std::string property, float value);
    std::map<std::string, core::ValueHandle > &getNodeNameHandleReMap();

    void analyzeRenderHandles(const std::map<std::string, core::ValueHandle> &handles);
    void analyzeRenderPass(const std::string &id, const core::ValueHandle &handle);
    void analyzeRenderTarget(const std::string &id, const core::ValueHandle &handle);
    void analyzeRenderLayer(const std::string &id, const core::ValueHandle &handle);
    void analyzeRenderBuffer(const std::string &id, const core::ValueHandle &handle);

    void setNodeNameHandleReMap(std::map<std::string, core::ValueHandle> nodeNameHandleReMap);
    bool getHandleFromObjectID(const std::string &objectID, raco::core::ValueHandle &handle);
    bool hasHandleFromObjectID(const std::string &objectID);

    void preOrderReverse(NodeData *pNode, const int &keyFrame, const std::string &sampleProperty);
    void setPropertyByCurveBinding(const std::string &objecID, const std::map<std::string, std::string> &map, const int &keyFrame);
    bool getKeyValue(std::string curve, EInterPolationType type, int keyFrame, double& value);
    void delNodeBindingByCurveName(std::string curveName);

public Q_SLOTS:
    void slotUpdateActiveAnimation(QString animation);
    void slotUpdateKeyFrame(int keyFrame);
    void slotResetNodeData();
    void slotUpdateNodeProperty(const std::string &objectID, const raco::core::ValueHandle &handle);
    void slotUpdateMeshNodeTranslation(const std::string &objectID, const double &transX, const double &transY, const double &transZ);
    void slotUpdateMeshNodeRotation(const std::string &objectID, const double &rotatX, const double &rotatY, const double &rotatZ);
    void slotUpdateMeshNodeScaling(const std::string &objectID, const double &scaling);

signals:
	void sig_getHandles_from_NodePro(std::set<core::ValueHandle>& handles);
	void sig_initCurveBindingWidget__NodePro();

private:
    QMutex handleMapMutex_;
    QMutex tagMutex_;
	std::map<std::string, core::ValueHandle> nodeObjectIDHandleReMap_;
	raco::core::CommandInterface *commandInterface_;
    QString curAnimation_;
};
}

#endif // NODEDATAPRO_H
