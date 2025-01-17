/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ramses-renderer-api/IRendererEventHandler.h>
#include <ramses-renderer-api/IRendererSceneControlEventHandler.h>
#include <ramses-renderer-api/RamsesRenderer.h>
#include <ramses-renderer-api/RendererSceneControl.h>
#include <unordered_map>
#include <unordered_set>

namespace raco::ramses_widgets {

class SceneStateEventHandler final : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty {
public:
	explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer);

	void offscreenBufferLinked(ramses::displayBufferId_t offscreenBufferId, ramses::sceneId_t, ramses::dataConsumerId_t, bool success) override;
	void offscreenBufferCreated(ramses::displayId_t, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override;
	void offscreenBufferDestroyed(ramses::displayId_t, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult) override;
	void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
	void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
	void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override;
	void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion) override;

	void waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
	bool waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion);
	bool waitForDisplayCreation(ramses::displayId_t displayId);
	bool waitForDisplayDestruction(ramses::displayId_t displayId);
	bool waitForOffscreenBufferCreation(ramses::displayBufferId_t displayBufferId);
	bool waitForOffscreenBufferDestruction(ramses::displayBufferId_t displayBufferId);
	bool waitForOffscreenBufferLinked(ramses::displayBufferId_t displayBufferId);
	bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction);

	ramses::RendererSceneState sceneState(ramses::sceneId_t sceneId);

private:
	struct SceneInfo {
		ramses::RendererSceneState state = ramses::RendererSceneState::Unavailable;
		ramses::sceneVersionTag_t version = ramses::InvalidSceneVersionTag;
	};

	typedef std::unordered_map<ramses::sceneId_t, SceneInfo> SceneSet;

	template <typename T>
	void waitForElementInSet(const T element, const std::unordered_set<T>& set);

	std::unordered_set<ramses::displayId_t> displays_;
	std::unordered_set<ramses::displayBufferId_t> offscreenBuffers_;
	std::unordered_set<ramses::displayBufferId_t> linkedOffscreenBuffers_;
	SceneSet scenes_;
	ramses::RamsesRenderer& renderer_;
};

}  // namespace raco::ramses_widgets
