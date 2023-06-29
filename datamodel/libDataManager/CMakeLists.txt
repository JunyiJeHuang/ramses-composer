#[[

This file is part of Ramses Composer
(see https://github.com/GENIVI/ramses-composer).

]]

raco_find_qt_components(Widgets)

add_library(libDataManager
    include/data_Convert/ProgramDefine.h
    include/data_Convert/ProgramManager.h src/ProgramManager.cpp
    include/animation_editor/ConvertEditorAnimation.h src/ConvertEditorAnimation.cpp
    )

target_include_directories(libDataManager
    PUBLIC
    include/
)
enable_warnings_as_errors(libDataManager)

set_target_properties(libDataManager PROPERTIES AUTOMOC TRUE)
set_target_properties(libDataManager PROPERTIES AUTORCC TRUE)
set_target_properties(libDataManager PROPERTIES AUTOUIC TRUE)

target_link_libraries(libDataManager
    PUBLIC
        raco::Core
        raco::UserTypes
        raco::Components
        raco::TimeAxis
        raco::GuiData
        raco::PropertyBrowser
        #-  raco::assets
        openctm
        Qt5::Widgets
        Qt5::Core
        Qt5::Gui
    PRIVATE
        raco::LogSystem
    #-     protobuf::libprotobuf-lite
    #-     protobuf::libprotobuf
    #-     protobuf::libprotoc
)

add_library(raco::dataConvert ALIAS libDataManager)

