# 收集源文件
set(LIB_SOURCES
    audio_device.cpp
    opus_codec.cpp
    asio_network.cpp
    voice_server.cpp
    voice_client.cpp
)

# 收集头文件
set(PUBLIC_HEADERS
    ../include/audio_interface.hpp
    ../include/network_interface.hpp
    ../include/audio_device.hpp
    ../include/opus_codec.hpp
    ../include/asio_network.hpp
    ../include/voice_server.hpp
    ../include/voice_client.hpp
)

# 创建共享库
add_library(voicechat_lib SHARED
    ${LIB_SOURCES}
)

# 设置库的属性
set_target_properties(voicechat_lib PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

# 设置库的包含目录
target_include_directories(voicechat_lib 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/proto
        ${Boost_INCLUDE_DIRS}
        ${PORTAUDIO_INCLUDE_DIRS}
        ${OPUS_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
)

# 链接依赖库
target_link_libraries(voicechat_lib 
    PUBLIC
        voicechat_proto
        Boost::system
        Boost::thread
        ${PORTAUDIO_LIBRARIES}
        ${OPUS_LIBRARIES}
        ${Protobuf_LIBRARIES}
        opus
)

# 创建可执行文件
add_executable(voice_server server/main.cpp)
add_executable(voice_client client/main.cpp)

# 链接库
target_link_libraries(voice_server PRIVATE voicechat_lib)
target_link_libraries(voice_client PRIVATE voicechat_lib)

# 统一的安装配置
install(TARGETS voicechat_lib voicechat_proto voice_server voice_client
    EXPORT VoiceChatTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# 安装导出目标
install(EXPORT VoiceChatTargets
    NAMESPACE VoiceChat::
    FILE VoiceChatTargets.cmake
    DESTINATION lib/cmake/VoiceChat
) 