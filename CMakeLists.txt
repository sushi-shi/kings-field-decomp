cmake_minimum_required(VERSION 3.25)
project(kings_field VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions> $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
option(KF_SANITIZE "Enable address and undefined behavior sanitizers" OFF)
if(KF_SANITIZE AND NOT EMSCRIPTEN)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

find_program(CARGO_EXECUTABLE cargo REQUIRED)
set(KF_RUST_DIR "${CMAKE_BINARY_DIR}/rust")
set(KF_RUST_ARGS --offline --locked --release --manifest-path ${CMAKE_SOURCE_DIR}/codec-bridge/Cargo.toml)
if(EMSCRIPTEN)
  list(APPEND KF_RUST_ARGS --target wasm32-unknown-emscripten -Zbuild-std=core,compiler_builtins)
  # Cargo 1.95's pinned source-root override avoids mutating the Nix sysroot.
  set(KF_RUST_ENV "__CARGO_TESTS_ONLY_SRC_ROOT=$ENV{KF_RUST_SOURCE}")
  set(KF_CODEC_LIBRARY "${KF_RUST_DIR}/wasm32-unknown-emscripten/release/libkf_codec_bridge.a")
else()
  set(KF_CODEC_LIBRARY "${KF_RUST_DIR}/release/libkf_codec_bridge.a")
endif()
file(GLOB_RECURSE KF_CODEC_SOURCES CONFIGURE_DEPENDS codecs/src/*.rs codec-bridge/src/*.rs)
add_custom_command(OUTPUT "${KF_CODEC_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${KF_RUST_DIR} RUSTC_BOOTSTRAP=1
    RUSTFLAGS=-Cpanic=abort ${KF_RUST_ENV} ${CARGO_EXECUTABLE} build ${KF_RUST_ARGS}
  DEPENDS ${KF_CODEC_SOURCES} codecs/Cargo.toml codec-bridge/Cargo.toml codec-bridge/Cargo.lock
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} VERBATIM)
add_custom_target(kf_codecs_build DEPENDS "${KF_CODEC_LIBRARY}")
add_library(kf_codecs STATIC IMPORTED GLOBAL)
set_target_properties(kf_codecs PROPERTIES IMPORTED_LOCATION "${KF_CODEC_LIBRARY}")
add_dependencies(kf_codecs kf_codecs_build)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
# The Nix dev shell and package supply a delta derived from the translator's PPF
# release; otherwise a local resources/english-v1.kfdelta is used when present.
if(DEFINED ENV{KF_ENGLISH_PATCH} AND NOT "$ENV{KF_ENGLISH_PATCH}" STREQUAL "")
  set(KF_ENGLISH_PATCH_DEFAULT "$ENV{KF_ENGLISH_PATCH}")
else()
  set(KF_ENGLISH_PATCH_DEFAULT "${CMAKE_SOURCE_DIR}/resources/english-v1.kfdelta")
endif()
set(KF_ENGLISH_PATCH "${KF_ENGLISH_PATCH_DEFAULT}" CACHE FILEPATH
    "English translation delta to embed (optional; never distribute builds containing it)")
# Checked on every build, so a payload added after configuring is picked up;
# the generated file is rewritten only when its content changes.
add_custom_target(kf_english_patch
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/english_patch.py" embed
    --input "${KF_ENGLISH_PATCH}" --optional --output "${CMAKE_BINARY_DIR}/generated/english_patch.inc"
  BYPRODUCTS "${CMAKE_BINARY_DIR}/generated/english_patch.inc" VERBATIM)
include(cmake/OriginalSources.cmake)
add_library(kf_support STATIC src/platform/files.cpp src/platform/disc.cpp src/platform/assets.cpp src/platform/language.cpp src/platform/translation.cpp src/platform/controls.cpp src/lib/fixed_math.cpp src/lib/random.cpp)
target_sources(kf_support PRIVATE "${CMAKE_BINARY_DIR}/generated/english_patch.inc")
target_include_directories(kf_support PUBLIC include PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(kf_support PUBLIC kf_codecs)
add_dependencies(kf_support kf_english_patch)
target_compile_options(kf_support PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(EMSCRIPTEN)
  if(NOT DEFINED ENV{KF_SDL_SOURCE})
    message(FATAL_ERROR "Build in nix develop: KF_SDL_SOURCE must identify the pinned SDL3 source")
  endif()
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  add_subdirectory("$ENV{KF_SDL_SOURCE}" "${CMAKE_BINARY_DIR}/sdl3" EXCLUDE_FROM_ALL)
else()
  find_package(SDL3 CONFIG REQUIRED)
  find_library(KF_GLES_LIBRARY GLESv2 REQUIRED)
endif()
add_library(kf_host STATIC src/platform/host.cpp src/platform/saves.cpp src/platform/language_runtime.cpp src/audio/sound.cpp src/renderer/renderer.cpp src/renderer/textures.cpp src/renderer/ui_text.cpp src/renderer/lighting.cpp src/renderer/projection.cpp)
target_include_directories(kf_host PUBLIC include)
target_link_libraries(kf_host PUBLIC kf_support SDL3::SDL3)
target_compile_options(kf_host PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_executable(kings-field src/platform/entry.cpp
  $<TARGET_OBJECTS:kf_game> $<TARGET_OBJECTS:kf_opening> $<TARGET_OBJECTS:kf_common>)
target_link_libraries(kings-field PRIVATE kf_host)
target_compile_options(kings-field PRIVATE -Wall -Wextra)
if(EMSCRIPTEN)
  target_link_options(kings-field PRIVATE
    -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=262144
    -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
    -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=536870912
    -sENVIRONMENT=web -sEXIT_RUNTIME=1 -sFORCE_FILESYSTEM=1
    "-sEXPORTED_RUNTIME_METHODS=['ccall','FS','callMain']"
    "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html")
  set_target_properties(kings-field PROPERTIES SUFFIX .html)
  set_property(TARGET kings-field APPEND PROPERTY LINK_DEPENDS "${CMAKE_SOURCE_DIR}/web/shell.html")
else()
  target_link_libraries(kings-field PRIVATE ${KF_GLES_LIBRARY})
endif()
install(TARGETS kings-field RUNTIME DESTINATION bin)
