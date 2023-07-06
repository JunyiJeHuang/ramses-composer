#[[
SPDX-License-Identifier: MPL-2.0

This file is part of Ramses Composer
(see https://github.com/GENIVI/ramses-composer).

This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

add_library(libGuiData
    include/CurveData/CurveData.h src/CurveData.cpp
    include/CurveData/CurveManager.h src/CurveManager.cpp
    include/NodeData/NodeManager.h src/NodeManager.cpp
    include/NodeData/NodeDataEx.h src/NodeDataEx.cpp
    include/AnimationData/AnimationData.h src/AnimationData.cpp
    include/PropertyData/PropertyData.h src/PropertyData.cpp
    include/MaterialData/MaterialData.h  src/MaterialData.cpp
    include/MaterialData/MaterialManager.h  src/MaterialManager.cpp
    include/MeshData/MeshDataManager.h src/MeshDataManager.cpp
    include/RenderData/RenderDataManager.h src/RenderDataManager.cpp
    include/PropertyData/PropertyType.h
    include/FolderData/FolderDataManager.h src/FolderDataManager.cpp
    include/VisualCurveData/VisualCurvePosManager.h src/VisualCurvePosManager.cpp
)

target_include_directories(libGuiData PUBLIC include)
enable_warnings_as_errors(libGuiData)


target_link_libraries(libGuiData
PUBLIC
        raco::Core
        raco::PropertyBrowser
PRIVATE
        raco::LogSystem
)


add_library(raco::GuiData ALIAS libGuiData)
