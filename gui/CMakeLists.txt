# 设置 Qt 自动生成的文件目录
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# 添加可执行文件
add_executable(voice_chat_qt
    ui/src/main.cpp
    ui/src/mainwindow.cpp
    ui/src/voiceclient.cpp
    ui/include/mainwindow.h
    ui/include/voiceclient.h
    ui/mainwindow.ui
)

# 设置包含目录
target_include_directories(voice_chat_qt PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 链接依赖库
target_link_libraries(voice_chat_qt PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    voicechat_lib
) 