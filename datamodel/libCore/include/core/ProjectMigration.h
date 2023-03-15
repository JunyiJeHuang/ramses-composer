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

#include "core/ProxyObjectFactory.h"
#include "core/Serialization.h"

namespace raco::serialization {
/**
 * History of versions:
 * 1: Initial
 * 2: Added [core::ProjectSettings].
 * 3: Added [viewport in core::ProjectSettings]
 * 4: Changed indexing of arrays [user_types::SyncTableWithEngineInterface]
 * 5: Added submesh index selection and baking flag [user_types::Mesh]
 *    added texture origin [user_types::Texture] 
 *    removed cubeMap format [user_types::CubeMap]
 * 6: Added URI for shader defines [user_types::Material]
 * 7: Prefab support: 
 *    added prefab child -> instance child map in PrefabInstance
 * 8: Added link validity flag to links
 *    Links can get broken now; these broken links get serialized just like normal links
 * 9: External references
 * 10: The viewport property in cameras is now four individual integers instead of a vec4i,
 *    needed for camera bindings [user_types::BaseCamera].
 * 11: Added the viewport background color to the ProjectSettings.
 * 12: Added 'private' property in meshnode material container.
 *     Renamed 'depthfunction' ->  'depthFunction' in options container of meshnode material slot.
 *     Added LinkEndAnnotation to material uniform properties
 * 13: Introduced Struct properties and converted
 *     - material and meshnode blend options
 *     - camera viewport and frustum
 * 14: Replaced "U/V Origin" enum with Texture flip flag
 *     Origin "Top Left" -> flag enabled
 * 15: Added offscreen rendering support:
 *     - changed texture uniform type for normal 2D textures from STexture -> STextureSampler2DBase
 *     - added tag properties to Node and Material user types.
 * 16: New Ramses version that changes transformation order
 *     (previously: Translation * Scale * Rotation, now: Translation * Rotation * Scale)
 * 17: Remove "Optimized" from options in render layer sort order
 * 18: Added Animation and AnimationChannel types
 * 19: Changed ProjectSettings::backgroundColor from Vec3f to Vec4f
 *     Links from Vec4f to Node::rotation are now allowed
 * 20: Added LuaScriptModule type as well as basic Lua module support
 * 21: Added mipmap flag to textures.
 * 22: Added support for setting default resource folders per project
 * 23: Serialization changes to support new-style migration code
 * 24: Deterministics object IDs for PrefabInstance child objects
 * 25: Added mipmap flag to cubemaps
 * 26: Added support for ramses-logic INT64 type
 * 27: Removed vector PrimitiveTypes and made them normal structs. 
 *     Vector types are now included in the serialized struct type map.
 * 28: Added zipability functionality to projects.
 *     Added "Save As Zipped File" option to Project Settings.
 * 29: CubeMap: added custom mipmap functionality (+ 18 URIs, 1 bool, 1 int property)
 * 30: Animation property changes: removed loop, play, rewindOnStop properties and added progress property.
 * 31: Added texture format enum property to Texture and CubeMap
 * 32: Added Timer type
 * 33: Added HiddenProperty annotation to all tag-related properties
 * 34: Replaced RenderLayer invertMaterialFilter bool by materialFilterMode int property.
 * 35: Removed redundant information from .rca file.
 *     This needed changes in the loading code so the files can't be loaded with older version anymore.
 * 36: Added LuaInterface user type.
 *     Added LinkEndAnnotation to LuaScript luaInputs property
 * 37: Added stdModules property to LuaScript and LuaScriptModule types.
 * 38: Texture: added custom mipmap functionality (+ 3 URIs, 1 int property)
 * 39: Added interfaceSubdirectory to the default resource folder struct in the ProjectSettings.
 * 40: Renamed 6 user type properties
 * 41: Renamed Animation property: "animationOutputs" -> "outputs"
 * 42: Added isWeak flag to links.
 * 43: Removed enableTimerFlag and runTimer properties in ProjectSettings.
 * 44: Added ProjectSettings "featureLevel" property
 *     Added "enabled" property to Node.
 *     Added AnchorPoint user type.
 *     RenderPass property changes
 *     - added "renderOnce" property
 *     - renamed "order" -> "renderOrder"
 *     - made 'enabled', 'renderOrder', and 'clearColor' linkable
 *     Added "frustumType" property to PerspectiveCamera and changed frustum from static struct to dynamic Table.
 * 45: RenderLayer changes
*      - added LinkEndAnnotation to all properties in the renderableTags property
*      - removed HiddenProperty from renderableTags property
 * 46: Added BlitPass usertype.
 *     Added RenderBufferMS usertype.
 *     Added 8 RenderBufferMS reference properties to RenderTarget.
 *     Changed BaseCamera viewport width and height ranges.
 *     Added ExpectEmptyReference annotation to RenderTarget::buffer0 property.
 * 47: Added Skin user type.
 * 48: Added LuaInterface "stdModules" and "luaModules" properties.
 * 49: Added TextureExternal user type.
 * 50: Added "userTags" property to EditorObject.
 * 51: Added support for struct uniforms.
 * 52: Added z_up to the ProjectSettings
 */
constexpr int RAMSES_PROJECT_FILE_VERSION = 52;

void migrateProject(ProjectDeserializationInfoIR& deserializedIR, raco::serialization::proxy::ProxyObjectFactory& factory);

}  // namespace raco::serialization
