#ifndef OUTPUT_ASSETS_H
#define OUTPUT_ASSETS_H

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QThread>
#include <mutex>

#include "PropertyData/PropertyData.h"
#include "NodeData/nodeManager.h"
#include "PropertyData/PropertyType.h"
#include "AnimationData/animationData.h"
#include "CurveData/CurveManager.h"
#include "MaterialData/materialManager.h"
#include "signal/SignalProxy.h"
#include "MeshData/MeshDataManager.h"
#include "data_Convert/AssetsFunction.h"
#include "data_Convert/ExternalInterface.h"

#include "proto/Numeric.pb.h"
#include "proto/Common.pb.h"
#include "proto/Scenegraph.pb.h"
#include "proto/HmiWidget.pb.h"
#include "proto/HmiBase.pb.h"
#include <google/protobuf/text_format.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <qdir.h>

using namespace raco::guiData;

namespace raco::dataConvert {

struct NodeWithMaterial {
	std::string nodeTextureName;
	MaterialData material;
};

struct CurvesSingleProp{
	std::string curveName;
	double defaultData;
};

struct AnimationsSingleCurve {
	NodeData* pNode;
	std::string property;
	double defaultData;
};

class OutputPtx : public QObject {
    Q_OBJECT
public:
    bool writeProgram2Ptx(std::string filePath, QString relativePath);
	void writeNodePtx(NodeData* pNode, HmiScenegraph::TNode* parent);
	void setMaterialTextureByNodeUniforms(NodeData* childNode, MaterialData& materialData);
    void setPtxNode(NodeData* childNode, HmiScenegraph::TNode& hmiNode);
    void setPtxTCamera(NodeData* childNode, HmiScenegraph::TNode& hmiNode);
	bool isEqualUniform(std::vector<Uniform> publicUniforms, Uniform privateUniform);
	void setPtxTMesh(NodeData* node, HmiScenegraph::TMesh& mesh);
	void setMeshBaseNode(NodeData* node, HmiScenegraph::TNode* baseNode);
	void setRootSRT(HmiScenegraph::TNode* hmiNode);
	std::string delNodeNameSuffix(std::string nodeName);
	TECompareFunction matchCompareFunction(DepthCompare depthCmp);
	TEBlendFactor matchBlendFactor(BlendFactor blendFactor);
	TEBlendOperation matchBlendOperation(BlendOperation blendOpera);
	TEFace matchFaceCulling(Culling cull);
	TEWinding matchWinding(WindingType wind);
	void setMaterialDefaultRenderMode(RenderMode& renderMode, HmiScenegraph::TRenderMode* rRenderMode);
	void setMaterialRenderMode(RenderMode& renderMode, HmiScenegraph::TRenderMode* rRenderMode);
	void uniformTypeValue(Uniform data, HmiScenegraph::TUniform& tUniform);
	TETextureFilter matchFilter(Filter filter);
	TETextureWrapMode matchWrapMode(WrapMode mode);
	bool mkdir(QString path);
	bool isStored(std::string name, std::set<std::string>& nameArr);
	std::string getShaderPtxNameByShaderName(std::string name);
	void messageBoxError(std::string materialName, int type);
	void writeBitmap2MaterialLib(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary);
	void writeShaders2MaterialLib(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary);
	void writeMaterial2MaterialLib(HmiScenegraph::TMaterialLib* materialLibrary);
	void writeMaterialLib2Ptx(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary);
	void isNotAddedAttribute(std::string name);
	void functionIconUniform(Uniform data, HmiScenegraph::TUniform& tUniform);

private:
	bool isPtxOutputError_{false};
    std::vector<NodeWithMaterial> nodeWithMaterial_;
};

class OutputPtw{
public:
	void WriteAsset(std::string filePath);
	void WriteBasicInfo(HmiWidget::TWidget* widget);

private:
	void ModifyTranslation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, NodeData& node);
	void CreateTranslation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, raco::guiData::NodeData& node);

	void ModifyScale(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform);
	void CreateScale(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, raco::guiData::NodeData node);

	void ModifyRotation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform);
	void CreateRotation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, raco::guiData::NodeData node);

	void switchMultAnimsOneCurve(HmiWidget::TWidget* widget);
	void selfTriggerDomain(HmiWidget::TWidget* widget);
	void ConvertBind(HmiWidget::TWidget* widget, raco::guiData::NodeData& node);
	void ConvertCurveInfo(HmiWidget::TWidget* widget, std::string animation_interal);
	void modifyOnePointCurve(Point* point, TCurveDefinition* curveDefinition, std::string curveName);
	void addPoint2Curve(Point* pointData, TCurveDefinition* curveDefinition, std::string curveName);
	bool isAddedUniform(std::string name, raco::guiData::NodeData* node);
	bool isVecUniformValue(std::string name);
	void addOperandCurveRef2Operation(TOperation* operation, std::string curveName, std::string multiCurveName = "");
	void setUniformOperationByType(raco::guiData::Uniform& vecUniform, TOperation* operation, std::string* curveNameArr, std::string multiCurveName = "", bool isMultiAnimationSingleCurve = false);
	void addVecValue2Uniform(HmiWidget::TWidget* widget, std::pair<std::string, std::string> curveProP, HmiWidget::TNodeParam* nodeParam, raco::guiData::NodeData* node);
	void addOperandOne2Operation(TOperation* operation, float data);
	void AddUniform(HmiWidget::TWidget* widget, std::pair<std::string, std::string> curveProP, HmiWidget::TNodeParam* nodeParam, raco::guiData::NodeData* node);
	void messageBoxError(std::string curveName, int errorNum);
	void addAnimationDomain(HmiWidget::TWidget* widget, std::string animationName);
	void triggerByInternalModel(HmiWidget::TWidget* widget);
	void triggerByExSingleAnimation(HmiWidget::TWidget* widget);
	bool hasMultiCurveOneProp(std::string prop, NodeData* node, std::vector<std::map<std::string, CurvesSingleProp>>& curves);
	void modifyMultiCurveTransform(HmiWidget::TWidget* widget, HmiWidget::TNodeTransform* transform, std::string propName, std::vector<std::map<std::string, CurvesSingleProp>> curves);
	bool hasMultiAnimationOneCurve(std::string curveName, NodeData* pNode, AnimationsSingleCurve& aniSingleCurv, std::string& animationName);
	void modifyMultiAnimaTransform(HmiWidget::TWidget* widget, HmiWidget::TNodeTransform* transform, std::string propName, std::string curve, AnimationsSingleCurve c, std::string na);
	void addAnimationCurveSwitch(HmiWidget::TWidget* widget, std::string animationName, std::string curveName, AnimationsSingleCurve aniSingleCurve);

	void externalScaleData(HmiWidget::TWidget* widget);
	void externalScale(HmiWidget::TWidget* widget);

	void externalSysUniformData(HmiWidget::TWidget* widget);
	void externalAnimation(HmiWidget::TWidget* widget);
	void externalOpacity(HmiWidget::TWidget* widget);
	void createResourceParam(HmiWidget::TWidget* widget);

	void externalColorData(HmiWidget::TWidget* widget);
	void externalColorUniform(HmiWidget::TUniform& tUniform, int index);
	void AddUColorUniforms(HmiWidget::TNodeParam* nodeParam, NodeData* node);

	void animationSwitchPreData(HmiWidget::TWidget* widget);
	void animationSwitch(HmiWidget::TWidget* widget);
	void sumAnimationValue(HmiWidget::TWidget* widget);
	void switchAnimations(HmiWidget::TWidget* widget);

	void internalDurationValue(HmiWidget::TWidget* widget);

	void multiCurveBindingSinglePropSwitch(HmiWidget::TWidget* widget, std::string propName, std::vector<std::map<std::string, CurvesSingleProp>> curves);

	void proExVarMapping(HmiWidget::TWidget* widget);
	bool getAnimationInteral(std::string curveName, std::string& animationInteral);
	//void externalDotbackground(HmiWidget::TWidget* widget);
	void externalDotOpacity(HmiWidget::TWidget* widget);

	void externalDotSize(HmiWidget::TWidget* widget);
	void triggerTest();

	void addEx2Ellie(HmiWidget::TWidget* widget);

	void triggerByExMultiAnimation(HmiWidget::TWidget* widget);
	void setUniformOperation(guiData::Uniform& vecUniform, TOperation* operation, std::string* curveNameArr, std::string* multiCurveOrAnimationName);

private:
	std::map<std::string, std::vector<std::string>> nodeIDUniformsName_;
	bool isPtwOutputError_{false};
	bool addTrigger_{false};
	bool isSingleAnimation_{false};
	AssetsFunction assetsFun_;
	ExternalInterface externalInterface_;
};
}

#endif // OUTPUT_ASSETS_H
