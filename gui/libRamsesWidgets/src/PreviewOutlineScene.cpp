#include "ramses_widgets/PreviewOutlineScene.h"
#include "ramses_widgets/SceneStateEventHandler.h"
#include "ramses_widgets/outline_shader.h"
namespace raco::ramses_widgets {

using namespace raco::ramses_base;

PreviewOutlineScene::PreviewOutlineScene(raco::ramses_base::RamsesScene& scene, raco::ramses_adaptor::SceneBackend* sceneBackend) : scene_(scene),
																																	sceneBackend_(sceneBackend),
																																	ogCamera_v_{ramsesOrthographicCamera(scene_.get())},
																																	ogCamera_h_{ramsesOrthographicCamera(scene_.get())},
																																	ogCamera_ol_{ramsesOrthographicCamera(scene_.get())},
																																	ppCamera{ramsesPerspectiveCamera(scene_.get())}{
    QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigSwithOutLineModel, this, &PreviewOutlineScene::selectObject);
	QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigUpdateMeshModelMatrixCompleted, this, &PreviewOutlineScene::updateMeshModelMatrix);
    QObject::connect(&raco::signal::signalProxy::GetInstance(), &raco::signal::signalProxy::sigSetVisibleMeshNodeCompleted, this, &PreviewOutlineScene::setVisibleMeshNode);
    InitCamera();
    InitMeshdata();
    InitVRenderPass();
    InitHRenderPass();
    InitOLRenderPass();
    InitSMRenderPass();
    scene_->flush();
    scene_->publish();
}

void PreviewOutlineScene::sceneUpdate(bool z_up, float scaleValue) {
	if (globalCamera_ == nullptr) {
		auto scene = const_cast<ramses::Scene*>(sceneBackend_->currentScene());
		auto id = scene->getSceneId();
		ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
		if (object) {
			if (object->getType() == ramses::ERamsesObjectType_PerspectiveCamera) {
                globalCamera_ = static_cast<ramses::PerspectiveCamera*>(object);

                CameraParam_t globalCameraParam;
                ramses::ERotationConvention type;
                globalCamera_->getTranslation(globalCameraParam.translation[0], globalCameraParam.translation[1], globalCameraParam.translation[2]);
                globalCamera_->getRotation(globalCameraParam.rotation[0], globalCameraParam.rotation[1], globalCameraParam.rotation[2], type);
                globalCameraParam.viewport[0] = globalCamera_->getViewportX();
                globalCameraParam.viewport[1] = globalCamera_->getViewportY();
                globalCameraParam.viewport[2] = globalCamera_->getViewportWidth();
                globalCameraParam.viewport[3] = globalCamera_->getViewportHeight();
                globalCameraParam.frustum[0] = globalCamera_->getVerticalFieldOfView();
                globalCameraParam.frustum[1] = globalCamera_->getAspectRatio();
                globalCameraParam.frustum[2] = globalCamera_->getNearPlane();
                globalCameraParam.frustum[3] = globalCamera_->getFarPlane();

                if (currentCameraParam_ != globalCameraParam) {
                    (*ppCamera)->setTranslation(globalCameraParam.translation[0], globalCameraParam.translation[1], globalCameraParam.translation[2]);
                    (*ppCamera)->setRotation(globalCameraParam.rotation[0], globalCameraParam.rotation[1], globalCameraParam.rotation[2], type);
                    (*ppCamera)->setViewport(globalCamera_->getViewportX(), globalCamera_->getViewportY(), globalCamera_->getViewportWidth(), globalCamera_->getViewportHeight());
                    (*ppCamera)->setFrustum(globalCamera_->getVerticalFieldOfView(), globalCamera_->getAspectRatio(), globalCamera_->getNearPlane(), globalCamera_->getFarPlane());
                    scene_->flush();
                    scene_->publish();
                    currentCameraParam_ = globalCameraParam;
                }
			}
		}
    }
}

//void PreviewOutlineScene::updateScreenFPS(uint32_t width, uint32_t height) {
//    globalCamera_ = nullptr;
//}

void PreviewOutlineScene::updateMeshModelMatrix(const std::string& objectID) {
	if (appearance_sm_ != nullptr) {
		raco::guiData::MeshData meshdata;
        guiData::MeshDataManager::GetInstance().getMeshData(selectedObjectId_, meshdata);
		ramses::UniformInput mMatixInput;
        auto mMatrix = meshdata.getModelMatrix();
		auto uniformState = (*appearance_sm_)->getEffect().findUniformInput("u_MMatrix", mMatixInput);
        (*appearance_sm_)->setInputValueMatrix44f(mMatixInput, mMatrix.data());

		scene_->flush();
		scene_->publish();
    }
}

void PreviewOutlineScene::setVisibleMeshNode(const bool &visible, const std::string &objectID) {
    if (visible) {
        selectObject(QString::fromStdString(objectID));
    } else {
        ramses::AttributeInput vertexInputV;
        auto attributeState = effect_sm_->findAttributeInput("a_Position", vertexInputV);
        (*geometryBinding_sm_)->setInputBuffer(vertexInputV, *emptyDataBuffer_.get());

        scene_->flush();
        scene_->publish();
    }
}

void PreviewOutlineScene::selectObject(const QString& objectId) {
    if (objectId.isEmpty() && geometryBinding_sm_.get()) {
        ramses::AttributeInput vertexInputV;
        auto attributeState = effect_sm_->findAttributeInput("a_Position", vertexInputV);
        (*geometryBinding_sm_)->setInputBuffer(vertexInputV, *emptyDataBuffer_.get());
        scene_->flush();
        scene_->publish();
        return;
    }

	raco::guiData::MeshData meshdata;
	guiData::MeshDataManager::GetInstance().getMeshData(objectId.toStdString(), meshdata);
	auto indexData = meshdata.getIndices();
    selectedObjectId_ = objectId.toStdString();
	geometryBinding_sm_ = ramsesGeometryBinding(scene_.get(), effect_sm_);
	indexDataBuffer_sm_ = ramsesArrayResource(scene_.get(), ramses::EDataType::UInt32, indexData.size(), indexData.data(), "indices");
    if (!indexDataBuffer_sm_.get()) {
        return;
    }
	(*geometryBinding_sm_)->setIndices(*indexDataBuffer_sm_.get());
	auto attributes = meshdata.getAttributes();
	for (auto elem : attributes) {
		if (elem.name == "a_Position") {
			vertexDataBuffer_sm_ = ramsesArrayResource(scene_.get(), ramses::EDataType::Vector3F, elem.data.size() / 3, elem.data.data(), "a_Position");
			ramses::AttributeInput vertexInputV;
			auto attributeState = effect_sm_->findAttributeInput("a_Position", vertexInputV);
			(*geometryBinding_sm_)->setInputBuffer(vertexInputV, *vertexDataBuffer_sm_.get());
		}
    }
    meshNode_sm_->setGeometryBinding(geometryBinding_sm_);
    ramses::UniformInput mMatixInput;
    auto mMatrix = meshdata.getModelMatrix();
    auto uniformState = (*appearance_sm_)->getEffect().findUniformInput("u_MMatrix", mMatixInput);
    (*appearance_sm_)->setInputValueMatrix44f(mMatixInput, mMatrix.data());
	scene_->flush();
    scene_->publish();
}

void PreviewOutlineScene::InitCamera() {
    auto scene = const_cast<ramses::Scene*>(sceneBackend_->currentScene());
    auto id = scene->getSceneId();
    ramses::RamsesObject* object = scene->findObjectByName("PerspectiveCamera");
    if (object) {
        if (object->getType() == ramses::ERamsesObjectType_PerspectiveCamera) {
            globalCamera_ = static_cast<ramses::PerspectiveCamera*>(object);

        }
    }
    width_ = globalCamera_->getViewportWidth();
    height_ = globalCamera_->getViewportHeight();
    QVector3D translation;
    QVector3D rotation;
    globalCamera_->getTranslation(translation[0], translation[1], translation[2]);
    globalCamera_->getRotation(rotation[0], rotation[1], rotation[2]);

    float nearPlane = globalCamera_->getNearPlane();
    float farPlane = globalCamera_->getFarPlane();
    float viewportX = globalCamera_->getViewportX();
    float viewportY = globalCamera_->getViewportY();
    float fieldOfView = globalCamera_->getVerticalFieldOfView();
    float aspectRatio = globalCamera_->getAspectRatio();

    (*ogCamera_v_)->setTranslation(0.0f, 0.0f, 10.f);
    (*ogCamera_v_)->setFrustum(0.0f, width_, 0.0f, height_, nearPlane, farPlane);
    (*ogCamera_v_)->setViewport(viewportX, viewportY, width_, height_);

    (*ogCamera_h_)->setTranslation(0.0f, 0.0f, 10.f);
    (*ogCamera_h_)->setFrustum(0.0f, width_, 0.0f, height_, nearPlane, farPlane);
    (*ogCamera_h_)->setViewport(viewportX, viewportY, width_, height_);

    (*ogCamera_ol_)->setTranslation(0.0f, 0.0f, 10.f);
    (*ogCamera_ol_)->setFrustum(0.0f, width_, 0.0f, height_, nearPlane, farPlane);
    (*ogCamera_ol_)->setViewport(viewportX, viewportY, width_, height_);

    (*ppCamera)->setTranslation(translation[0], translation[1], translation[2]);
    (*ppCamera)->setFrustum(fieldOfView, aspectRatio, nearPlane, farPlane);
    (*ppCamera)->setViewport(viewportX, viewportY, width_, height_);
    (*ppCamera)->setRotation(rotation[0], rotation[1], rotation[2], ramses::ERotationConvention::XYZ);

    currentCameraParam_.translation[0] = translation[0];
    currentCameraParam_.translation[1] = translation[1];
    currentCameraParam_.translation[2] = translation[2];
    currentCameraParam_.rotation[0] = rotation[0];
    currentCameraParam_.rotation[1] = rotation[1];
    currentCameraParam_.rotation[2] = rotation[2];
    currentCameraParam_.viewport[0] = globalCamera_->getViewportX();
    currentCameraParam_.viewport[1] = globalCamera_->getViewportY();
    currentCameraParam_.viewport[2] = globalCamera_->getViewportWidth();
    currentCameraParam_.viewport[3] = globalCamera_->getViewportHeight();
    currentCameraParam_.frustum[0] = globalCamera_->getVerticalFieldOfView();
    currentCameraParam_.frustum[1] = globalCamera_->getAspectRatio();
    currentCameraParam_.frustum[2] = globalCamera_->getNearPlane();
    currentCameraParam_.frustum[3] = globalCamera_->getFarPlane();
    globalCamera_ = nullptr;

//    globalCamera_ = nullptr;
//    (*ogCamera_v_)->setTranslation(0, 0, 10.0f);
//    (*ogCamera_v_)->setFrustum(0.0f, 1440.0f, 0.0f, 720.0f, 0.1f, 1000.0f);
//    (*ogCamera_v_)->setViewport(0, 0, 1440, 720);

//    (*ogCamera_h_)->setTranslation(0, 0, 10.0f);
//    (*ogCamera_h_)->setFrustum(0.0f, 1440.0f, 0.0f, 720.0f, 0.1f, 1000.0f);
//    (*ogCamera_h_)->setViewport(0, 0, 1440, 720);

//    (*ogCamera_ol_)->setTranslation(0, 0, 10.0f);
//    (*ogCamera_ol_)->setFrustum(0.0f, 1440.0f, 0.0f, 720.0f, 0.1f, 1000.0f);
//    (*ogCamera_ol_)->setViewport(0, 0, 1440, 720);

//    (*ppCamera)->setViewport(0, 0, 1440u, 720u);
//    (*ppCamera)->setFrustum(35.f, 1440.f / 720.f, 0.1f, 1000.f);
//    (*ppCamera)->setTranslation(0.0f, 0.0f, 10.0f);
//    (*ppCamera)->setRotation(0.0, 0.0, 0.0, ramses::ERotationConvention::XYZ);

//    currentCameraParam_.translation[0] = 0.0f;
//    currentCameraParam_.translation[1] = 0.0f;
//    currentCameraParam_.translation[2] = 10.0f;
//    currentCameraParam_.rotation[0] = 0.0f;
//    currentCameraParam_.rotation[1] = 0.0f;
//    currentCameraParam_.rotation[2] = 0.0f;
//    currentCameraParam_.viewport[0] = 0;
//    currentCameraParam_.viewport[1] = 0;
//    currentCameraParam_.viewport[2] = 1440;
//    currentCameraParam_.viewport[3] = 720;
//    currentCameraParam_.frustum[0] = 35.f;
//    currentCameraParam_.frustum[1] = 1440.f / 720.f;
//    currentCameraParam_.frustum[2] = 0.1f;
//    currentCameraParam_.frustum[3] = 1000.f;
}

void PreviewOutlineScene::InitMeshdata() {
    std::vector<float> vertex_data = {
        0.0f, 0.0f, 0.f,
        0.0f, height_, 0.f,
        width_, 0.0f, 0.f,
        width_, height_, 0.f};
    std::vector<float> uv_data = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f};
    std::vector<uint32_t> index_data = {
		0, 2, 1,
		1, 2, 3};
    std::vector<float> empty_data = {
        0.0f, 0.0f, 0.f,
        0.0f, 0.0f, 0.f,
        0.0f, 0.0f, 0.f,
        0.0f, 0.0f, 0.f};
	indexDataBuffer_ = ramsesArrayResource(scene_.get(), ramses::EDataType::UInt32, index_data.size(), index_data.data());
	vertexDataBuffer_ = ramsesArrayResource(scene_.get(), ramses::EDataType::Vector3F, vertex_data.size() / 3, vertex_data.data());
	uvDataBuffer_ = ramsesArrayResource(scene_.get(), ramses::EDataType::Vector2F, uv_data.size() / 2, uv_data.data());
    emptyDataBuffer_ = ramsesArrayResource(scene_.get(), ramses::EDataType::Vector3F, empty_data.size() / 3, empty_data.data());
}

void PreviewOutlineScene::InitSMRenderPass() {
	InitRenderPass(renderGroup_sm_, renderPass_sm_, **ppCamera);
	ramses::EffectDescription effectDescriptionSM{};
    effectDescriptionSM.setVertexShader(colorVertexShader);
    effectDescriptionSM.setFragmentShader(colorFragmentShader);
	effectDescriptionSM.setUniformSemantic("u_PMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);
	effectDescriptionSM.setUniformSemantic("u_VMatrix", ramses::EEffectUniformSemantic::ViewMatrix);
	effect_sm_ = ramsesEffect(scene_.get(), effectDescriptionSM);

	ramses::TextureSampler* sampler_sm_ = setTartget(renderPass_sm_);
	ramses::UniformInput textureInput_v;
	(*appearance_v_)->getEffect().findUniformInput("uTex0", textureInput_v);
	(*appearance_v_)->setInputTexture(textureInput_v, *sampler_sm_);

	ramses::UniformInput textureInputMaintex_ol;
	(*appearance_ol_)->getEffect().findUniformInput("u_MainTex", textureInputMaintex_ol);
	(*appearance_ol_)->setInputTexture(textureInputMaintex_ol, *sampler_sm_);

	appearance_sm_ = ramsesAppearance(scene_.get(), effect_sm_);
	meshNode_sm_ = ramsesMeshNode(scene_.get());
	meshNode_sm_->setAppearance(appearance_sm_);
	renderGroup_sm_->addMeshNode(**meshNode_sm_);
}

void PreviewOutlineScene::InitVRenderPass() {
	InitRenderPass(renderGroup_v_, renderPass_v_, **ogCamera_v_);
	InitEffect(effect_v_, geometryBinding_v_, fragmentShaderBlur);
	appearance_v_ = ramsesAppearance(scene_.get(), effect_v_);
	geometryBinding_v_ = ramsesGeometryBinding(scene_.get(), effect_v_);
	(*geometryBinding_v_)->setIndices(*indexDataBuffer_.get());
	ramses::AttributeInput uvInput_v;
	effect_v_->findAttributeInput("aUVSet0", uvInput_v);
	(*geometryBinding_v_)->setInputBuffer(uvInput_v, *uvDataBuffer_.get());
	ramses::AttributeInput vertexInput_v;
	effect_v_->findAttributeInput("aPosition", vertexInput_v);
	(*geometryBinding_v_)->setInputBuffer(vertexInput_v, *vertexDataBuffer_.get());
	meshNode_v_ = ramsesMeshNode(scene_.get());
	meshNode_v_->setAppearance(appearance_v_);
	meshNode_v_->setGeometryBinding(geometryBinding_v_);
	renderGroup_v_->addMeshNode(**meshNode_v_);
	{
		ramses::UniformInput BlurDirectionInput_v;
		(*appearance_v_)->getEffect().findUniformInput("BlurDirection", BlurDirectionInput_v);
		(*appearance_v_)->setInputValueVector2f(BlurDirectionInput_v, 0.0, 1.0);
		ramses::UniformInput BlurRadiusInput_v;
		(*appearance_v_)->getEffect().findUniformInput("BlurRadius", BlurRadiusInput_v);
        (*appearance_v_)->setInputValueFloat(BlurRadiusInput_v, 1.2);
		ramses::UniformInput kzTextureSize0Input_v;
		(*appearance_v_)->getEffect().findUniformInput("kzTextureSize0", kzTextureSize0Input_v);
        (*appearance_v_)->setInputValueVector2f(kzTextureSize0Input_v, width_, height_);
	}
}

void PreviewOutlineScene::InitHRenderPass() {
	InitRenderPass(renderGroup_h_, renderPass_h_, **ogCamera_h_);
	InitEffect(effect_h_, geometryBinding_h_, fragmentShaderBlur);
	appearance_h_ = ramsesAppearance(scene_.get(), effect_h_);
	geometryBinding_h_ = ramsesGeometryBinding(scene_.get(), effect_h_);
	(*geometryBinding_h_)->setIndices(*indexDataBuffer_.get());
	ramses::AttributeInput uvInput_h;
	effect_h_->findAttributeInput("aUVSet0", uvInput_h);
	(*geometryBinding_h_)->setInputBuffer(uvInput_h, *uvDataBuffer_.get());
	ramses::AttributeInput vertexInput_h;
	effect_h_->findAttributeInput("aPosition", vertexInput_h);
	(*geometryBinding_h_)->setInputBuffer(vertexInput_h, *vertexDataBuffer_.get());
	meshNode_h_ = ramsesMeshNode(scene_.get());
	meshNode_h_->setAppearance(appearance_h_);
	renderGroup_h_->addMeshNode(**meshNode_h_);
	meshNode_h_->setGeometryBinding(geometryBinding_h_);

	ramses::TextureSampler* sampler_v_ = setTartget(renderPass_v_);
	ramses::UniformInput textureInputV;
	(*appearance_h_)->getEffect().findUniformInput("uTex0", textureInputV);
	(*appearance_h_)->setInputTexture(textureInputV, *sampler_v_);
	{
		ramses::UniformInput BlurDirectionInput_v;
		(*appearance_h_)->getEffect().findUniformInput("BlurDirection", BlurDirectionInput_v);
		(*appearance_h_)->setInputValueVector2f(BlurDirectionInput_v, 1.0, 0.0);
		ramses::UniformInput BlurRadiusInput_v;
		(*appearance_h_)->getEffect().findUniformInput("BlurRadius", BlurRadiusInput_v);
        (*appearance_h_)->setInputValueFloat(BlurRadiusInput_v, 1.2);

		ramses::UniformInput kzTextureSize0Input_h;
		(*appearance_h_)->getEffect().findUniformInput("kzTextureSize0", kzTextureSize0Input_h);
        (*appearance_h_)->setInputValueVector2f(kzTextureSize0Input_h, width_, height_);
	}
}

void PreviewOutlineScene::InitOLRenderPass() {
	InitRenderPass(renderGroup_ol_, renderPass_ol_, **ogCamera_ol_);
	InitEffect(effect_ol_, geometryBinding_ol_, fragmentShaderOutline);
	appearance_ol_ = ramsesAppearance(scene_.get(), effect_ol_);
	geometryBinding_ol_ = ramsesGeometryBinding(scene_.get(), effect_ol_);
	(*geometryBinding_ol_)->setIndices(*indexDataBuffer_.get());
	ramses::AttributeInput uvInput_ol;
	effect_ol_->findAttributeInput("aUVSet0", uvInput_ol);
	(*geometryBinding_ol_)->setInputBuffer(uvInput_ol, *uvDataBuffer_.get());
	ramses::AttributeInput vertexInput_ol;
	effect_ol_->findAttributeInput("aPosition", vertexInput_ol);
	(*geometryBinding_ol_)->setInputBuffer(vertexInput_ol, *vertexDataBuffer_.get());
	meshNode_ol_ = ramsesMeshNode(scene_.get());
	meshNode_ol_->setAppearance(appearance_ol_);
	meshNode_ol_->setGeometryBinding(geometryBinding_ol_);
	renderGroup_ol_->addMeshNode(**meshNode_ol_);

	ramses::TextureSampler* sampler_h_ = setTartget(renderPass_h_);
	ramses::UniformInput textureInputBlur_v;
	(*appearance_ol_)->getEffect().findUniformInput("u_BlurTex", textureInputBlur_v);
    (*appearance_ol_)->setInputTexture(textureInputBlur_v, *sampler_h_);

	meshNode_ol_->setGeometryBinding(geometryBinding_ol_);

	ramses::RenderTargetDescription rtDesc_ol;
    ramses::RenderBuffer* renderBuffer_ol = scene_->createRenderBuffer(width_, height_, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite);
	rtDesc_ol.addRenderBuffer(*renderBuffer_ol);
	renderTarget_ol_ = scene_->createRenderTarget(rtDesc_ol);
	outlineSampler_ = scene_->createTextureSampler(
		ramses::ETextureAddressMode_Repeat,
		ramses::ETextureAddressMode_Repeat,
		ramses::ETextureSamplingMethod_Nearest,
		ramses::ETextureSamplingMethod_Nearest,
		*renderBuffer_ol);
	renderPass_ol_->setRenderTarget(renderTarget_ol_);
	ramses::UniformInput InputOutLineColor_v;
	(*appearance_ol_)->getEffect().findUniformInput("u_ourlineColor", InputOutLineColor_v);
	(*appearance_ol_)->setInputValueVector4f(InputOutLineColor_v, 0.0, 0.0, 0.0, 1.0);
}
void PreviewOutlineScene::InitEffect(raco::ramses_base::RamsesEffect& effect, raco::ramses_base::RamsesGeometryBinding& geometryBinding, const char* frageShader) {
	ramses::EffectDescription effectDescription{};
	effectDescription.setVertexShader(vertexShader);
	effectDescription.setFragmentShader(frageShader);
	effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
	effect = ramsesEffect(scene_.get(), effectDescription);
}
void PreviewOutlineScene::InitRenderPass(std::shared_ptr<ramses::RenderGroup>& renderGroup, std::shared_ptr<ramses::RenderPass>& renderPass, ramses::Camera& camera) {
	renderGroup = {std::shared_ptr<ramses::RenderGroup>(scene_.get()->createRenderGroup(), raco::ramses_base::createRamsesObjectDeleter<ramses::RenderGroup>(scene_.get()))};
	renderPass = {scene_->createRenderPass(), raco::ramses_base::createRamsesObjectDeleter<ramses::RenderPass>(scene_.get())};
	renderPass->setCamera(camera);
	renderPass->addRenderGroup(*renderGroup.get());
	renderPass->setClearColor(0.0, 0.0, 0.0, 1.0);
	renderPass->setClearFlags(ramses::EClearFlags_Color);
}
ramses::TextureSampler* PreviewOutlineScene::setTartget(std::shared_ptr<ramses::RenderPass>& renderPass) {
	ramses::RenderTargetDescription rtDesc;
    ramses::RenderBuffer* renderBuffer = scene_->createRenderBuffer(width_, height_, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite);
	rtDesc.addRenderBuffer(*renderBuffer);
	ramses::RenderTarget* renderTarget = scene_->createRenderTarget(rtDesc);
	renderPass->setRenderTarget(renderTarget);
	return scene_->createTextureSampler(
		ramses::ETextureAddressMode_Repeat,
		ramses::ETextureAddressMode_Repeat,
		ramses::ETextureSamplingMethod_Nearest,
		ramses::ETextureSamplingMethod_Nearest,
		*renderBuffer);
}
}
