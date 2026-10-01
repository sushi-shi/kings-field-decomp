cmake_minimum_required(VERSION 3.25)
project(kings_field VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
if(EMSCRIPTEN)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fexceptions>)
  add_link_options(-fexceptions)
endif()
option(KF_SANITIZE "Enable address and undefined behavior sanitizers" OFF)
if(KF_SANITIZE AND NOT EMSCRIPTEN)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
# The Nix dev shell and package supply a delta derived from the translator's PPF
# release; otherwise a local english-v1.kfdelta in the build directory is used.
if(DEFINED ENV{KF_ENGLISH_PATCH} AND NOT "$ENV{KF_ENGLISH_PATCH}" STREQUAL "")
  set(KF_ENGLISH_PATCH_DEFAULT "$ENV{KF_ENGLISH_PATCH}")
else()
  set(KF_ENGLISH_PATCH_DEFAULT "${CMAKE_BINARY_DIR}/english-v1.kfdelta")
endif()
set(KF_ENGLISH_PATCH "${KF_ENGLISH_PATCH_DEFAULT}" CACHE FILEPATH
    "English translation delta to embed (optional; never distribute builds containing it)")
# Checked on every build, so a payload added after configuring is picked up;
# the generated file is rewritten only when its content changes.
add_custom_target(kf_english_patch
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/english_patch.py" embed
    --input "${KF_ENGLISH_PATCH}" --optional --output "${CMAKE_BINARY_DIR}/generated/english_patch.inc"
  BYPRODUCTS "${CMAKE_BINARY_DIR}/generated/english_patch.inc" VERBATIM)
set(KF_GAME_SOURCES
  src/game/avatar_pose.cpp
  src/game/campaign.cpp
  src/game/coop_session.cpp
  src/game/party.cpp
  src/game/party_runtime.cpp
  src/game/player_actions.cpp
  src/game/player_avatar_render.cpp
  src/game/snapshot.cpp
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
add_library(kf_support STATIC
  src/audio/codec.cpp
  src/lib/fixed_math.cpp
  src/lib/random.cpp
  src/lib/resource_decode.cpp
  src/platform/avatar_pack.cpp
  src/platform/avatar_source.cpp
  src/platform/avatar_disc.cpp
  src/platform/assets.cpp
  src/platform/controls.cpp
  src/platform/disc.cpp
  src/platform/files.cpp
  src/platform/language.cpp
  src/platform/translation.cpp
  src/renderer/tim.cpp)
target_sources(kf_support PRIVATE "${CMAKE_BINARY_DIR}/generated/english_patch.inc")
target_include_directories(kf_support PUBLIC include PRIVATE "${CMAKE_BINARY_DIR}/generated")
add_dependencies(kf_support kf_english_patch)
target_compile_options(kf_support PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(EMSCRIPTEN)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
  if(DEFINED ENV{KF_SDL_SOURCE} AND NOT "$ENV{KF_SDL_SOURCE}" STREQUAL "")
    add_subdirectory("$ENV{KF_SDL_SOURCE}" "${CMAKE_BINARY_DIR}/sdl3" EXCLUDE_FROM_ALL)
  else()
    include(FetchContent)
    FetchContent_Declare(SDL3
      URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.8/SDL3-3.4.8.tar.gz
      URL_HASH SHA256=e9fff7467fb60f037e6708da18b25560649e4c63edc2a69bb871b960d9cbfbba
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(SDL3)
  endif()
else()
  find_package(SDL3 CONFIG REQUIRED)
  find_library(KF_GLES_LIBRARY GLESv2 REQUIRED)
endif()
# Header-only JSON is shared by the native and browser signaling parsers.
find_package(nlohmann_json 3.12 CONFIG QUIET NO_CMAKE_FIND_ROOT_PATH)
if(NOT nlohmann_json_FOUND)
  include(FetchContent)
  FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(nlohmann_json)
endif()
add_library(kf_network OBJECT src/net/codec.cpp src/net/world.cpp src/net/protocol.cpp
  src/net/transport.cpp src/net/transport_session.cpp)
target_include_directories(kf_network PUBLIC include)
target_link_libraries(kf_network PRIVATE nlohmann_json::nlohmann_json)
target_compile_options(kf_network PRIVATE -Wall -Wextra -Werror)
if(EMSCRIPTEN)
  target_sources(kf_network PRIVATE src/net/transport_web.cpp src/net/transport_browser.cpp)
else()
  find_package(LibDataChannel CONFIG REQUIRED)
  target_sources(kf_network PRIVATE src/net/transport_native.cpp)
  target_link_libraries(kf_network PRIVATE LibDataChannel::LibDataChannel)
endif()

add_library(kf_host STATIC src/platform/avatars.cpp src/platform/campaign.cpp src/platform/host.cpp src/platform/saves.cpp src/platform/language_runtime.cpp src/audio/sound.cpp src/renderer/renderer.cpp src/renderer/textures.cpp src/renderer/lighting.cpp src/renderer/projection.cpp)
target_include_directories(kf_host PUBLIC include)
target_link_libraries(kf_host PUBLIC kf_support SDL3::SDL3)
target_compile_options(kf_host PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_library(kf_game OBJECT ${KF_GAME_SOURCES})
target_include_directories(kf_game PRIVATE include)
add_executable(kings-field src/platform/entry.cpp $<TARGET_OBJECTS:kf_game>)
target_link_libraries(kings-field PRIVATE kf_host kf_network)
target_compile_options(kings-field PRIVATE -Wall -Wextra)
if(EMSCRIPTEN)
  target_link_options(kings-field PRIVATE
    # Coroutine continuations may nest on the normal WASM stack before yielding.
    # Asyncify's stack stores suspended platform calls and is a separate budget.
    -sSTACK_SIZE=1048576 -sSTACK_OVERFLOW_CHECK=2
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

include(CTest)
if(BUILD_TESTING AND EMSCRIPTEN)
  add_executable(coop-browser-test tests/coop_transport_browser.cpp)
  target_link_libraries(coop-browser-test PRIVATE kf_network)
  target_link_options(coop-browser-test PRIVATE -sASYNCIFY=1 -sASYNCIFY_STACK_SIZE=262144 -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web -sEXIT_RUNTIME=1)
  set_target_properties(coop-browser-test PROPERTIES SUFFIX .html)
endif()
if(BUILD_TESTING AND NOT EMSCRIPTEN)
  add_executable(coop-protocol-test tests/coop_protocol.cpp)
  target_link_libraries(coop-protocol-test PRIVATE kf_network)
  target_compile_options(coop-protocol-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME coop-protocol COMMAND coop-protocol-test)
  add_executable(coop-world-codec-test tests/coop_world_codec.cpp)
  target_link_libraries(coop-world-codec-test PRIVATE kf_network)
  add_test(NAME coop-world-codec COMMAND coop-world-codec-test)
  add_executable(coop-avatar-codec-test tests/coop_avatar_codec.cpp)
  target_link_libraries(coop-avatar-codec-test PRIVATE kf_support)
  target_compile_options(coop-avatar-codec-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME coop-avatar-codec COMMAND coop-avatar-codec-test)
  add_executable(coop-transport-inbox-test tests/coop_transport_inbox.cpp src/net/transport_session.cpp)
  target_include_directories(coop-transport-inbox-test PRIVATE include)
  target_link_libraries(coop-transport-inbox-test PRIVATE nlohmann_json::nlohmann_json)
  target_compile_options(coop-transport-inbox-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME coop-transport-inbox COMMAND coop-transport-inbox-test)
  add_executable(coop-crossplay-host tests/coop_crossplay_host.cpp)
  target_link_libraries(coop-crossplay-host PRIVATE kf_network)
  add_executable(coop-transport-test tests/coop_transport.cpp)
  target_link_libraries(coop-transport-test PRIVATE kf_network)
  target_compile_options(coop-transport-test PRIVATE -Wall -Wextra -Werror)
  add_executable(coop-runtime-test tests/coop_runtime.cpp
    $<TARGET_OBJECTS:kf_game>)
  target_link_libraries(coop-runtime-test PRIVATE kf_host kf_network ${KF_GLES_LIBRARY})
  target_compile_options(coop-runtime-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME coop-runtime COMMAND coop-runtime-test)
  add_executable(coop-movement-test tests/coop_movement.cpp
    $<TARGET_OBJECTS:kf_game>)
  target_link_libraries(coop-movement-test PRIVATE kf_host kf_network ${KF_GLES_LIBRARY})
  target_compile_options(coop-movement-test PRIVATE -Wall -Wextra -Werror)
  add_test(NAME coop-movement COMMAND coop-movement-test)
endif()
