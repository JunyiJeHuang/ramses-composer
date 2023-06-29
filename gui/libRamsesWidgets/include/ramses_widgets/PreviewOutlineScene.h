#ifndef __PREVIEW_OUTLINE_SCENE_H__
#define __PREVIEW_OUTLINE_SCENE_H__

#pragma once

#include "ramses_base/RamsesHandles.h"
#include "ramses_widgets/RendererBackend.h"
#include "ramses_adaptor/SceneBackend.h"
#include "signal/SignalProxy.h"
#include <QtGlobal>
#include <qobject.h>
namespace raco::ramses_widgets {

class PreviewOutlineScene : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(PreviewOutlineScene);
public:
	explicit PreviewOutlineScene(raco::ramses_base::RamsesScene& scene , raco::ramses_adaptor::SceneBackend* sceneBackend);
	void sceneUpdate(bool z_up, float scaleValue);
//    void updateScreenFPS(uint32_t width, uint32_t height);
private:
	void InitCamera();
	void InitMeshdata();
	void InitSMRenderPass();
	void InitVRenderPass();
	void InitHRenderPass();
	void InitOLRenderPass();
	void InitEffect(raco::ramses_base::RamsesEffect& effect, raco::ramses_base::RamsesGeometryBinding& geometryBinding,const char* frageShader);
	void InitRenderPass(std::shared_ptr<ramses::RenderGroup>& renderGroup, std::shared_ptr<ramses::RenderPass>& renderPass, ramses::Camera& camera);
	ramses::TextureSampler* setTartget(std::shared_ptr<ramses::RenderPass>& renderPass);
public Q_SLOTS:
	void updateMeshModelMatrix(const std::string& objectID);
    void setVisibleMeshNode(const bool &visible, const std::string &objectID);
	void selectObject(const QString& objectId);
	ramses::TextureSampler* getOutlineTexture() { return outlineSampler_; }

private:
	struct CameraParam_t {
		float translation[3];
		float rotation[3];
		float scaling[3];
		int32_t viewport[4];
		float frustum[4];

		bool operator!=(const CameraParam_t& other) {
			return (this->translation[0] != other.translation[0] || this->translation[1] != other.translation[1] || this->translation[2] != other.translation[2] || this->rotation[0] != other.rotation[0] || this->rotation[1] != other.rotation[1] || this->rotation[2] != other.rotation[2] || this->scaling[0] != other.scaling[0] || this->scaling[1] != other.scaling[1] || this->scaling[2] != other.scaling[2] || this->viewport[0] != other.viewport[0] || this->viewport[1] != other.viewport[1] || this->viewport[2] != other.viewport[2] || this->viewport[3] != other.viewport[3] || this->frustum[0] != other.frustum[0] || this->frustum[1] != other.frustum[1] || this->frustum[2] != other.frustum[2] || this->frustum[3] != other.frustum[3]);
		};
	};
	raco::ramses_base::RamsesScene& scene_;
	raco::ramses_adaptor::SceneBackend* sceneBackend_;
	raco::ramses_base::RamsesArrayResource indexDataBuffer_;
	raco::ramses_base::RamsesArrayResource vertexDataBuffer_;
    raco::ramses_base::RamsesArrayResource emptyDataBuffer_;
	raco::ramses_base::RamsesArrayResource uvDataBuffer_;
	raco::ramses_base::RamsesPerspectiveCamera ppCamera;
	ramses::TextureSampler* outlineSampler_;
	std::vector<float> vertex_data_;
	//  selected mesh
	CameraParam_t currentCameraParam_;
    std::string selectedObjectId_;

    ramses::PerspectiveCamera* globalCamera_{nullptr};

	std::shared_ptr<ramses::RenderGroup> renderGroup_sm_;
	std::shared_ptr<ramses::RenderPass> renderPass_sm_;
	raco::ramses_base::RamsesEffect effect_sm_;
	raco::ramses_base::RamsesAppearance appearance_sm_;
	raco::ramses_base::RamsesArrayResource indexDataBuffer_sm_;
	raco::ramses_base::RamsesArrayResource vertexDataBuffer_sm_;
	raco::ramses_base::RamsesGeometryBinding geometryBinding_sm_;
	raco::ramses_base::RamsesMeshNode meshNode_sm_;

	// vertical blur
	raco::ramses_base::RamsesOrthographicCamera ogCamera_v_;
	std::shared_ptr<ramses::RenderGroup> renderGroup_v_;
	std::shared_ptr<ramses::RenderPass> renderPass_v_;
	raco::ramses_base::RamsesEffect effect_v_;
	raco::ramses_base::RamsesAppearance appearance_v_;
	raco::ramses_base::RamsesGeometryBinding geometryBinding_v_;
	raco::ramses_base::RamsesMeshNode meshNode_v_;

	// horizontal blur
	raco::ramses_base::RamsesOrthographicCamera ogCamera_h_;
	std::shared_ptr<ramses::RenderGroup> renderGroup_h_;
	std::shared_ptr<ramses::RenderPass> renderPass_h_;
	raco::ramses_base::RamsesEffect effect_h_;
	raco::ramses_base::RamsesAppearance appearance_h_;
	raco::ramses_base::RamsesGeometryBinding geometryBinding_h_;
	raco::ramses_base::RamsesMeshNode meshNode_h_;

	// outline color
	raco::ramses_base::RamsesOrthographicCamera ogCamera_ol_;
	std::shared_ptr<ramses::RenderGroup> renderGroup_ol_;
	std::shared_ptr<ramses::RenderPass> renderPass_ol_;
	raco::ramses_base::RamsesEffect effect_ol_;
	raco::ramses_base::RamsesAppearance appearance_ol_;
	raco::ramses_base::RamsesGeometryBinding geometryBinding_ol_;
	raco::ramses_base::RamsesMeshNode meshNode_ol_;
	ramses::RenderTarget* renderTarget_ol_;
    float width_{1440.f};
    float height_{720.f};
};

}

#endif // __PREVIEW_BACKGROUND_SCENE_H__
