include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/bitrate_controller src1)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/pacing src2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/remote_bitrate_estimator src3)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/congestion_controller/rtp src4)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/congestion_controller/goog_cc src5)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/api src6)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/api/transport src7)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/api/units src8)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/call src9)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/rtc_base/experiments src10)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/rtc_base/network src11)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/system_wrappers/source src12)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/rtc_base src13)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/libwebrtc/libwebrtc/modules/rtp_rtcp/include src14)

add_library(webrtc_lib 
            ${src1} ${src2}
            ${src3} ${src4}
            ${src5} ${src6}
            ${src7} ${src8}
            ${src9} ${src10}
            ${src11} ${src12}
            ${src13} ${src14})