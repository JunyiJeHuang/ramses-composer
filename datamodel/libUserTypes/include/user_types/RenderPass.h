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

#include "core/PrefabOperations.h"

#include "user_types/BaseCamera.h"
#include "user_types/BaseObject.h"
#include "user_types/OrthographicCamera.h"
#include "user_types/PerspectiveCamera.h"
#include "user_types/RenderLayer.h"
#include "user_types/RenderTarget.h"

namespace raco::user_types {

class RenderPass : public BaseObject {
public:
	static inline const TypeDescriptor typeDescription = {"RenderPass", true};
	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	RenderPass(RenderPass const& other)
		: BaseObject(other),
		  target_(other.target_),
		  camera_(other.camera_),
		  layer0_(other.layer0_),
		  layer1_(other.layer1_),
		  layer2_(other.layer2_),
		  layer3_(other.layer3_),
		  layer4_(other.layer4_),
		  layer5_(other.layer5_),
		  layer6_(other.layer6_),
		  layer7_(other.layer7_),
		  enabled_(other.enabled_),
		  renderOnce_(other.renderOnce_),
		  renderOrder_(other.renderOrder_),
		  clearColor_(other.clearColor_),
		  enableClearColor_(other.enableClearColor_),
		  enableClearDepth_(other.enableClearDepth_),
		  enableClearStencil_(other.enableClearStencil_) {
		fillPropertyDescription();
	}

	RenderPass(const std::string& name, const std::string& id) : BaseObject(name, id) {
		fillPropertyDescription();
	}

	void fillPropertyDescription() {
		properties_.emplace_back("target", &target_);
		properties_.emplace_back("camera", &camera_);
		properties_.emplace_back("layer0", &layer0_);
		properties_.emplace_back("layer1", &layer1_);
		properties_.emplace_back("layer2", &layer2_);
		properties_.emplace_back("layer3", &layer3_);
		properties_.emplace_back("layer4", &layer4_);
		properties_.emplace_back("layer5", &layer5_);
		properties_.emplace_back("layer6", &layer6_);
		properties_.emplace_back("layer7", &layer7_);

		properties_.emplace_back("enabled", &enabled_);
		properties_.emplace_back("renderOnce", &renderOnce_);
		properties_.emplace_back("renderOrder", &renderOrder_);
		properties_.emplace_back("clearColor", &clearColor_);
		properties_.emplace_back("enableClearColor", &enableClearColor_);
		properties_.emplace_back("enableClearDepth", &enableClearDepth_);
		properties_.emplace_back("enableClearStencil", &enableClearStencil_);
	}

	bool isClearTargetProperty(ValueHandle const& handle) const;
	
	Property<SRenderTarget, DisplayNameAnnotation, ExpectEmptyReference> target_{{}, {"Target"}, {"Default Framebuffer"}};
	Property<SBaseCamera, DisplayNameAnnotation> camera_{{}, {"Camera"}};

	Property<SRenderLayer, DisplayNameAnnotation> layer0_{{}, {"Layer 0"}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer1_{{}, {"Layer 1"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer2_{{}, {"Layer 2"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer3_{{}, {"Layer 3"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer4_{{}, {"Layer 4"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer5_{{}, {"Layer 5"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer6_{{}, {"Layer 6"}, {}};
	Property<SRenderLayer, DisplayNameAnnotation, ExpectEmptyReference> layer7_{{}, {"Layer 7"}, {}};

	Property<bool, DisplayNameAnnotation, LinkEndAnnotation> enabled_{true, {"Enabled"}, {2}};
	Property<bool, DisplayNameAnnotation, LinkEndAnnotation, FeatureLevel> renderOnce_{false, {"Render Once"}, {2}, {2}};
	Property<int, DisplayNameAnnotation, LinkEndAnnotation> renderOrder_{1, {"Render Order"}, {2}};

	Property<Vec4f, DisplayNameAnnotation, LinkEndAnnotation> clearColor_{{}, {"Clear Color"}, {2}};

	Property<bool, DisplayNameAnnotation> enableClearColor_{true, {"Enable Clear Color"}};
	Property<bool, DisplayNameAnnotation> enableClearDepth_{true, {"Enable Clear Depth"}};
	Property<bool, DisplayNameAnnotation> enableClearStencil_{true, {"Enable Clear Stencil"}};
};

using SRenderPass = std::shared_ptr<RenderPass>;

}  // namespace raco::user_types
