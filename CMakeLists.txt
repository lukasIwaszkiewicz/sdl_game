cmake_minimum_required(VERSION 3.14)
project(sdl_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Options ──────────────────────────────────────────────────────────────────
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# ── FetchContent ─────────────────────────────────────────────────────────────
include(FetchContent)

# ── SDL3 ─────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

# ── SDL_image (for RmlUi texture loading) ────────────────────────────────────
FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG        release-3.2.6
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_VENDORED ON CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIFF OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)

# ── FreeType (required by RmlUi's default font engine) ───────────────────────
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG        VER-2-13-3
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)

# ── RmlUi ────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.2
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_BACKEND "SDL_GL3" CACHE STRING "" FORCE)

# Make SDL3, SDL_image, and FreeType available first
FetchContent_MakeAvailable(SDL3 SDL3_image freetype)

# RmlUi expects FreeType via find_package(Freetype) => target "Freetype::Freetype".
# FetchContent builds it as "freetype". Create an alias so RmlUi finds it.
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# RmlUi backends reference SDL targets as "SDL::SDL" and "SDL_image::SDL_image",
# but SDL3 exports "SDL3::SDL3" (alias of SDL3-static/SDL3-shared).
# CMake can't alias an alias, so we alias the real underlying targets.
if(NOT TARGET SDL::SDL)
    if(TARGET SDL3-static)
        add_library(SDL::SDL ALIAS SDL3-static)
    elseif(TARGET SDL3-shared)
        add_library(SDL::SDL ALIAS SDL3-shared)
    endif()
endif()
if(NOT TARGET SDL_image::SDL_image)
    if(TARGET SDL3_image-static)
        add_library(SDL_image::SDL_image ALIAS SDL3_image-static)
    elseif(TARGET SDL3_image-shared)
        add_library(SDL_image::SDL_image ALIAS SDL3_image-shared)
    endif()
endif()

# Now make RmlUi available (it will find Freetype::Freetype, SDL::SDL, SDL_image::SDL_image)
FetchContent_MakeAvailable(RmlUi)

# ── Main executable ─────────────────────────────────────────────────────────
# Include the RmlUi backend sources directly in our target.
# The rmlui_backend_SDL_GL3 is an INTERFACE library that adds sources via
# INTERFACE target_sources - we include them explicitly for MSVC compatibility.
set(RMLUI_BACKEND_DIR "${rmlui_SOURCE_DIR}/Backends")
add_executable(${PROJECT_NAME}
    src/main.cpp
    "${RMLUI_BACKEND_DIR}/RmlUi_Platform_SDL.cpp"
    "${RMLUI_BACKEND_DIR}/RmlUi_Renderer_GL3.cpp"
    "${RMLUI_BACKEND_DIR}/RmlUi_Backend_SDL_GL3.cpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    rmlui
    rmlui_debugger
    SDL3::SDL3
    SDL3_image::SDL3_image
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${RMLUI_BACKEND_DIR}
)

# RmlUi backend sources need to know which SDL major version is in use
target_compile_definitions(${PROJECT_NAME} PRIVATE RMLUI_SDL_VERSION_MAJOR=3)

# ── Copy assets next to the executable ───────────────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to output directory"
)
