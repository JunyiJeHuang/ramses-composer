#[[
SPDX-License-Identifier: MPL-2.0

This file is part of Ramses Composer
(see https://github.com/bmwcarit/ramses-composer).

This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

set(SPDLOG_WCHAR_FILENAMES ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog/)
set_target_properties(spdlog PROPERTIES FOLDER third_party)

set(BUILD_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(Qt-Advanced-Docking-System/ EXCLUDE_FROM_ALL)
set_target_properties(qtadvanceddocking PROPERTIES FOLDER third_party)

# Mesh file libraries
## CTM
add_subdirectory(OpenCTM-1.0.3/)
set_target_properties(openctm PROPERTIES FOLDER third_party/openctm POSITION_INDEPENDENT_CODE ON)
set_target_properties(lzma PROPERTIES FOLDER third_party/openctm POSITION_INDEPENDENT_CODE ON)


## TINYGLTF
set(TINYGLTF_USE_CPP14 ON CACHE BOOL "" FORCE)
set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_NO_EXTERNAL_IMAGE OFF CACHE INTERNAL "" FORCE)
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE tinygltf/)
set_target_properties(tinygltf PROPERTIES
    FOLDER third_party/tinygltf
)

## GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm/)
set_target_properties(glm PROPERTIES
    FOLDER third_party/glm
)

## pybind11
add_subdirectory(pybind11)
add_library(raco_pybind11 INTERFACE)
target_sources(raco_pybind11 PUBLIC raco_pybind11_embed.h)
target_include_directories(raco_pybind11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(raco_pybind11 INTERFACE pybind11::embed)
if(PYTHON_DEBUG_LIBRARY_AVAILABLE AND WIN32)
    # Needed to avoid pybind11/embed.h to cause linking to the non-debug DLL if the debug DLL is available.
    # See https://github.com/pybind/pybind11/issues/3403#issuecomment-962878324
	target_compile_definitions(raco_pybind11 INTERFACE $<$<CONFIG:Debug>:PYTHON_DEBUG_LIBRARY_AVAILABLE>)
endif()
set_target_properties(raco_pybind11 PROPERTIES
    FOLDER third_party/raco_pybind11
)
add_library(raco::pybind11 ALIAS raco_pybind11)

## zip
add_subdirectory(zip/)
set_target_properties(zip uninstall test_append.out test_entry.out test_extract.out test_permissions.out test_read.out test_write.out PROPERTIES
    FOLDER third_party/zip
)

# Configure ramses-sdk build options
set(ramses-sdk_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_IVI_TEST_APPS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_DEMO_ECM OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_SMOKE_TESTS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_ENABLE_WAYLAND_SHELL  OFF CACHE BOOL "" FORCE)
set(ramses-sdk_ENABLE_DLT OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_WITH_LTO  OFF CACHE BOOL "" FORCE)
set(ramses-sdk_FORCE_OFF_SOMEIP_IC ON CACHE BOOL "" FORCE)
set(ramses-sdk_FORCE_USE_SOMEIP_IC OFF CACHE BOOL "" FORCE)

# These need to be set
set(ramses-sdk_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_FULL_SHARED_LIB ON CACHE BOOL "" FORCE)
set(ramses-sdk_BUILD_CLIENT_ONLY_SHARED_LIB ON CACHE BOOL "" FORCE)
set(ramses-sdk_CONSOLE_LOGLEVEL "info" CACHE STRING "" FORCE)
# TODO: why are we building ramses-daemon?
set(ramses-sdk_BUILD_TOOLS ON CACHE BOOL "" FORCE)

# TODO: check if we need to enable this
set(ramses-sdk_ENABLE_TCP_SUPPORT OFF CACHE BOOL "" FORCE)

add_subdirectory(ramses-logic/external/ramses/ EXCLUDE_FROM_ALL)

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    # es-3-0 / 4-5 / 4-2-core
    add_library(raco::ramses-lib ALIAS ramses-shared-lib-windows-wgl-4-5)
endif()
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    add_library(raco::ramses-lib ALIAS ramses-shared-lib-x11-egl-es-3-0)
endif()

add_library(raco::ramses-lib-client-only ALIAS ramses-shared-lib-client-only)


set(ramses-logic_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ramses-logic_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(ramses-logic_ALLOW_RAMSES_BUILD OFF CACHE BOOL "" FORCE)
set(ramses-logic_RAMSES_TARGET "raco::ramses-lib" CACHE STRING "" FORCE)
set(ramses-logic_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(ramses-logic/ EXCLUDE_FROM_ALL)

add_library(ramses-logic-api INTERFACE)
target_include_directories(ramses-logic-api INTERFACE
    ramses-logic/include
)

function(add_ramses_logic_target TARGET RAMSES_TARGET)
    add_library(${TARGET} SHARED)
    target_link_libraries(${TARGET} PRIVATE ramses-logic-obj)
    target_link_libraries(${TARGET} PUBLIC ${RAMSES_TARGET})
    target_include_directories(${TARGET} PUBLIC ramses-logic/include)
    set_target_properties(${TARGET} PROPERTIES
        PUBLIC_HEADER "${public_headers}"
        SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
    folderize_target(${TARGET} "third_party/ramses-logic")
endfunction()

add_library(raco::ramses-logic-lib ALIAS ramses-logic)

add_ramses_logic_target(ramses-logic-shared-lib-client-only ramses-shared-lib-client-only)
add_library(raco::ramses-logic-lib-client-only ALIAS ramses-logic-shared-lib-client-only)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_program(LINUXDEPLOYQT linuxdeployqt)
		if(EXISTS "${LINUXDEPLOYQT}")
            add_custom_command(OUTPUT  ${CMAKE_BINARY_DIR}/ramses-logic-viewer
            COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:RaCoEditor>/ramses-logic-viewer.sh"
            COMMAND ${CMAKE_COMMAND} -D TARGET_FILE=$<TARGET_FILE_DIR:RaCoEditor>/ramses-logic-viewer -D ROOT_DIR=${CMAKE_SOURCE_DIR} -P "${CMAKE_SOURCE_DIR}/ubuntustartscript.cmake"
            COMMAND chmod +x "$<TARGET_FILE_DIR:RaCoEditor>/ramses-logic-viewer.sh" 
            DEPENDS ramses-logic-viewer RaCoEditor
            )
        
            add_custom_target(generate_ramses_logic_viewer_launch_script ALL
            DEPENDS ${CMAKE_BINARY_DIR}/ramses-logic-viewer
            )
        endif()
endif()

if(WIN32)
    add_subdirectory(python-windows)
elseif(UNIX)
    add_subdirectory(python-linux)
endif()

register_python_target(RaCoPythonModuleDeployment ${PYTHON_DEPLOYMENT_FOLDER})
register_python_target(RaCoHeadlessPythonModuleDeployment ${HEADLESS_PYTHON_DEPLOYMENT_FOLDER})

macro(deploy_python_folder tgt)
    add_dependencies(${tgt} RaCoPythonModuleDeployment)
endmacro()

#protobuf
#- add_subdirectory(protobuf)
#- set_target_properties(libprotobuf-lite PROPERTIES FOLDER third_party/protobuf POSITION_INDEPENDENT_CODE ON)
#- set_target_properties(libprotobuf PROPERTIES FOLDER third_party/protobuf POSITION_INDEPENDENT_CODE ON)
#- set_target_properties(libprotoc PROPERTIES FOLDER third_party/protobuf POSITION_INDEPENDENT_CODE ON)

macro(deploy_python_folder_headless tgt)
    add_dependencies(${tgt} RaCoHeadlessPythonModuleDeployment)
endmacro()