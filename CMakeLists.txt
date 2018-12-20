cmake_minimum_required (VERSION 3.12.4)
project (explore-github)
set(CMAKE_C_COMPILER g++)
set(VENDOR_PATH ${PROJECT_SOURCE_DIR}/vendor)
add_compile_options(-std=c++11)
#add_compile_options(-Wc++11-extensions)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(ExternalProject)

set(RAPIDJSONPATH
    ${VENDOR_PATH}/rapidJson/src/rapidJson)

ExternalProject_Add(
    rapidJson
    PREFIX ${RAPIDJSONPATH}
    GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
    GIT_TAG "v1.1.0"
    STEP_TARGETS build
    EXCLUDE_FROM_ALL TRUE
    #BUILD_COMMAND ${CMAKE_COMMAND} -E echo "Starting $<CONFIG> build"
    #COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>
    #COMMAND ${CMAKE_COMMAND} -E echo "$<CONFIG> build complete"
)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS} "${RAPIDJSONPATH}/include/")
add_definitions(-D__STDC_FORMAT_MACROS)
set_property(DIRECTORY PROPERTY COMPILE_OPTIONS ${EXTRA_CXX_FLAGS})

set(LIBS ${RAPIDJSONPATH}/include)
#add_library(explore SHARED main.cpp)
add_executable(explore.exe main.cpp)
target_link_libraries(explore.exe ${CURL_LIBRARIES})
