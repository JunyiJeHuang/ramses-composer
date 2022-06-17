/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "PreviewFramebufferScene.h"
#include "PreviewBackgroundScene.h"
#include "RendererBackend.h"
#include "ramses_adaptor/SceneBackend.h"
#include <QSize>
#include <QLabel>
#include <QColor>
#include <memory>
#include <ramses-framework-api/RamsesFrameworkTypes.h>

namespace raco::ramses_widgets {

class RamsesPreviewWindow final {
public:
	struct State {
		ramses::sceneId_t sceneId{ramses::sceneId_t::Invalid()};
		/**
		 * The pixel sizes here are given in device pixel sizes, NOT virtualized Qt pixel sizes.
		 * ----------------------------------------------------
		 * |\                                                 |
		 * | \ (x, y) viewportOffset                          |
		 * |  -----------------------                         |
		 * |  | displayed in        |                         |
		 * |  | RamsesPreviewWindow |                         |
		 * |  ----------------------- (w, h) viewportSize     |
		 * |                                                  |
		 * ---------------------------------------------------- (w, h) virtualSize (targetSize * scale)
		 */
		QPoint viewportOffset{0, 0};
		QSize viewportSize{0, 0};
		QSize targetSize{0, 0};
		QSize virtualSize{0, 0};
		QPoint maskViewportPosition{0, 0};
		QSize maskViewportSize{0, 0};
		QColor backgroundColor{};
		PreviewFilteringMode filteringMode{PreviewFilteringMode::NearestNeighbor};

		bool operator!=(const State & other) const {
			return this->backgroundColor != other.backgroundColor
				|| this->filteringMode != other.filteringMode
				|| this->sceneId != other.sceneId
				|| this->targetSize != other.targetSize
				|| this->viewportOffset != other.viewportOffset
				|| this->viewportSize != other.viewportSize
				|| this->virtualSize != other.virtualSize;
		}
	};

	explicit RamsesPreviewWindow(
		void* windowHandle,
		RendererBackend& rendererBackend,
		raco::ramses_adaptor::SceneBackend* sceneBackend);
	~RamsesPreviewWindow();

	const State& currentState();
	State& nextState();
	void commit();
	void setEnableDisplayGrid(bool enable);
    void sceneUpdate(bool z_up, float scaleValue);

private:
	void* windowHandle_;
	RendererBackend& rendererBackend_;

	ramses::displayId_t displayId_;
	ramses::displayBufferId_t offscreenBufferId_;
	std::unique_ptr<raco::ramses_widgets::PreviewFramebufferScene> framebufferScene_;
	std::unique_ptr<raco::ramses_widgets::PreviewBackgroundScene> backgroundScene_;

	State current_{};
	State next_{};
    QLabel *axesIcon_{nullptr};
    bool updateAxesIconLabel_{false};
};

}  // namespace raco::ramses_widgets
