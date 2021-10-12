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

#include "core/EngineInterface.h"
#include "user_types/DefaultValues.h"
#include "user_types/BaseTexture.h"

namespace raco::user_types {

class RenderBuffer : public TextureSampler2DBase {
public:
	static inline const TypeDescriptor typeDescription = {"RenderBuffer", true};
	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	RenderBuffer(RenderBuffer const& other)
		: TextureSampler2DBase(other), width_(other.width_), height_(other.height_), format_(other.format_) {
		fillPropertyDescription();
	}

	RenderBuffer(const std::string& name, const std::string& id) : TextureSampler2DBase(name, id) {
		fillPropertyDescription();
	}

	void fillPropertyDescription() {
		properties_.emplace_back("width", &width_);
		properties_.emplace_back("height", &height_);
		properties_.emplace_back("format", &format_);
	}

	Property<int, RangeAnnotation<int>, DisplayNameAnnotation> width_{256, {1, 7680}, {"Width"}};
	Property<int, RangeAnnotation<int>, DisplayNameAnnotation> height_{256, {1, 7680}, {"Height"}};

	Property<int, DisplayNameAnnotation, EnumerationAnnotation> format_{DEFAULT_VALUE_RENDER_BUFFER_FORMAT, DisplayNameAnnotation("Format"), EnumerationAnnotation{EngineEnumeration::RenderBufferFormat}};

private:
};

using SRenderBuffer = std::shared_ptr<RenderBuffer>;

}  // namespace raco::user_types