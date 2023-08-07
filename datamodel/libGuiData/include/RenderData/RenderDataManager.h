#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H
#include "MaterialData/MaterialData.h"
#include <string>
#include <any>
#include <vector>
#include <iostream>
#include <QDebug>
#include <list>
#include <map>
#include <vector>

namespace raco::guiData {

class RenderBuffer {
public:
    RenderBuffer() {}

    void setObjectName(std::string objectName) {
        objectName_ = objectName;
    }

    std::string getObjectName() {
        return objectName_;
    }

    void setUWrapMode(WrapMode mode) {
        uMode_ = mode;
    }

    WrapMode getUWrapMode() {
        return uMode_;
    }

    void setVWrapMode(WrapMode mode) {
        vMode_ = mode;
    }

    WrapMode getVWrapMode() {
        return vMode_;
    }

    void setMinSamplingMethod(Filter method) {
        minSamplingMethod_ = method;
    }

    Filter getMinSamplingMethod() {
        return minSamplingMethod_;
    }

    void setMagSamplingMethod(Filter method) {
        magSamplingMethod_ = method;
    }

    Filter getMagSamplingMethod() {
        return magSamplingMethod_;
    }

    void setAnisotropyLevel(int level) {
        anisotropyLevel_ = level;
    }

    int getAnisotropyLevel() {
        return anisotropyLevel_;
    }

    void setWidth(int width) {
        width_ = width;
    }

    int getWidth() {
        return width_;
    }

    void setHeight(int height) {
        height_ = height;
    }

    int getHeight() {
        return height_;
    }

    void setFormat(FORMAT format) {
        format_ = format;
    }

    FORMAT getFormat() {
        return format_;
    }

private:
    std::string objectName_;
    WrapMode uMode_;
    WrapMode vMode_;
    Filter minSamplingMethod_;
    Filter magSamplingMethod_;
    int anisotropyLevel_{1};
    int width_{256};
    int height_{256};
    FORMAT format_{RGBA8};
};

class RenderLayer {
public:
    RenderLayer() {}

    void setObjectName(std::string objectName) {
        objectName_ = objectName;
    }

    std::string getObjectName() {
        return objectName_;
    }

    void addRenderTag(std::string tag) {
        renderTags_.push_back(tag);
    }

    std::vector<std::string> getRenderTags() {
        return renderTags_;
    }

    void clearRenderTags() {
        renderTags_.clear();
    }

    void addMaterialFilterTag(std::string tag) {
        materialFilterTags_.push_back(tag);
    }

    std::vector<std::string> getMaterialFilterTags() {
        return materialFilterTags_;
    }

    void clearMaterialFilterTags() {
        materialFilterTags_.clear();
    }

    void setMaterialFilterMode(int filterMode) {
        materialFilterMode_ = filterMode;
    }

    int getMaterialFilterMode() {
        return materialFilterMode_;
    }

    void setRenderOrder(int renderOrder) {
        renderOrder_ = renderOrder;
    }

    int getRenderOrder() {
        return renderOrder_;
    }
private:
    std::string objectName_;
    std::vector<std::string> renderTags_;
    std::vector<std::string> materialFilterTags_;
    int materialFilterMode_{0};
    int renderOrder_{0};
};

class RenderTarget {
public:
    RenderTarget() {}

    void setObjectName(std::string objectName) {
        objectName_ = objectName;
    }

    std::string getObjectName() {
        return objectName_;
    }

    void addRenderBuffer(std::string renderBuffer) {
        renderBuffers_.push_back(renderBuffer);
    }

    std::vector<std::string> getRenderBuffers() {
        return renderBuffers_;
    }

    void clearRenderBuffers() {
        renderBuffers_.clear();
    }
private:
    std::string objectName_;
    std::vector<std::string> renderBuffers_;
};

class RenderPass {
public:
    RenderPass() {}

    void setObjectName(std::string objectName) {
        objectName_ = objectName;
    }

    std::string getObjectName() {
        return objectName_;
    }

    void setRenderTarget(std::string renderTarget) {
        target_ = renderTarget;
    }

    std::string getRenderTarget() {
        return target_;
    }

    void setCamera(std::string camera) {
        camera_ = camera;
    }

    std::string getCamera() {
        return camera_;
    }

    void addRenderLayer(std::string renderLayer) {
        renderLayers_.push_back(renderLayer);
    }

    std::vector<std::string> getRenderLayers() {
        return renderLayers_;
    }

    void clearRenderLayers() {
        renderLayers_.clear();
    }

    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    bool getEnabled() {
        return enabled_;
    }

    void setRenderOrder(int renderOrder) {
        renderOrder_ = renderOrder;
    }

    int getRenderOrder() {
        return renderOrder_;
    }

    void setClearColor(float x, float y, float z, float w) {
        clearColor_.clear();
        clearColor_.push_back(x);
        clearColor_.push_back(y);
        clearColor_.push_back(z);
        clearColor_.push_back(w);
    }

    std::vector<float> getClearColor() {
        return clearColor_;
    }

    void setEnableClearColor(bool enable) {
        enableClearColor_ = enable;
    }

    bool getEnableClearColor() {
        return enableClearColor_;
    }

    void setEnableClearDepth(bool enable) {
        enableClearDepth_ = enable;
    }

    bool getEnableClearDepth() {
        return enableClearDepth_;
    }

    void setEnableClearStencil(bool enable) {
        enableClearStencil_ = enable;
    }

    bool getEnableClearStencil() {
        return enableClearStencil_;
    }
private:
    std::string objectName_;
    std::string target_;
    std::string camera_;
    std::vector<std::string> renderLayers_;
    bool enabled_{true};
    int renderOrder_{0};
    std::vector<float> clearColor_;
    bool enableClearColor_{true};
    bool enableClearDepth_{true};
    bool enableClearStencil_{true};
};

class RenderDataManager
{
public:
    static RenderDataManager& GetInstance();
    ~RenderDataManager() {}
    RenderDataManager(const RenderDataManager&) = delete;
    RenderDataManager& operator=(const RenderDataManager&) = delete;

    void addRenderPass(std::string name, RenderPass renderPass) {
        renderPasses_.emplace(name, renderPass);
    }

    bool getRenderPass(std::string name, RenderPass &renderPass) {
        auto it = renderPasses_.find(name);
        if (it == renderPasses_.end()) {
            return false;
        }
        renderPass = it->second;
        return true;
    }

    bool getRenderPassByName(std::string name, RenderPass &renderPass) {
        for (const auto &it : renderPasses_) {
            if (it.first.compare(name) == 0) {
                renderPass = it.second;
                return true;
            }
        }
        return false;
    }

    void addRenderBuffer(std::string name, RenderBuffer renderBuffer) {
        renderBuffers_.emplace(name, renderBuffer);
    }

    bool getRenderBuffer(std::string name, RenderBuffer &renderBuffer) {
        auto it = renderBuffers_.find(name);
        if (it == renderBuffers_.end()) {
            return false;
        }
        renderBuffer = it->second;
        return true;
    }

    void addRenderTarget(std::string name, RenderTarget renderTarget) {
        renderTargets_.emplace(name, renderTarget);
    }

    bool getRenderTarget(std::string name, RenderTarget &renderTarget) {
        auto it = renderTargets_.find(name);
        if (it == renderTargets_.end()) {
            return false;
        }
        renderTarget = it->second;
        return true;
    }

    void addRenderLayer(std::string name, RenderLayer renderLayer) {
        renderLayers_.emplace(name, renderLayer);
    }

    bool getRenderLayer(std::string name, RenderLayer &renderLayer) {
        auto it = renderLayers_.find(name);
        if (it == renderLayers_.end()) {
            return false;
        }
        renderLayer = it->second;
        return true;
    }

    void clear() {
        renderPasses_.clear();
        renderBuffers_.clear();
        renderTargets_.clear();
        renderLayers_.clear();
    }
private:
    RenderDataManager();

private:
    std::map<std::string, RenderPass> renderPasses_;
    std::map<std::string, RenderBuffer> renderBuffers_;
    std::map<std::string, RenderTarget> renderTargets_;
    std::map<std::string, RenderLayer> renderLayers_;
};
}

#endif // RENDERDATAMANAGER_H
