#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VoiceChat::voicechat_lib" for configuration ""
set_property(TARGET VoiceChat::voicechat_lib APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(VoiceChat::voicechat_lib PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libvoicechat_lib.so.1.0.0"
  IMPORTED_SONAME_NOCONFIG "libvoicechat_lib.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS VoiceChat::voicechat_lib )
list(APPEND _IMPORT_CHECK_FILES_FOR_VoiceChat::voicechat_lib "${_IMPORT_PREFIX}/lib/libvoicechat_lib.so.1.0.0" )

# Import target "VoiceChat::voicechat_proto" for configuration ""
set_property(TARGET VoiceChat::voicechat_proto APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(VoiceChat::voicechat_proto PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libvoicechat_proto.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS VoiceChat::voicechat_proto )
list(APPEND _IMPORT_CHECK_FILES_FOR_VoiceChat::voicechat_proto "${_IMPORT_PREFIX}/lib/libvoicechat_proto.a" )

# Import target "VoiceChat::voice_server" for configuration ""
set_property(TARGET VoiceChat::voice_server APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(VoiceChat::voice_server PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/voice_server"
  )

list(APPEND _IMPORT_CHECK_TARGETS VoiceChat::voice_server )
list(APPEND _IMPORT_CHECK_FILES_FOR_VoiceChat::voice_server "${_IMPORT_PREFIX}/bin/voice_server" )

# Import target "VoiceChat::voice_client" for configuration ""
set_property(TARGET VoiceChat::voice_client APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(VoiceChat::voice_client PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/voice_client"
  )

list(APPEND _IMPORT_CHECK_TARGETS VoiceChat::voice_client )
list(APPEND _IMPORT_CHECK_FILES_FOR_VoiceChat::voice_client "${_IMPORT_PREFIX}/bin/voice_client" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
