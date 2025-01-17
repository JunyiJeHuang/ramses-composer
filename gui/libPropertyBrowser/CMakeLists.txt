#[[
SPDX-License-Identifier: MPL-2.0

This file is part of Ramses Composer
(see https://github.com/bmwcarit/ramses-composer).

This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

find_package(Qt5 COMPONENTS Widgets Test REQUIRED)

add_library(libPropertyBrowser
    include/property_browser/controls/ExpandButton.h src/controls/ExpandButton.cpp
    include/property_browser/controls/MouseWheelGuard.h src/controls/MouseWheelGuard.cpp
    include/property_browser/controls/ScalarSlider.h src/controls/ScalarSlider.cpp
    include/property_browser/controls/SpinBox.h src/controls/SpinBox.cpp

    include/property_browser/editors/BoolEditor.h src/editors/BoolEditor.cpp
    include/property_browser/editors/DoubleEditor.h src/editors/DoubleEditor.cpp
    include/property_browser/editors/EnumerationEditor.h src/editors/EnumerationEditor.cpp
    include/property_browser/editors/Int64Editor.h src/editors/Int64Editor.cpp
    include/property_browser/editors/IntEditor.h src/editors/IntEditor.cpp
    include/property_browser/editors/LinkEditor.h src/editors/LinkEditor.cpp
    include/property_browser/editors/PropertyEditor.h src/editors/PropertyEditor.cpp
    include/property_browser/editors/RefEditor.h src/editors/RefEditor.cpp
    include/property_browser/editors/StringEditor.h src/editors/StringEditor.cpp
    include/property_browser/editors/TagContainerEditor.h src/editors/TagContainerEditor.cpp
    include/property_browser/editors/URIEditor.h src/editors/URIEditor.cpp
    include/property_browser/editors/VecNTEditor.h src/editors/VecNTEditor.cpp

    include/property_browser/PropertyBrowserEditorPopup.h src/PropertyBrowserEditorPopup.cpp
    include/property_browser/PropertyBrowserItem.h src/PropertyBrowserItem.cpp
    include/property_browser/PropertyBrowserLayouts.h
    include/property_browser/PropertyBrowserModel.h
    include/property_browser/PropertyBrowserRef.h src/PropertyBrowserRef.cpp
    include/property_browser/PropertyBrowserWidget.h src/PropertyBrowserWidget.cpp
    include/property_browser/PropertySubtreeChildrenContainer.h src/PropertySubtreeChildrenContainer.cpp
    include/property_browser/PropertySubtreeView.h src/PropertySubtreeView.cpp
    include/property_browser/WidgetFactory.h src/WidgetFactory.cpp

    include/property_browser_extend/PropertyBrowserNodeWidget.h  src/property_browser_extend/PropertyBrowserNodeWidget.cpp
    include/property_browser_extend/PropertyBrowserMeshWidget.h src/property_browser_extend/PropertyBrowserMeshWidget.cpp
    include/property_browser_extend/PropertyBrowserCustomWidget.h src/property_browser_extend/PropertyBrowserCustomWidget.cpp
    include/property_browser_extend/PropertyBrowserCustomView.h src/property_browser_extend/PropertyBrowserCustomView.cpp
    include/property_browser_extend/PropertyBrowserCurveBindingWidget.h src/property_browser_extend/PropertyBrowserCurveBindingWidget.cpp
    include/property_browser_extend/PropertyBrowserCurveBindingView.h src/property_browser_extend/PropertyBrowserCurveBindingView.cpp
    include/property_browser_extend/controls/ExpandControlNoItemButton.h  src/property_browser_extend/controls/ExpandControlNoItemButton.cpp

    include/property_browser/editors/BoolEditor.h src/editors/BoolEditor.cpp
    include/property_browser/editors/DoubleEditor.h src/editors/DoubleEditor.cpp
    include/property_browser/editors/EnumerationEditor.h src/editors/EnumerationEditor.cpp
    include/property_browser/editors/IntEditor.h src/editors/IntEditor.cpp
    include/property_browser/editors/Int64Editor.h src/editors/Int64Editor.cpp
    include/property_browser/editors/LinkEditor.h src/editors/LinkEditor.cpp
    include/property_browser/editors/PropertyEditor.h src/editors/PropertyEditor.cpp
    include/property_browser/editors/RefEditor.h src/editors/RefEditor.cpp
    include/property_browser/editors/StringEditor.h src/editors/StringEditor.cpp
    include/property_browser/editors/TagContainerEditor.h src/editors/TagContainerEditor.cpp
    include/property_browser/editors/URIEditor.h src/editors/URIEditor.cpp
    include/property_browser/editors/VecNTEditor.h src/editors/VecNTEditor.cpp
    include/property_browser/editors/SearchEditor.h src/editors/SearchEditor.cpp
    
    include/property_browser/controls/ExpandButton.h src/controls/ExpandButton.cpp
    include/property_browser/controls/MouseWheelGuard.h src/controls/MouseWheelGuard.cpp
    include/property_browser/controls/ScalarSlider.h src/controls/ScalarSlider.cpp
    include/property_browser/controls/SpinBox.h src/controls/SpinBox.cpp
    include/property_browser/controls/ResultTree.h src/controls/ResultTree.cpp

    src/ErrorBox.h src/ErrorBox.cpp
    src/editors/TagContainerEditor/TagContainerEditor_Popup.h src/editors/TagContainerEditor/TagContainerEditor_Popup.cpp
    src/editors/TagContainerEditor/TagContainerEditor_AvailableTagsItemModel.h src/editors/TagContainerEditor/TagContainerEditor_AvailableTagsItemModel.cpp

    src/editors/TagContainerEditor/TagContainerEditor_AppliedTagModel.h src/editors/TagContainerEditor/TagContainerEditor_AppliedTagModel.cpp
    src/editors/TagContainerEditor/TagContainerEditor_AvailableTagsItemModel.h src/editors/TagContainerEditor/TagContainerEditor_AvailableTagsItemModel.cpp
    src/editors/TagContainerEditor/TagContainerEditor_Popup.h src/editors/TagContainerEditor/TagContainerEditor_Popup.cpp
    src/editors/TagContainerEditor/TreeViewWithDel.h
    src/ErrorBox.h src/ErrorBox.cpp
)

target_include_directories(libPropertyBrowser
    PUBLIC
        include/
)
enable_warnings_as_errors(libPropertyBrowser)

set_target_properties(libPropertyBrowser PROPERTIES AUTOMOC TRUE)
set_target_properties(libPropertyBrowser PROPERTIES AUTORCC TRUE)
set_target_properties(libPropertyBrowser PROPERTIES AUTOUIC TRUE)

target_link_libraries(libPropertyBrowser
PUBLIC
    raco::Core
    raco::UserTypes
    raco::Components
    raco::CommonWidgets
    raco::TimeAxis
    raco::node_logic
    raco::material_logic
    raco::GuiData
    raco::signal
    Qt5::Widgets
    raco::dataConvert
PRIVATE
    raco::Style
    lua::lua
)

add_library(raco::PropertyBrowser ALIAS libPropertyBrowser)

if(PACKAGE_TESTS)
    add_subdirectory(tests)
endif()
