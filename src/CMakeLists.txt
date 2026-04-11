aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ src1)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Channel src2)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/handles src3)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Utils src4)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/grpc src5)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc/Codecs src6)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc/rtcp src7)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc/RtpDictionaries src8)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc/sctp src9)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc/SctpDictionaries src10)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtc src11)

include_directories(
    ${PROJECT_SOURCE_DIR}/subprojects/libuv-v1.51.0/include
    ${PROJECT_SOURCE_DIR}/subprojects/libsrtp-3.0.0-beta/include
    ${PROJECT_SOURCE_DIR}/subprojects/openssl-3.0.8/include
    ${PROJECT_SOURCE_DIR}/subprojects/usrsctp-d45b53f5dfa79533f5c5e7aefa5d7570405afb39/usrsctplib
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/protobuf
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/abseil-cpp
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/cares/cares/lib64
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/re2
    ${PROJECT_SOURCE_DIR}/subprojects/nlohmann
    ${PROJECT_SOURCE_DIR}/deps/libwebrtc/libwebrtc
    ${PROJECT_SOURCE_DIR}/deps/libwebrtc
)

link_directories(
    ${PROJECT_SOURCE_DIR}/subprojects/libuv-v1.51.0/build
    ${PROJECT_SOURCE_DIR}/subprojects/libsrtp-3.0.0-beta/build
    ${PROJECT_SOURCE_DIR}/subprojects/openssl-3.0.8
    ${PROJECT_SOURCE_DIR}/subprojects/usrsctp-d45b53f5dfa79533f5c5e7aefa5d7570405afb39/build/usrsctplib
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/protobuf
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/abseil-cpp
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/cares/cares/lib64
    ${PROJECT_SOURCE_DIR}/subprojects/grpc/build/third_party/re2
    )

add_executable(webrtcServer 
               ${src1} ${src2}
               ${src3} ${src4}
               ${src5} ${src6}
               ${src7} ${src8} 
               ${src9} ${src10} 
               ${src11} ${src12}) 

target_link_libraries(webrtcServer
                      libuv.a
                      libsrtp3.a
                      libusrsctp.a
                      libgrpc++.a                      
                      libgrpc.a                      
                      libgrpc_authorization_provider.a
                      libaddress_sorting.a                      
                      libgpr.a                      
                      libupb.a                      
                      libssl.a
                      libcrypto.a
                      libabsl_log_globals.a
                      libabsl_log_internal_message.a
                      libabsl_log_internal_conditions.a
                      libabsl_log_internal_format.a
                      libabsl_log_internal_nullguard.a
                      libabsl_die_if_null.a
                      libabsl_log_internal_log_sink_set.a
                      libabsl_log_initialize.a
                      libabsl_log_internal_check_op.a
                      libabsl_log_entry.a
                      libabsl_log_sink.a
                      libabsl_log_internal_globals.a
                      libabsl_log_flags.a
                      libabsl_log_internal_proto.a
                      libabsl_synchronization.a
                      libabsl_graphcycles_internal.a
                      libabsl_bad_optional_access.a
                      libabsl_bad_any_cast_impl.a
                      libabsl_bad_variant_access.a
                      libabsl_crc_cpu_detect.a
                      libabsl_hash.a
                      libabsl_city.a
                      libabsl_low_level_hash.a
                      libabsl_statusor.a
                      libabsl_status.a
                      libabsl_civil_time.a
                      libabsl_time.a
                      libabsl_time_zone.a
                      libabsl_hashtablez_sampler.a
                      libabsl_raw_hash_set.a
                      libabsl_int128.a
                      libabsl_malloc_internal.a
                      libabsl_strerror.a
                      libabsl_raw_logging_internal.a
                      libabsl_base.a
                      libabsl_scoped_set_env.a
                      libabsl_log_severity.a
                      libabsl_spinlock_wait.a
                      libabsl_exponential_biased.a
                      libabsl_periodic_sampler.a
                      libabsl_random_internal_randen_slow.a
                      libabsl_random_internal_pool_urbg.a
                      libabsl_random_internal_randen_hwaes_impl.a
                      libabsl_random_internal_seed_material.a
                      libabsl_random_seed_sequences.a
                      libabsl_random_internal_randen_hwaes.a
                      libabsl_random_distributions.a
                      libabsl_random_seed_gen_exception.a
                      libabsl_random_internal_distribution_test_util.a
                      libabsl_random_internal_randen.a
                      libabsl_random_internal_platform.a
                      libabsl_cord_internal.a
                      libabsl_throw_delegate.a
                      libabsl_cordz_sample_token.a
                      libabsl_cord.a
                      libabsl_crc32c.a
                      libabsl_crc_cord_state.a
                      libabsl_crc_internal.a
                      libabsl_str_format_internal.a
                      libabsl_cordz_functions.a
                      libabsl_cordz_info.a
                      libabsl_strings.a
                      libabsl_cordz_handle.a
                      libabsl_strings_internal.a
                      libabsl_flags_reflection.a
                      libabsl_flags_commandlineflag.a
                      libabsl_flags_config.a
                      libabsl_flags_usage_internal.a
                      libabsl_flags.a
                      libabsl_flags_parse.a
                      libabsl_flags_marshalling.a
                      libabsl_flags_internal.a
                      libabsl_flags_commandlineflag_internal.a
                      libabsl_flags_private_handle_accessor.a
                      libabsl_flags_program_name.a
                      libabsl_flags_usage.a
                      libabsl_stacktrace.a
                      libabsl_symbolize.a
                      libabsl_failure_signal_handler.a
                      libabsl_leak_check.a
                      libabsl_debugging_internal.a
                      libabsl_examine_stack.a
                      libabsl_demangle_internal.a
                      libcares.a
                      libprotobuf.a
                      libre2.a
                      libabsl_exponential_biased.a
                      z
                      systemd
                      pthread
                      dl
                      webrtc_lib) 