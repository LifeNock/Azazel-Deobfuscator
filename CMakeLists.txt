cmake_minimum_required(VERSION 3.20)
project(AzazelDeobfuscator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(stb)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/azazel.png
    ${CMAKE_CURRENT_BINARY_DIR}/azazel.png
    COPYONLY
)

add_executable(AzazelDeobfuscator WIN32
    src/main.cpp
    src/deobfuscator.cpp
    src/sandbox.cpp
    resources/app.rc
    ${IMGUI_SOURCES}
)

target_include_directories(AzazelDeobfuscator PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
    src
)

target_link_libraries(AzazelDeobfuscator PRIVATE
    d3d11
    d3dcompiler
    dxgi
    dwmapi
    comctl32
    comdlg32
    shlwapi
)

target_compile_definitions(AzazelDeobfuscator PRIVATE
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
)

if(MSVC)
    target_compile_options(AzazelDeobfuscator PRIVATE /W3 /utf-8)
    set_target_properties(AzazelDeobfuscator PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup"
    )
endif()
