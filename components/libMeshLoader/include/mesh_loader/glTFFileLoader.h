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

#include "core/MeshCacheInterface.h"

namespace tinygltf {
class TinyGLTF;
class Model;
class Node;
}

namespace raco::mesh_loader {

class glTFFileLoader final : public raco::core::MeshCacheEntry {
public:
	glTFFileLoader(std::string absPath);
	~glTFFileLoader() override;

	raco::core::SharedMeshData loadMesh(const raco::core::MeshDescriptor& descriptor) override;
	const raco::core::MeshScenegraph* getScenegraph(const std::string& absPath) override;
    bool writeScenegraphGltf(const core::MeshScenegraph& sceneGraph, const std::string& absPath) override;
	int getTotalMeshCount() override;
	raco::core::SharedAnimationSamplerData getAnimationSamplerData(const std::string& absPath, int animIndex, int samplerIndex) override;
	std::string getError() override;
	void reset() override;
	
	raco::core::SharedSkinData loadSkin(const std::string& absPath, int skinIndex, std::string& outError) override;

private:
	std::string path_;

	std::unique_ptr<tinygltf::Model> scene_;
	std::unique_ptr<tinygltf::TinyGLTF> importer_;
	std::unique_ptr<raco::core::MeshScenegraph> sceneGraph_;
	std::string error_;
	std::string warning_;

	bool buildglTFScenegraph();
	bool importglTFScene(const std::string& absPath);
	void importAnimations();
	void importSkins();

};

} // namespace raco::mesh_loader
