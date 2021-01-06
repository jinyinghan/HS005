#SET(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "set build type to MinSizeRel")
#SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_C_COMPILER "mips-linux-gnu-gcc")
#SET(CMAKE_CXX_COMPILER "mips-linux-gnu-g++")

SET(PROG_NAME carrier-server-multicast)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMULTICAST_FLAGS")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMULTICAST_FLAGS")

#IF(NOT EXISTS $ENV{ISVP_ENV_RELEASE_DIR_SDK})
#    MESSAGE(FATAL_ERROR "SDK prebuilt NOT found, please cd to isvp top and run \"source build/env_setup.sh first\"")
#ENDIF()
#IF(NOT EXISTS $ENV{ISVP_ENV_PREBUILT_DIR_MXU_DEBUG})
#    MESSAGE(FATAL_ERROR "MXU prebuilt NOT found, please cd to isvp top and run \"source build/env_setup.sh first\"")
#ENDIF()
#IF(NOT EXISTS $ENV{ISVP_ENV_PREBUILT_DIR_IVS_DEBUG})
#    MESSAGE(FATAL_ERROR "IVS prebuilt NOT found, please cd to isvp top and run \"source build/env_setup.sh first\"")
#ENDIF()
#IF(NOT EXISTS $ENV{ISVP_ENV_PREBUILT_DIR_ALGO})
#    MESSAGE(FATAL_ERROR "ALGO prebuilt NOT found, please cd to isvp top and run \"source build/env_setup.sh first\"")
#ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
#SET(LIBTYPE "glibc")

SET(SRCS
    Sample-RTSPServer.cpp
 #   H264VideoServerMediaSubsession.cpp
    ../Options.cpp
    ../VideoInput.cpp
    ../H264VideoStreamSource.cpp
    ../AudioInput.cpp
    ../AudioRTPCommon.cpp
    ../AudioServerMediaSubsession.cpp
    ../AudioStreamSource.cpp
    ../RCF/RCF/RCF.cpp
    ../RCF/class/CarrierCommand.cpp
    ../RCF/RCFServer.cpp
)

SET(SDK_INC_DIR $ENV{ISVP_ENV_RELEASE_DIR_SDK}/include)
SET(IVS_INC_DIR $ENV{ISVP_ENV_PREBUILT_DIR_IVS_DEBUG}/include)
SET(ALGO_INC_DIR $ENV{ISVP_ENV_PREBUILT_DIR_ALGO}/include)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/live555/prebuilt/include/BasicUsageEnvironment)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/live555/prebuilt/include/UsageEnvironment)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/live555/prebuilt/include/groupsock)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/live555/prebuilt/include/liveMedia)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/RCF/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/RCF/class)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/RCF)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${SDK_INC_DIR})
INCLUDE_DIRECTORIES(${IVS_INC_DIR})
INCLUDE_DIRECTORIES(${ALGO_INC_DIR})

MACRO(DEFPROG LIBTYPE)
    MESSAGE(STATUS "Build: ${LIBTYPE}")

    SET(SDK_LIB_DIR $ENV{ISVP_ENV_RELEASE_DIR_SDK}/lib/${LIBTYPE})
    SET(MXU_LIB_DIR $ENV{ISVP_ENV_PREBUILT_DIR_MXU_DEBUG}/lib/${LIBTYPE})
    SET(IVS_LIB_DIR $ENV{ISVP_ENV_PREBUILT_DIR_IVS_DEBUG}/lib/${LIBTYPE})
    SET(ALGO_LIB_DIR $ENV{ISVP_ENV_PREBUILT_DIR_ALGO}/lib/${LIBTYPE})

    ADD_EXECUTABLE(${PROG_NAME}-${LIBTYPE}-shared ${SRCS})
    ADD_EXECUTABLE(${PROG_NAME}-${LIBTYPE}-static ${SRCS})

    ADD_CUSTOM_COMMAND(TARGET ${PROG_NAME}-${LIBTYPE}-shared POST_BUILD
        COMMAND ${CMAKE_STRIP} ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}/${PROG_NAME}-shared
    )

    ADD_CUSTOM_COMMAND(TARGET ${PROG_NAME}-${LIBTYPE}-static POST_BUILD
        COMMAND ${CMAKE_STRIP} ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}/${PROG_NAME}-static
    )
    TARGET_LINK_LIBRARIES(${PROG_NAME}-${LIBTYPE}-shared
        ${IVS_LIB_DIR}/libface_inf.so
	${IVS_LIB_DIR}/libfigure_inf.so
	${IVS_LIB_DIR}/libline_inf.so
	${IVS_LIB_DIR}/libmove_inf.so
	${IVS_LIB_DIR}/libperm_inf.so
	${IVS_LIB_DIR}/liblpr_inf.so
	${IVS_LIB_DIR}/libshade_inf.so
	${MXU_LIB_DIR}/libmxu_objdetect.so
	${MXU_LIB_DIR}/libmxu_contrib.so
	${MXU_LIB_DIR}/libmxu_merge.so
	${MXU_LIB_DIR}/libmxu_video.so
	${MXU_LIB_DIR}/libmxu_imgproc.so
	${MXU_LIB_DIR}/libmxu_core.so
	${SDK_LIB_DIR}/libimp.so
	${ALGO_LIB_DIR}/libisave.so
	${SDK_LIB_DIR}/libsysutils.so
	${SDK_LIB_DIR}/libalog.so
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libliveMedia.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libBasicUsageEnvironment.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libUsageEnvironment.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libgroupsock.a
	pthread rt dl
    )

    TARGET_LINK_LIBRARIES(${PROG_NAME}-${LIBTYPE}-static
        ${IVS_LIB_DIR}/libface_inf.a
        ${IVS_LIB_DIR}/libfigure_inf.a
	${IVS_LIB_DIR}/libline_inf.a
	${IVS_LIB_DIR}/libmove_inf.a
	${IVS_LIB_DIR}/libperm_inf.a
	${IVS_LIB_DIR}/liblpr_inf.a
	${IVS_LIB_DIR}/libshade_inf.a
	${MXU_LIB_DIR}/libmxu_objdetect.a
	${MXU_LIB_DIR}/libmxu_contrib.a
	${MXU_LIB_DIR}/libmxu_merge.a
	${MXU_LIB_DIR}/libmxu_video.a
	${MXU_LIB_DIR}/libmxu_imgproc.a
	${MXU_LIB_DIR}/libmxu_core.a
	${SDK_LIB_DIR}/libimp.a
	${ALGO_LIB_DIR}/libisave.a
	${SDK_LIB_DIR}/libsysutils.a
	${SDK_LIB_DIR}/libalog.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libliveMedia.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libBasicUsageEnvironment.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libUsageEnvironment.a
	${PROJECT_SOURCE_DIR}/live555/prebuilt/lib/${LIBTYPE}/libgroupsock.a
	pthread rt dl
    )

    IF(${LIBTYPE} STREQUAL glibc)
    SET_TARGET_PROPERTIES(${PROG_NAME}-${LIBTYPE}-shared PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}
            OUTPUT_NAME "${PROG_NAME}-shared"
	    LINK_FLAGS "-Wl,-gc-sections"
	)

    SET_TARGET_PROPERTIES(${PROG_NAME}-${LIBTYPE}-static PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}
            OUTPUT_NAME "${PROG_NAME}-static"
	    LINK_FLAGS "-Wl,-gc-sections"
    )
    ELSE()
        SET_TARGET_PROPERTIES(${PROG_NAME}-${LIBTYPE}-shared PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}
            OUTPUT_NAME "${PROG_NAME}-shared"
	    COMPILE_FLAGS "-muclibc"
	    LINK_FLAGS "-muclibc -Wl,-gc-sections"
	)

        SET_TARGET_PROPERTIES(${PROG_NAME}-${LIBTYPE}-static PROPERTIES
	    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/${LIBTYPE}
            OUTPUT_NAME "${PROG_NAME}-static"
	    COMPILE_FLAGS "-muclibc"
	    LINK_FLAGS "-muclibc -Wl,-gc-sections"
        )
    ENDIF()
ENDMACRO()

DEFPROG(glibc)
DEFPROG(uclibc)

