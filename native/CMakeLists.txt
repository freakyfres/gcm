cmake_minimum_required(VERSION 3.15...3.31)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(goodClipboardManagerNative)

add_compile_definitions(-DNAPI_VERSION=7)

file(GLOB_RECURSE SOURCE_FILES "src/**.cpp")

execute_process(COMMAND node -e "f = require('fs');n=require('node-addon-api').include;f.writeFileSync(1, n.substring(1, n.length-1, 'utf-8'))"
  OUTPUT_VARIABLE NAPI_PATH
)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${NAPI_PATH})

find_package(PkgConfig REQUIRED)

pkg_check_modules(X11 REQUIRED x11)
pkg_check_modules(XFIXES REQUIRED xfixes)

add_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -Wextra -Wpeantic -fPIC)
add_compile_options(${PROJECT_NAME} PRIVATE -ggdb3 -Og)
add_link_options(${PROJECT_NAME} PRIVATE -ggdb3)

target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${X11_CFLAGS_OTHER})
target_include_directories(${PROJECT_NAME} PRIVATE ${XFIXES_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${XFIXES_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE ${XFIXES_CFLAGS_OTHER})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_26)

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()