cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(InSearchCreator VERSION 1.0)

option(STATIC "Set to ON to build xlnt as a static library instead of a shared library" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    FIND_PACKAGE_ARGS NAMES GTest
)
FetchContent_MakeAvailable(googletest)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(MSVC)
set_target_properties(gmock gmock_main gtest gtest_main PROPERTIES FOLDER "GTest")
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-char8_t /Zc:char8_t-")

set(SANITIZER_FLAGS "-fsanitize=leak,undefined,address")
set(ERROR_FLAGS "-Wall")
if(MSVC)
	set(ERROR_FLAGS "${ERROR_FLAGS} -W4")
else()
	set(ERROR_FLAGS "${ERROR_FLAGS} -Wpedantic -Werror")
endif()

option(SANITIZE "Enable sanitizers" ON)
if (${SANITIZE})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
endif()

option(DEBUG "Enable debug" ON)
if (${DEBUG})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_compile_definitions(OPENGL RES_FOLDER="${CMAKE_SOURCE_DIR}/")

# add_custom_target(check chmod 777 ${CMAKE_SOURCE_DIR}/lint.sh && ${CMAKE_SOURCE_DIR}/lint.sh)

add_subdirectory(Engine)
add_subdirectory(App)
