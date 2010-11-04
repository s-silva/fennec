add_definitions(-DMPP_ENCODER -DFAST_MATH -DCVD_FASTLOG)
add_executable(mppenc analy_filter encode_sv7 huffsv7 profile stderr winmsg ans fastmath keyboard psy tags bitstream fft4g mppenc psy_tab tools cvd fft_routines pipeopen quant wave_in)

if(WIN32)
target_link_libraries(mppenc winmm)
endif(WIN32)

if(NOT MSVC)
target_link_libraries(mppenc m)
endif(NOT MSVC)

if(CMAKE_SYSTEM_NAME STREQUAL OpenBSD)
target_link_libraries(mppenc ossaudio)
endif(CMAKE_SYSTEM_NAME STREQUAL OpenBSD)

install(TARGETS mppenc RUNTIME DESTINATION bin)
