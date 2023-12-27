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

#include "core/BasicTypes.h"
#include "core/EditorObject.h"

namespace raco::core {

class DefaultResourceDirectories : public StructBase {
public:
	static inline const TypeDescriptor typeDescription = {"DefaultResourceDirectories", false};

	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	bool serializationRequired() const override {
		return true;
	}

	DefaultResourceDirectories(const DefaultResourceDirectories& other, std::function<SEditorObject(SEditorObject)>* translateRef = nullptr)
		: StructBase(),
		  imageSubdirectory_(other.imageSubdirectory_),
		  meshSubdirectory_(other.meshSubdirectory_),
		  scriptSubdirectory_(other.scriptSubdirectory_),
		  interfaceSubdirectory_(other.interfaceSubdirectory_),
		  shaderSubdirectory_(other.shaderSubdirectory_) {
		fillPropertyDescription();
	}

	DefaultResourceDirectories() : StructBase() {
		fillPropertyDescription();
	}

	void fillPropertyDescription() {
		properties_.emplace_back("imageSubdirectory", &imageSubdirectory_);
		properties_.emplace_back("meshSubdirectory", &meshSubdirectory_);
		properties_.emplace_back("scriptSubdirectory", &scriptSubdirectory_);
		properties_.emplace_back("interfaceSubdirectory", &interfaceSubdirectory_);
		properties_.emplace_back("shaderSubdirectory", &shaderSubdirectory_);
	}

	DefaultResourceDirectories& operator=(const DefaultResourceDirectories& other) {
		imageSubdirectory_ = other.imageSubdirectory_;
		meshSubdirectory_ = other.meshSubdirectory_;
		scriptSubdirectory_ = other.scriptSubdirectory_;
		interfaceSubdirectory_ = other.interfaceSubdirectory_;
		shaderSubdirectory_ = other.shaderSubdirectory_;
		return *this;
	}

	void copyAnnotationData(const DefaultResourceDirectories& other) {
		imageSubdirectory_.copyAnnotationData(other.imageSubdirectory_);
		meshSubdirectory_.copyAnnotationData(other.meshSubdirectory_);
		scriptSubdirectory_.copyAnnotationData(other.scriptSubdirectory_);
		interfaceSubdirectory_.copyAnnotationData(other.interfaceSubdirectory_);
		shaderSubdirectory_.copyAnnotationData(other.shaderSubdirectory_);
	}

public:
	Property<std::string, DisplayNameAnnotation, URIAnnotation> imageSubdirectory_{{"images"}, {"Images"}, {URIAnnotation::projectSubdirectoryFilter, core::PathManager::FolderTypeKeys::Project}};
	Property<std::string, DisplayNameAnnotation, URIAnnotation> meshSubdirectory_{{"meshes"}, {"Meshes"}, {URIAnnotation::projectSubdirectoryFilter, core::PathManager::FolderTypeKeys::Project}};
	Property<std::string, DisplayNameAnnotation, URIAnnotation> scriptSubdirectory_{{"scripts"}, {"Scripts"}, {URIAnnotation::projectSubdirectoryFilter, core::PathManager::FolderTypeKeys::Project}};
	Property<std::string, DisplayNameAnnotation, URIAnnotation> interfaceSubdirectory_{{"interfaces"}, {"Interfaces"}, {URIAnnotation::projectSubdirectoryFilter, core::PathManager::FolderTypeKeys::Project}};
	Property<std::string, DisplayNameAnnotation, URIAnnotation> shaderSubdirectory_{{"shaders"}, {"Shaders"}, {URIAnnotation::projectSubdirectoryFilter, core::PathManager::FolderTypeKeys::Project}};
};

class ProjectSettings : public EditorObject {
public:
	static inline const TypeDescriptor typeDescription{"ProjectSettings", false};
	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	ProjectSettings(ProjectSettings const& other) : EditorObject(other),
													sceneId_(other.sceneId_),
													featureLevel_(other.featureLevel_),
													viewport_(other.viewport_),
													backgroundColor_(other.backgroundColor_),
													saveAsZip_(other.saveAsZip_) {
		fillPropertyDescription();
	}

	ProjectSettings(const std::string& name, const std::string& id = std::string()) : EditorObject(name, id) {
		fillPropertyDescription();
		backgroundColor_->w = 1.0;
	}

	ProjectSettings() : ProjectSettings("Main") {}

	void fillPropertyDescription() {
		properties_.emplace_back("sceneId", &sceneId_);
		properties_.emplace_back("featureLevel", &featureLevel_);
		properties_.emplace_back("viewport", &viewport_);
		properties_.emplace_back("backgroundColor", &backgroundColor_);
		properties_.emplace_back("defaultResourceFolders", &defaultResourceDirectories_);
		properties_.emplace_back("saveAsZip", &saveAsZip_);
        //properties_.emplace_back("axes", &axes_);
        //properties_.emplace_back("displayGrid", &displayGrid_);
	}

	Property<int, DisplayNameAnnotation, RangeAnnotation<int>> sceneId_{123u, DisplayNameAnnotation("Scene Id"), {1, 1024}};

	// Values are the same as in rlogic::EFeatureLevel enum
	// See raco::ramses_base::BaseEngineBackend for definitions of min/max feature levels
	Property<int, DisplayNameAnnotation, ReadOnlyAnnotation> featureLevel_{1, {"Feature Level"}, {}};

	Property<Vec2i, DisplayNameAnnotation> viewport_{{{1440, 720}, 0, 4096}, {"Display Size"}};
	Property<Vec4f, DisplayNameAnnotation> backgroundColor_{{}, {"Display Background Color"}};
	Property<bool, DisplayNameAnnotation> saveAsZip_{false, {"Save As Zipped File"}};

	Property<DefaultResourceDirectories, DisplayNameAnnotation> defaultResourceDirectories_{{}, {"Default Resource Folders"}};
	Property<bool, DisplayNameAnnotation> axes_{false, {"+Z up"}};
	Property<bool, DisplayNameAnnotation> displayGrid_{true, {"Display Grid"}};
	// Properties related to timer running hack - remove these properties and all related code when proper animations have been implemented
	Property<bool, HiddenProperty> enableTimerFlag_{false, HiddenProperty()};
	Property<bool, HiddenProperty> runTimer_{false, HiddenProperty()};
};

}  // namespace raco::core
