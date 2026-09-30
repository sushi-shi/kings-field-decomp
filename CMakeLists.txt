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
find_program(BINDGEN_EXECUTABLE bindgen REQUIRED)
find_program(BASH_EXECUTABLE bash REQUIRED)
set(KF_RUST_DIR "${CMAKE_BINARY_DIR}/rust")
set(KF_RUST_ARGS --offline --locked --release --manifest-path ${CMAKE_SOURCE_DIR}/codecs/Cargo.toml)
if(EMSCRIPTEN)
  list(APPEND KF_RUST_ARGS --target wasm32-unknown-emscripten -Zbuild-std=core,compiler_builtins)
  # Cargo 1.95's pinned source-root override avoids mutating the Nix sysroot.
  set(KF_RUST_ENV "__CARGO_TESTS_ONLY_SRC_ROOT=$ENV{KF_RUST_SOURCE}")
  set(KF_CODEC_LIBRARY "${KF_RUST_DIR}/wasm32-unknown-emscripten/release/libkf_codec.a")
else()
  set(KF_CODEC_LIBRARY "${KF_RUST_DIR}/release/libkf_codec.a")
endif()
file(GLOB_RECURSE KF_CODEC_SOURCES CONFIGURE_DEPENDS codecs/src/*.rs)
add_custom_command(OUTPUT "${KF_CODEC_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E env BINDGEN=${BINDGEN_EXECUTABLE}
    ${BASH_EXECUTABLE} codecs/bindings.sh
  COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${KF_RUST_DIR} RUSTC_BOOTSTRAP=1
    RUSTFLAGS=-Cpanic=abort ${KF_RUST_ENV} ${CARGO_EXECUTABLE} build ${KF_RUST_ARGS}
  COMMAND ${CMAKE_COMMAND} -E touch "${KF_CODEC_LIBRARY}"
  DEPENDS ${KF_CODEC_SOURCES} codecs/Cargo.toml codecs/Cargo.lock
    codecs/bindings.sh include/kf/lib/codec.h include/kf/audio/codec.h
    ${BINDGEN_EXECUTABLE}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} VERBATIM)
add_custom_target(kf_codecs_build DEPENDS "${KF_CODEC_LIBRARY}")
add_library(kf_codecs STATIC IMPORTED GLOBAL)
set_target_properties(kf_codecs PROPERTIES IMPORTED_LOCATION "${KF_CODEC_LIBRARY}")
add_dependencies(kf_codecs kf_codecs_build)

set(KF_GAME_SOURCES
  src/game/display_play_transition.cpp
  src/game/player_warp_shimmer_at_player.cpp
  src/game/game.cpp
  src/game/equipment.cpp
  src/game/player_death.cpp
  src/game/player_core.cpp
  src/game/player_use_item.cpp
  src/game/player_death_fade.cpp
  src/game/lighting_presets.cpp
  src/game/player_fire_defense.cpp
  src/game/player_update.cpp
  src/game/collision_grid.cpp
  src/game/collision.cpp
  src/game/resource_file.cpp
  src/game/resources.cpp
  src/game/render.cpp
  src/game/render_enqueuers.cpp
  src/game/render_sprite.cpp
  src/game/render_map_cells.cpp
  src/game/entity_model_render.cpp
  src/game/entity_render.cpp
  src/game/map_event_render.cpp
  src/game/render_scene.cpp
  src/game/geometry_render.cpp
  src/game/render_frame.cpp
  src/game/asset_registry.cpp
  src/game/animation_cache.cpp
  src/game/item.cpp
  src/game/menu.cpp
  src/game/menu_list.cpp
  src/game/menu_map_viewer.cpp
  src/game/menu_panels.cpp
  src/game/menu_select.cpp
  src/game/menu_runtime.cpp
  src/game/save_system.cpp
  src/game/actor.cpp
  src/game/actor_behavior.cpp
  src/game/actor_pool.cpp
  src/game/map_object_pool.cpp
  src/game/map_object.cpp
  src/game/audio.cpp
  src/game/camera_path.cpp
  src/game/map_event.cpp
  src/game/lighting.cpp
  src/game/map_scripts.cpp
  src/game/map_events.cpp
  src/game/map_load.cpp
  src/game/player_warp.cpp
  src/game/menu_enter_mode.cpp
  src/game/effect_pool.cpp
  src/game/effect_map_collision.cpp
  src/game/effect_update.cpp
  src/game/effect_dispatch.cpp
  src/game/magic.cpp
  src/game/debug_text.cpp
  src/game/session.cpp
  src/cutscene/opening_render.cpp
  src/cutscene/opening_helpers.cpp
  src/cutscene/camera_path.cpp
  src/cutscene/opening_scenes.cpp
  src/cutscene/opening_fade.cpp
  src/cutscene/opening_controller.cpp
  src/cutscene/resources.cpp
  src/cutscene/render_init.cpp
  src/cutscene/render.cpp
  src/cutscene/render_tmd.cpp
  src/cutscene/render_unlit.cpp
  src/cutscene/render_map.cpp
  src/cutscene/render_sprite.cpp
  src/cutscene/render_map_cells.cpp
  src/cutscene/entity_render.cpp
  src/cutscene/opening_scene0_render.cpp
  src/cutscene/opening_render_entities.cpp
  src/cutscene/opening_entity_pool.cpp
  src/cutscene/audio.cpp
  src/cutscene/audio_spatial.cpp
  src/cutscene/playback.cpp
  src/lib/sprite_add.cpp
  src/lib/matrix_rotation.cpp
  src/lib/vector_math.cpp
  src/lib/camera_path.cpp
  src/lib/memory.cpp
  src/lib/matrix_lerp.cpp
  src/lib/audio.cpp
  src/lib/display.cpp
  src/lib/tmd.cpp
  src/lib/floor_item.cpp
  src/lib/format.cpp
  src/lib/resource_file.cpp
  src/lib/resources.cpp
  src/lib/debug_matrix.cpp
  src/lib/render_view.cpp
  src/lib/render_face.cpp
)
set_source_files_properties(${KF_GAME_SOURCES} PROPERTIES
  COMPILE_OPTIONS "-Wall;-Wextra;-fwrapv;-fno-strict-aliasing")
add_library(kf_support STATIC src/platform/files.cpp src/platform/disc.cpp src/platform/assets.cpp src/platform/controls.cpp src/lib/fixed_math.cpp src/lib/random.cpp)
target_include_directories(kf_support PUBLIC include)
target_link_libraries(kf_support PUBLIC kf_codecs)
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
add_library(kf_host STATIC src/platform/host.cpp src/platform/saves.cpp src/audio/sound.cpp src/renderer/renderer.cpp src/renderer/textures.cpp src/renderer/lighting.cpp src/renderer/projection.cpp)
target_include_directories(kf_host PUBLIC include)
target_link_libraries(kf_host PUBLIC kf_support SDL3::SDL3)
target_compile_options(kf_host PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_executable(kings-field src/platform/entry.cpp
  ${KF_GAME_SOURCES})
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
