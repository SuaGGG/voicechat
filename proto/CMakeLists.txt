# 定义proto文件列表
set(PROTO_FILES
    voice_message.proto
)

# 生成Protocol Buffers代码
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES}
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    PROTOC_OPTIONS "--experimental_allow_proto3_optional"
)

# 创建Protocol Buffers库
add_library(voicechat_proto STATIC
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# 设置库的属性
set_target_properties(voicechat_proto PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# 修改这部分：设置库的包含目录
target_include_directories(voicechat_proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 链接Protobuf库
target_link_libraries(voicechat_proto PUBLIC
    protobuf::libprotobuf
) 