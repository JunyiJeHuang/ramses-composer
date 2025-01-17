/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "core/Serialization.h"


#include "testing/TestEnvironmentCore.h"
#include "testing/TestUtil.h"
#include "core/BasicAnnotations.h"
#include "core/ExternalReferenceAnnotation.h"
#include "user_types/LuaScript.h"
#include "user_types/Material.h"
#include "user_types/MeshNode.h"
#include "user_types/Node.h"
#include "user_types/Texture.h"
#include "utils/FileUtils.h"
#include "utils/u8path.h"
#include <gtest/gtest.h>

constexpr bool WRITE_RESULT{false};
#ifndef CMAKE_CURRENT_SOURCE_DIR
#define CMAKE_CURRENT_SOURCE_DIR "."
#endif

using namespace raco::utils;

struct SerializationTest : public TestEnvironmentCore {
	void assertFileContentEqual(const std::string &filePath, const std::string &deserializedFileContent) {
		auto expectedFileContent = file::read(filePath);

#if (defined(__linux__))
        expectedFileContent.erase(std::remove(expectedFileContent.begin(), expectedFileContent.end(), '\r'), expectedFileContent.end());
#endif

		ASSERT_EQ(expectedFileContent, deserializedFileContent);
	}
};

TEST_F(SerializationTest, serializeNode) {
	const auto sNode{std::make_shared<raco::user_types::Node>("node", "node_id")};
	sNode->scaling_->z.staticQuery<raco::core::RangeAnnotation<double>>().max_ = 100.0;
	auto result = raco::serialization::test_helpers::serializeObject(sNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "Node.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "Node.json").string(), result);
}

TEST_F(SerializationTest, serializeNodeRotated) {
	const auto sNode{context.createObject(raco::user_types::Node::typeDescription.typeName, "node", "node_id")};
	commandInterface.set({sNode, {"rotation", "x"}}, 90.0);
	commandInterface.set({sNode, {"rotation", "y"}}, -90.0);
	commandInterface.set({sNode, {"rotation", "z"}}, 180.0);
	auto result = raco::serialization::test_helpers::serializeObject(sNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "NodeRotated.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "NodeRotated.json").string(), result);
}

TEST_F(SerializationTest, serializeNodeWithAnnotations) {
	const auto sNode{std::make_shared<raco::user_types::Node>("node", "node_id")};

	auto anno = std::make_shared<raco::core::ExternalReferenceAnnotation>();
	sNode->addAnnotation(anno);
	anno->projectID_ = "base_id";

	auto result = raco::serialization::test_helpers::serializeObject(sNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "NodeWithAnnotations.json").string(), result);

	ASSERT_EQ(file::read((test_path() / "expectations" / "NodeWithAnnotations.json").string()), result);
}
TEST_F(SerializationTest, serializeMeshNode) {
	const auto sMeshNode{std::make_shared<raco::user_types::MeshNode>("mesh_node", "mesh_node_id")};
	auto result = raco::serialization::test_helpers::serializeObject(sMeshNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "MeshNode.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "MeshNode.json").string(), result);
}

TEST_F(SerializationTest, serializeMeshNodeWithMesh) {
	const auto sMeshNode{context.createObject(raco::user_types::MeshNode::typeDescription.typeName, "mesh_node", "mesh_node_id")};
	const auto sMesh{context.createObject(raco::user_types::Mesh::typeDescription.typeName, "mesh", "mesh_id")};
	auto uri{(test_relative_path() / "testData" / "duck.glb").string()};
	commandInterface.set({sMesh, {"uri"}}, uri);
	commandInterface.set({sMeshNode, {"mesh"}}, sMesh);

	auto result = raco::serialization::test_helpers::serializeObject(sMeshNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "MeshNodeWithMesh.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "MeshNodeWithMesh.json").string(), result);
}

TEST_F(SerializationTest, serializeNodeWithChildMeshNode) {
	const auto sMeshNode{context.createObject(raco::user_types::MeshNode::typeDescription.typeName, "mesh_node", "mesh_node_id")};
	const auto sNode{context.createObject(raco::user_types::Node::typeDescription.typeName, "node", "node_id")};
	commandInterface.moveScenegraphChildren({sMeshNode}, sNode);
	auto result = raco::serialization::test_helpers::serializeObject(sNode);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "NodeWithChildMeshNode.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "NodeWithChildMeshNode.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScript) {
	const auto sLuaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	auto result = raco::serialization::test_helpers::serializeObject(sLuaScript);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScript.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScript.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptInFloat) {
	const auto sLuaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	auto uri { (test_relative_path() / "testData" / "in-float.lua").string() };
	commandInterface.set(raco::core::ValueHandle{sLuaScript, {"uri"}}, uri);
	auto result = raco::serialization::test_helpers::serializeObject(sLuaScript);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptInFloat.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptInFloat.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptInFloatArray) {
	const auto sLuaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	auto uri{(test_relative_path() / "testData" / "in-float-array.lua").string()};
	commandInterface.set(raco::core::ValueHandle{sLuaScript, {"uri"}}, uri);
	auto result = raco::serialization::test_helpers::serializeObject(sLuaScript);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptInFloatArray.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptInFloatArray.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptInStruct) {
	const auto sLuaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	auto uri{(test_relative_path() / "testData" / "in-struct.lua").string()};
	commandInterface.set(raco::core::ValueHandle{sLuaScript, {"uri"}}, uri);
	auto result = raco::serialization::test_helpers::serializeObject(sLuaScript);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptInStruct.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptInStruct.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptInSpecificPropNames) {
	const auto sLuaScript{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	auto uri{(test_relative_path() / "testData" / "in-specific-prop-names.lua").string()};
	commandInterface.set(raco::core::ValueHandle{sLuaScript, {"uri"}}, uri);
	auto result = raco::serialization::test_helpers::serializeObject(sLuaScript);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptSpecificPropNames.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptSpecificPropNames.json").string(), result);
}

TEST_F(SerializationTest, serializeMesh) {
	const auto sMesh{context.createObject(raco::user_types::Mesh::typeDescription.typeName, "mesh", "mesh_id")};
	auto uri{(test_relative_path() / "testData" / "duck.glb").string()};
	commandInterface.set({sMesh, {"uri"}}, uri);
	auto result = raco::serialization::test_helpers::serializeObject(sMesh);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "Mesh.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "Mesh.json").string(), result);
}

TEST_F(SerializationTest, serializeMeshglTFSubmesh) {
	const auto sMesh{context.createObject(raco::user_types::Mesh::typeDescription.typeName, "mesh", "mesh_id")};
	auto uri{(test_relative_path() / "testData" / "ToyCar.gltf").string()};
	commandInterface.set({sMesh, {"uri"}}, uri);
	commandInterface.set({sMesh, {"bakeMeshes"}}, false);
	commandInterface.set({sMesh, {"meshIndex"}}, 2);
	auto result = raco::serialization::test_helpers::serializeObject(sMesh);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "MeshGLTFSubmesh.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "MeshGLTFSubmesh.json").string(), result);
}

TEST_F(SerializationTest, serializeMeshglTFBakedSubmeshes) {
	const auto sMesh{context.createObject(raco::user_types::Mesh::typeDescription.typeName, "mesh", "mesh_id")};
	auto uri{(test_relative_path() / "testData" / "ToyCar.gltf").string()};
	commandInterface.set({sMesh, {"uri"}}, uri);
	commandInterface.set({sMesh, {"meshIndex"}}, 2);
	commandInterface.set({sMesh, {"bakeMeshes"}}, true);
	auto result = raco::serialization::test_helpers::serializeObject(sMesh);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "MeshGLTFBaked.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "MeshGLTFBaked.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptWithRefToUserTypeWithAnnotation) {
	const auto editorObject{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "mesh", "mesh_id")};
	raco::user_types::SLuaScript sLuaScript{std::dynamic_pointer_cast<raco::user_types::LuaScript>(editorObject)};
	sLuaScript->inputs_->addProperty("ref", new raco::data_storage::Property<raco::user_types::STexture, raco::user_types::EngineTypeAnnotation>({}, {raco::core::EnginePrimitive::TextureSampler2D}));

	auto result = raco::serialization::test_helpers::serializeObject(editorObject);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptWithRefToUserTypeWithAnnotation.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptWithRefToUserTypeWithAnnotation.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptWithURI) {
	const auto editorObject{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	raco::user_types::SLuaScript sLuaScript{std::dynamic_pointer_cast<raco::user_types::LuaScript>(editorObject)};
	sLuaScript->inputs_->addProperty("uri", new raco::data_storage::Property<std::string, raco::user_types::URIAnnotation>("", {}));

	auto result = raco::serialization::test_helpers::serializeObject(editorObject);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptWithURI.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptWithURI.json").string(), result);
}

TEST_F(SerializationTest, serializeLuaScriptWithAnnotatedDouble) {
	const auto editorObject{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	raco::user_types::SLuaScript sLuaScript{std::dynamic_pointer_cast<raco::user_types::LuaScript>(editorObject)};
	sLuaScript->inputs_->addProperty("double", new raco::data_storage::Property<double, raco::user_types::DisplayNameAnnotation, raco::user_types::RangeAnnotation<double>>({}, {"Double"}, {-10.0, 10.0}));

	auto result = raco::serialization::test_helpers::serializeObject(editorObject);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptWithAnnotatedDouble.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptWithAnnotatedDouble.json").string(), result);
}

TEST_F(SerializationTest, serializeNodeAndScript_withLink) {
	const auto editorObject{context.createObject(raco::user_types::LuaScript::typeDescription.typeName, "lua_script", "lua_script_id")};
	raco::user_types::SLuaScript sLuaScript{std::dynamic_pointer_cast<raco::user_types::LuaScript>(editorObject)};
	sLuaScript->inputs_->addProperty("double", new raco::data_storage::Property<double, raco::user_types::DisplayNameAnnotation, raco::user_types::RangeAnnotation<double>>({}, {"Double"}, {-10.0, 10.0}));

	auto result = raco::serialization::test_helpers::serializeObject(editorObject);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptWithAnnotatedDouble.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptWithAnnotatedDouble.json").string(), result);
}

TEST_F(SerializationTest, serializeObjects_luaScriptLinkedToNode) {
	auto objs{raco::createLinkedScene(context, test_relative_path())};
	std::map<std::string, raco::serialization::ExternalProjectInfo> externalProjectsMap;
	std::map<std::string, std::string> originFolders;

	auto result = raco::serialization::serializeObjects(
		{std::get<0>(objs), std::get<1>(objs)}, 
		{std::get<0>(objs)->objectID(), std::get<1>(objs)->objectID()},
		{std::get<2>(objs)}, "", "", "", "", externalProjectsMap, originFolders, -1, false);
	if (WRITE_RESULT) file::write((u8path{CMAKE_CURRENT_SOURCE_DIR} / "expectations" / "LuaScriptLinkedToNode.json").string(), result);

    assertFileContentEqual((test_path() / "expectations" / "LuaScriptLinkedToNode.json").string(), result);
}
