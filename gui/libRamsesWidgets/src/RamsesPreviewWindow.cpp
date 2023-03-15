/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "ramses_widgets/RamsesPreviewWindow.h"

#include "ramses_widgets/SceneStateEventHandler.h"

#include <log_system/log.h>

#include <ramses-renderer-api/DisplayConfig.h>
#include <ramses-renderer-api/RamsesRenderer.h>
#include <ramses_base/RamsesFormatter.h>

using raco::log_system::PREVIEW_WIDGET;

namespace {

using namespace raco::ramses_widgets;

void setAndWaitSceneState(
	RendererBackend& backend,
	const ramses::RendererSceneState state,
	const std::unique_ptr<raco::ramses_widgets::PreviewFramebufferScene>& framebufferScene,
	const std::unique_ptr<raco::ramses_widgets::PreviewBackgroundScene>& backgroundScene,
	const ramses::sceneId_t sceneId) {
	auto& sceneControlAPI = *backend.renderer().getSceneControlAPI();
	if (framebufferScene && framebufferScene->getSceneId().isValid()) {
		sceneControlAPI.setSceneState(framebufferScene->getSceneId(), state);
	}
	if (backgroundScene && backgroundScene->getSceneId().isValid()) {
		sceneControlAPI.setSceneState(backgroundScene->getSceneId(), state);
	}
	if (sceneId.isValid()) {
		sceneControlAPI.setSceneState(sceneId, state);
	}

	sceneControlAPI.flush();

	if (framebufferScene && framebufferScene->getSceneId().isValid()) {
		backend.eventHandler().waitForSceneState(framebufferScene->getSceneId(), state);
	}
	if (backgroundScene && backgroundScene->getSceneId().isValid()) {
		backend.eventHandler().waitForSceneState(backgroundScene->getSceneId(), state);
	}
	if (sceneId.isValid()) {
		backend.eventHandler().waitForSceneState(sceneId, state);
	}
}

/**
 * Sets and await's current scene state for frambuffer and conditionally for scene. The scene state is only set if the current scene state is greater than the request scene state.
 */
void reduceAndWaitSceneState(
	RendererBackend& backend,
	const ramses::RendererSceneState state,
	const std::unique_ptr<raco::ramses_widgets::PreviewFramebufferScene>& framebufferScene,
	const std::unique_ptr<raco::ramses_widgets::PreviewBackgroundScene>& backgroundScene,
	const ramses::sceneId_t sceneId) {
	auto& eventHandler = backend.eventHandler();
	auto& sceneControlAPI = *backend.renderer().getSceneControlAPI();
	if (framebufferScene && framebufferScene->getSceneId().isValid() && eventHandler.sceneState(framebufferScene->getSceneId()) > state) {
		sceneControlAPI.setSceneState(framebufferScene->getSceneId(), state);
	}
	if (backgroundScene && backgroundScene->getSceneId().isValid() && eventHandler.sceneState(backgroundScene->getSceneId()) > state) {
		sceneControlAPI.setSceneState(backgroundScene->getSceneId(), state);
	}
	if (sceneId.isValid() && eventHandler.sceneState(sceneId) > state) {
		sceneControlAPI.setSceneState(sceneId, state);
	}

	sceneControlAPI.flush();

	if (framebufferScene && framebufferScene->getSceneId().isValid()) {
		backend.eventHandler().waitForSceneState(framebufferScene->getSceneId(), state);
	}
	if (backgroundScene && backgroundScene->getSceneId().isValid()) {
		backend.eventHandler().waitForSceneState(backgroundScene->getSceneId(), state);
	}
	if (sceneId.isValid() && eventHandler.sceneState(sceneId) > state) {
		backend.eventHandler().waitForSceneState(sceneId, state);
	}
}

}  // namespace

namespace raco::ramses_widgets {

RamsesPreviewWindow::RamsesPreviewWindow(
	void* windowHandle,
	RendererBackend& rendererBackend,
	raco::ramses_adaptor::SceneBackend* sceneBackend)
	: windowHandle_{windowHandle}, rendererBackend_{rendererBackend}, displayId_{ramses::displayId_t::Invalid()}, offscreenBufferId_{ramses::displayBufferId_t::Invalid()},
	framebufferScene_{std::make_unique<raco::ramses_widgets::PreviewFramebufferScene>(rendererBackend_.client(), rendererBackend.internalSceneId())},
	backgroundScene_{std::make_unique<raco::ramses_widgets::PreviewBackgroundScene>(rendererBackend_.client(), sceneBackend, rendererBackend.internalSceneId())} {
}

RamsesPreviewWindow::~RamsesPreviewWindow() {
	setAndWaitSceneState(rendererBackend_, ramses::RendererSceneState::Available, framebufferScene_, backgroundScene_, current_.sceneId);
	if (offscreenBufferId_.isValid()) {
		rendererBackend_.renderer().destroyOffscreenBuffer(displayId_, offscreenBufferId_);
		rendererBackend_.renderer().flush();
		rendererBackend_.eventHandler().waitForOffscreenBufferDestruction(offscreenBufferId_);
	}
	if (displayId_.isValid()) {
		rendererBackend_.renderer().destroyDisplay(displayId_);
		rendererBackend_.renderer().flush();
		rendererBackend_.eventHandler().waitForDisplayDestruction(displayId_);
	}
}

const RamsesPreviewWindow::State& RamsesPreviewWindow::currentState() {
	return current_;
}

RamsesPreviewWindow::State& RamsesPreviewWindow::nextState() {
	return next_;
}

void RamsesPreviewWindow::commit(bool forceUpdate) {
	if (forceUpdate || !displayId_.isValid() || next_.viewportSize != current_.viewportSize || next_.sceneId != current_.sceneId || next_.targetSize != current_.targetSize || next_.sampleRate != current_.sampleRate || next_.filteringMode != current_.filteringMode) {
		// Unload current scenes
		reduceAndWaitSceneState(rendererBackend_, (displayId_.isValid()) ? ramses::RendererSceneState::Available : ramses::RendererSceneState::Unavailable, framebufferScene_, backgroundScene_, current_.sceneId);

		if (next_.viewportSize.width() > 0 && next_.viewportSize.height() > 0 || next_.sampleRate != current_.sampleRate || next_.filteringMode != current_.filteringMode) {
			auto& sceneControlAPI = *rendererBackend_.renderer().getSceneControlAPI();

			if (!displayId_.isValid()) {
				ramses::DisplayConfig displayConfig = {};
				/// @todo maybe this setWindowRectangle is not needed?
				constexpr auto displayX = 100;
				constexpr auto displayY = 100;
				displayConfig.setWindowRectangle(displayX, displayY, next_.viewportSize.width(), next_.viewportSize.height());
				constexpr auto clearColorR = 0.25;
				constexpr auto clearColorG = 0.25;
				constexpr auto clearColorB = 0.25;
				constexpr auto clearColorA = 1.0;
				displayConfig.setClearColor(clearColorR, clearColorG, clearColorB, clearColorA);
				displayConfig.setIntegrityRGLDeviceUnit(0);
				displayConfig.setWindowIviVisible();
				if constexpr (!BuildOptions::nativeRamsesDisplayWindow) {
#if (defined(__WIN32) || defined(_WIN32))
					displayConfig.setWindowsWindowHandle(windowHandle_);
#else
					displayConfig.setX11WindowHandle((unsigned long)windowHandle_);
#endif
				}
				displayId_ = rendererBackend_.renderer().createDisplay(displayConfig);
				rendererBackend_.renderer().flush();
				rendererBackend_.eventHandler().waitForDisplayCreation(displayId_);
				sceneControlAPI.setSceneMapping(framebufferScene_->getSceneId(), displayId_);
			}
			current_.viewportSize = next_.viewportSize;
			current_.sampleRate = next_.sampleRate;
			current_.filteringMode = next_.filteringMode;

			if (next_.sceneId.isValid()) {
				/// @todo maybe we need to reset old scene mapping?
				sceneControlAPI.setSceneMapping(next_.sceneId, displayId_);
			}
			setAndWaitSceneState(rendererBackend_, ramses::RendererSceneState::Ready, framebufferScene_, next_.sceneId);

			// Set up the render buffer we use as a default framebuffer.
			// This is not a normal Ramses render target / render buffer created with Scene::createRenderBuffer / Scene::createRenderTarget
			// but an offscreen render buffer created with RamsesRenderer::createOffscreenBuffer to avoid creating the
			// offscreen render buffer in the scene we eventually export (and in fact for Ramses this offscreen render buffer is
			// the framebuffer - that we use it later on to blit it into our preview makes for the Ramses scene no difference).
			const ramses::dataConsumerId_t dataConsumerId = framebufferScene_->setupFramebufferTexture(rendererBackend_, next_.targetSize, next_.filteringMode, next_.sampleRate);
			offscreenBufferId_ = rendererBackend_.renderer().createOffscreenBuffer(displayId_, next_.targetSize.width(), next_.targetSize.height(), next_.sampleRate);
			rendererBackend_.renderer().setDisplayBufferClearColor(displayId_, offscreenBufferId_, next_.backgroundColor.redF(), next_.backgroundColor.greenF(),
				next_.backgroundColor.blueF(), next_.backgroundColor.alphaF());
			rendererBackend_.renderer().flush();
			rendererBackend_.eventHandler().waitForOffscreenBufferCreation(offscreenBufferId_);
			current_.targetSize = next_.targetSize;
			current_.backgroundColor = next_.backgroundColor;

			if (next_.sceneId.isValid()) {
				sceneControlAPI.setSceneDisplayBufferAssignment(next_.sceneId, offscreenBufferId_);
			}

			sceneControlAPI.setSceneDisplayBufferAssignment(backgroundScene_->getSceneId(), offscreenBufferId_);
			sceneControlAPI.flush();

			sceneControlAPI.linkOffscreenBuffer(offscreenBufferId_, framebufferScene_->getSceneId(), dataConsumerId);
			sceneControlAPI.flush();
			rendererBackend_.eventHandler().waitForOffscreenBufferLinked(offscreenBufferId_);

			setAndWaitSceneState(rendererBackend_, ramses::RendererSceneState::Rendered, framebufferScene_, backgroundScene_, next_.sceneId);

			current_.sceneId = next_.sceneId;
			LOG_DEBUG(PREVIEW_WIDGET, "commit() sceneId {}", current_.sceneId);
		}
	}

	if (displayId_.isValid() && offscreenBufferId_.isValid() && (forceUpdate || next_.backgroundColor != current_.backgroundColor)) {
		rendererBackend_.renderer().setDisplayBufferClearColor(displayId_, offscreenBufferId_, next_.backgroundColor.redF(), next_.backgroundColor.greenF(), next_.backgroundColor.blueF(), next_.backgroundColor.alphaF());
		rendererBackend_.renderer().flush();
		current_.backgroundColor = next_.backgroundColor;
	}

	current_.viewportOffset = next_.viewportOffset;
	current_.virtualSize = next_.virtualSize;

	if (current_.viewportSize.width() > 0 && current_.viewportSize.height() > 0) {
		framebufferScene_->setViewport(current_.viewportOffset, current_.viewportSize, current_.virtualSize);
	}
}

void RamsesPreviewWindow::setEnableDisplayGrid(bool enable) {
	backgroundScene_->setEnableDisplayGrid(enable);
}

void RamsesPreviewWindow::sceneUpdate(bool z_up, float scaleValue) {
	backgroundScene_->sceneUpdate(z_up, scaleValue);
}

}  // namespace raco::ramses_widgets
