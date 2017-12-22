# Project setup
cmake_minimum_required(VERSION 2.6)
project(pbot C)
set (pbot_VERSION_MAJOR 0)
set (pbot_VERSION_MINOR 1)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

set(CMAKE_SYSTEM_NAME "Vocore")
SET(CMAKE_C_COMPILER   /home/furrtek/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-gcc)
SET(CMAKE_CXX_COMPILER /home/furrtek/openwrt/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-uclibc-g++)

# Remove debugging symbols otherwise the binary is >700kB !
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -s")

ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-Wextra)
ADD_DEFINITIONS(-Wno-unused)
ADD_DEFINITIONS(-Wsign-compare)
ADD_DEFINITIONS(-g3 -O0)
ADD_DEFINITIONS(-std=gnu99)

find_path(LIBRTLSDR_INCLUDE_DIRS NAMES rtl-sdr.h
	PATHS
	${LIBRTLSDR_PKG_INCLUDE_DIRS}
	/usr/include
	/usr/local/include
)

if(NOT LIBRTLSDR_INCLUDE_DIRS)
	message(STATUS "librtlsdr include dir not found")
endif(NOT LIBRTLSDR_INCLUDE_DIRS)

message(STATUS ${LIBRTLSDR_INCLUDE_DIRS})

# Setup the include and linker paths
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/liboauth
	${LIBRTLSDR_INCLUDE_DIRS}
)

set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix \${prefix})
set(libdir \${exec_prefix}/lib)
set(includedir \${prefix}/include)

FILE(GLOB liboauth_sources ./liboauth/*.c)

add_executable(pbot main.c fsk.c leds.c tweet.c sdr.c ${liboauth_sources})

target_link_libraries(
    pbot
    ${CMAKE_THREAD_LIBS_INIT}
	rtlsdr
	pthread
	m
	curl
	libcrypto.so
	libssl.so
)
