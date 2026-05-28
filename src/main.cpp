// anyone can fork this as long as there is credit, the ai will not work on other forks.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <commdlg.h>
#include <shlwapi.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "azazel_png.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "deobfuscator.h"
#include "sandbox.h"

#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <sstream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shlwapi.lib")

static ID3D11Device*            g_pd3dDevice          = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext   = nullptr;
static IDXGISwapChain*          g_pSwapChain          = nullptr;
static ID3D11RenderTargetView*  g_mainRTV             = nullptr;
static HWND                     g_hwnd                = nullptr;

static ID3D11ShaderResourceView* g_logoSRV    = nullptr;
static int                       g_logoW      = 0;
static int                       g_logoH      = 0;

static bool createDeviceD3D(HWND hWnd);
static void cleanupDeviceD3D();
static void createRenderTarget();
static void cleanupRenderTarget();
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
            cleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            createRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static bool loadLogoTexture() {
    int channels = 0;
    unsigned char* data = stbi_load_from_memory(
        AZAZEL_PNG_DATA, AZAZEL_PNG_SIZE, &g_logoW, &g_logoH, &channels, 4);
    if (!data) return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = g_logoW;
    desc.Height           = g_logoH;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem     = data;
    sub.SysMemPitch = g_logoW * 4;

    ID3D11Texture2D* tex = nullptr;
    g_pd3dDevice->CreateTexture2D(&desc, &sub, &tex);
    stbi_image_free(data);
    if (!tex) return false;

    g_pd3dDevice->CreateShaderResourceView(tex, nullptr, &g_logoSRV);
    tex->Release();
    return g_logoSRV != nullptr;
}

static void applyTheme() {
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowPadding     = {16, 16};
    s.FramePadding      = {10, 7};
    s.ItemSpacing       = {8, 8};
    s.ItemInnerSpacing  = {6, 4};
    s.IndentSpacing     = 20;
    s.ScrollbarSize     = 10;
    s.GrabMinSize       = 8;

    s.WindowRounding    = 6;
    s.ChildRounding     = 5;
    s.FrameRounding     = 4;
    s.PopupRounding     = 5;
    s.ScrollbarRounding = 4;
    s.GrabRounding      = 3;
    s.TabRounding       = 4;

    s.WindowBorderSize  = 1;
    s.FrameBorderSize   = 1;
    s.PopupBorderSize   = 1;

    auto& c = s.Colors;
    auto rgb = [](int r, int g, int b, float a = 1.0f) -> ImVec4 {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a};
    };

    c[ImGuiCol_WindowBg]              = rgb(10, 7, 7);
    c[ImGuiCol_ChildBg]               = rgb(15, 10, 10);
    c[ImGuiCol_PopupBg]               = rgb(18, 12, 12, 0.98f);

    c[ImGuiCol_Border]                = rgb(80, 22, 22, 0.7f);
    c[ImGuiCol_BorderShadow]          = rgb(0, 0, 0, 0.0f);

    c[ImGuiCol_FrameBg]               = rgb(22, 14, 14);
    c[ImGuiCol_FrameBgHovered]        = rgb(35, 20, 20);
    c[ImGuiCol_FrameBgActive]         = rgb(44, 24, 24);

    c[ImGuiCol_TitleBg]               = rgb(8, 5, 5);
    c[ImGuiCol_TitleBgActive]         = rgb(14, 8, 8);
    c[ImGuiCol_TitleBgCollapsed]      = rgb(8, 5, 5, 0.9f);

    c[ImGuiCol_ScrollbarBg]           = rgb(12, 8, 8, 0.6f);
    c[ImGuiCol_ScrollbarGrab]         = rgb(140, 20, 20);
    c[ImGuiCol_ScrollbarGrabHovered]  = rgb(180, 30, 30);
    c[ImGuiCol_ScrollbarGrabActive]   = rgb(210, 40, 40);

    c[ImGuiCol_CheckMark]             = rgb(230, 50, 50);
    c[ImGuiCol_SliderGrab]            = rgb(180, 30, 30);
    c[ImGuiCol_SliderGrabActive]      = rgb(220, 50, 50);

    c[ImGuiCol_Button]                = rgb(130, 16, 16);
    c[ImGuiCol_ButtonHovered]         = rgb(175, 24, 24);
    c[ImGuiCol_ButtonActive]          = rgb(210, 35, 35);

    c[ImGuiCol_Header]                = rgb(100, 16, 16, 0.7f);
    c[ImGuiCol_HeaderHovered]         = rgb(140, 22, 22, 0.85f);
    c[ImGuiCol_HeaderActive]          = rgb(170, 28, 28);

    c[ImGuiCol_Separator]             = rgb(60, 18, 18, 0.8f);
    c[ImGuiCol_SeparatorHovered]      = rgb(130, 25, 25);
    c[ImGuiCol_SeparatorActive]       = rgb(180, 35, 35);

    c[ImGuiCol_ResizeGrip]            = rgb(120, 18, 18, 0.4f);
    c[ImGuiCol_ResizeGripHovered]     = rgb(160, 26, 26, 0.7f);
    c[ImGuiCol_ResizeGripActive]      = rgb(200, 36, 36, 0.9f);

    c[ImGuiCol_Tab]                   = rgb(22, 13, 13);
    c[ImGuiCol_TabHovered]            = rgb(140, 22, 22);
    c[ImGuiCol_TabActive]             = rgb(100, 16, 16);
    c[ImGuiCol_TabUnfocused]          = rgb(14, 9, 9);
    c[ImGuiCol_TabUnfocusedActive]    = rgb(40, 16, 16);

    c[ImGuiCol_Text]                  = rgb(240, 215, 215);
    c[ImGuiCol_TextDisabled]          = rgb(110, 70, 70);
    c[ImGuiCol_TextSelectedBg]        = rgb(160, 22, 22, 0.5f);

    c[ImGuiCol_DragDropTarget]        = rgb(230, 50, 50, 0.9f);
    c[ImGuiCol_NavHighlight]          = rgb(200, 40, 40);
    c[ImGuiCol_NavWindowingHighlight] = rgb(1.0f, 1.0f, 1.0f, 0.7f);
    c[ImGuiCol_NavWindowingDimBg]     = rgb(0.8f * 255, 0.8f * 255, 0.8f * 255, 0.2f);
    c[ImGuiCol_ModalWindowDimBg]      = rgb(5, 3, 3, 0.65f);

    c[ImGuiCol_PlotLines]             = rgb(220, 60, 60);
    c[ImGuiCol_PlotHistogram]         = rgb(190, 40, 40);
}

static std::wstring toWide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    return w;
}

static std::string fromWide(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0') s.pop_back();
    return s;
}

static std::string browseOpenFile(HWND owner) {
    wchar_t path[MAX_PATH] = {};
    OPENFILENAMEW ofn      = {};
    ofn.lStructSize        = sizeof(ofn);
    ofn.hwndOwner          = owner;
    ofn.lpstrFile          = path;
    ofn.nMaxFile           = MAX_PATH;
    ofn.lpstrFilter        = L"Lua / Luau Files\0*.lua;*.luau\0All Files\0*.*\0";
    ofn.Flags              = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) return fromWide(path);
    return {};
}

static std::string browseSaveFile(HWND owner) {
    wchar_t path[MAX_PATH] = {};
    OPENFILENAMEW ofn      = {};
    ofn.lStructSize        = sizeof(ofn);
    ofn.hwndOwner          = owner;
    ofn.lpstrFile          = path;
    ofn.nMaxFile           = MAX_PATH;
    ofn.lpstrFilter        = L"Lua / Luau Files\0*.lua;*.luau\0All Files\0*.*\0";
    ofn.lpstrDefExt        = L"luau";
    ofn.Flags              = OFN_OVERWRITEPROMPT;
    if (GetSaveFileNameW(&ofn)) return fromWide(path);
    return {};
}

static std::string readFile(const std::string& path) {
    std::ifstream f(toWide(path), std::ios::binary);
    if (!f) return {};
    return {std::istreambuf_iterator<char>(f), {}};
}

static bool writeFile(const std::string& path, const std::string& data) {
    std::ofstream f(toWide(path), std::ios::binary);
    if (!f) return false;
    f.write(data.data(), (std::streamsize)data.size());
    return (bool)f;
}


static std::string formatBytes(int n) {
    if (n < 1024)        return std::to_string(n) + " B";
    if (n < 1024 * 1024) return std::to_string(n / 1024) + " KB";
    return std::to_string(n / (1024 * 1024)) + " MB";
}

static void labelText(const char* label, const ImVec4& col = {0.55f, 0.32f, 0.32f, 1.0f}) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
}

static void sectionHeader(const char* text) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.72f, 0.18f, 0.18f, 1.0f));
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.55f, 0.12f, 0.12f, 0.9f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

struct AppState {
    char inputPath[MAX_PATH]  = {};
    char outputPath[MAX_PATH] = {};

    DeobfuscateOptions opts;

    std::atomic<bool>  running{false};
    std::atomic<bool>  flushed{false};
    std::mutex         resultMtx;
    DeobfuscateResult  result;
    bool               hasResult = false;
};

static void runDeobfuscate(AppState& app) {
    app.flushed = false;
    app.running = true;

    DeobfuscateResult res;
    try {
        std::string source = readFile(app.inputPath);
        if (source.empty()) {
            res.success      = false;
            res.errorMessage = "Could not read input file.";
            res.log.push_back("Could not read input file.");
        } else {
            DeobfuscateOptions opts = app.opts;
            opts.useSandbox = true;
            res = deobfuscate(source, opts);
        }
    } catch (const std::exception& e) {
        res.success      = false;
        res.errorMessage = e.what();
        res.log.push_back(std::string("Error: ") + e.what());
    } catch (...) {
        res.success      = false;
        res.errorMessage = "Unknown error.";
        res.log.push_back("Error: unknown exception.");
    }

    {
        std::lock_guard<std::mutex> lk(app.resultMtx);
        app.result    = std::move(res);
        app.hasResult = true;
    }
    app.running = false;
    app.flushed = true;
}

static void renderUI(AppState& app) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    ImGuiWindowFlags wf =
        ImGuiWindowFlags_NoDecoration    |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,  0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##root", nullptr, wf);
    ImGui::PopStyleVar(2);

    const float totalW = ImGui::GetContentRegionAvail().x;
    const float totalH = ImGui::GetContentRegionAvail().y;

    const float logoDisplayH = 96.0f;
    const float logoDisplayW = g_logoH > 0
        ? logoDisplayH * ((float)g_logoW / (float)g_logoH)
        : logoDisplayH;
    const float bannerH = logoDisplayH + 20.0f;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.04f, 0.04f, 1.0f));
    ImGui::BeginChild("##banner", {totalW, bannerH}, false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    float textBlockW = totalW - logoDisplayW - 36.0f;

    ImGui::SetCursorPos({14, 16});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.20f, 0.20f, 1.0f));
    ImGui::SetWindowFontScale(1.45f);
    ImGui::TextUnformatted("AZAZEL DEOBFUSCATOR");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({16, 46});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.28f, 0.28f, 1.0f));
    ImGui::PushTextWrapPos(16.0f + textBlockW);
    ImGui::TextUnformatted("luau deobfuscator because people love obfuscating copy and paste luau scripts lmao");
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();

    ImVec2 bannerScreenPos  = ImGui::GetWindowPos();
    ImVec2 bannerScreenSize = ImGui::GetWindowSize();

    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (g_logoSRV) {
        float imgX = bannerScreenPos.x + bannerScreenSize.x - logoDisplayW - 14.0f;
        float imgY = bannerScreenPos.y + (bannerH - logoDisplayH) * 0.5f;
        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)g_logoSRV,
            ImVec2(imgX,                imgY),
            ImVec2(imgX + logoDisplayW, imgY + logoDisplayH)
        );
    }

    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.55f, 0.10f, 0.10f, 1.0f));
    ImGui::Separator();
    ImGui::PopStyleColor();

    const float browseW  = 90.0f;
    const float padH     = ImGui::GetStyle().ItemSpacing.x;

    float remainH = totalH
        - bannerH
        - ImGui::GetStyle().ItemSpacing.y * 2
        - 2.0f;

    ImGui::BeginChild("##scroll", {totalW, remainH}, false);

    const float contentW = ImGui::GetContentRegionAvail().x;

    sectionHeader("INPUT FILE");

    ImGui::SetNextItemWidth(contentW - browseW - padH);
    ImGui::InputText("##inpath", app.inputPath, MAX_PATH,
                     ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Browse##in", {browseW, 0})) {
        std::string p = browseOpenFile(g_hwnd);
        if (!p.empty()) {
            strncpy_s(app.inputPath, p.c_str(), MAX_PATH - 1);
            if (app.outputPath[0] == '\0') {
                size_t dot = p.rfind('.');
                std::string base = (dot != std::string::npos) ? p.substr(0, dot) : p;
                std::string out = base + "_deobf.luau";
                strncpy_s(app.outputPath, out.c_str(), MAX_PATH - 1);
            }
        }
    }

    ImGui::Spacing();
    sectionHeader("OUTPUT FILE");

    ImGui::SetNextItemWidth(contentW - browseW - padH);
    ImGui::InputText("##outpath", app.outputPath, MAX_PATH);
    ImGui::SameLine();
    if (ImGui::Button("Browse##out", {browseW, 0})) {
        std::string p = browseSaveFile(g_hwnd);
        if (!p.empty())
            strncpy_s(app.outputPath, p.c_str(), MAX_PATH - 1);
    }

    ImGui::Spacing();
    sectionHeader("OPTIONS");

    ImGui::Columns(2, "opts", false);
    ImGui::Checkbox("Decode string constants",  &app.opts.decodeStrings);
    ImGui::Checkbox("Join concatenations",       &app.opts.joinConcatenations);
    ImGui::NextColumn();
    ImGui::Checkbox("Fold arithmetic constants", &app.opts.foldArithmetic);
    ImGui::Checkbox("Resolve W() calls",         &app.opts.resolveWCalls);
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Spacing();

    bool canRun = app.inputPath[0] != '\0'
               && app.outputPath[0] != '\0'
               && !app.running;

    if (!canRun) ImGui::BeginDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.62f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.78f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.90f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f,  0.92f, 0.92f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 11));

    bool clicked = ImGui::Button(
        app.running ? "  Processing..." : "  Deobfuscate",
        {contentW, 0}
    );

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (!canRun) ImGui::EndDisabled();

    if (clicked && canRun) {
        app.hasResult = false;
        std::thread([&app]() { runDeobfuscate(app); }).detach();
    }

    if (app.flushed.exchange(false)) {
        std::lock_guard<std::mutex> lk(app.resultMtx);
        if (app.result.success && app.outputPath[0] != '\0')
            writeFile(app.outputPath, app.result.output);
    }

    ImGui::Spacing();
    sectionHeader("LOG");

    float logH = 180.0f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.04f, 0.04f, 1.0f));
    ImGui::BeginChild("##logbox", {contentW, logH}, true);

    if (app.hasResult) {
        std::lock_guard<std::mutex> lk(app.resultMtx);
        for (const auto& line : app.result.log) {
            ImVec4 col = {0.78f, 0.68f, 0.68f, 1.0f};
            const char* cs = line.c_str();
            if (strncmp(cs, "Done",     4) == 0) col = {0.35f, 0.95f, 0.45f, 1.0f};
            else if (strncmp(cs, "Decoded",  7) == 0 ||
                     strncmp(cs, "Folded",   6) == 0 ||
                     strncmp(cs, "Resolved", 8) == 0) col = {0.35f, 0.90f, 0.45f, 1.0f};
            else if (strncmp(cs, "Found",    5) == 0 ||
                     strncmp(cs, "Input",    5) == 0) col = {0.95f, 0.50f, 0.50f, 1.0f};
            else if (strncmp(cs, "Could",    5) == 0 ||
                     strncmp(cs, "Error",    5) == 0) col = {1.00f, 0.30f, 0.30f, 1.0f};
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextUnformatted(line.c_str());
            ImGui::PopStyleColor();
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 4.0f)
            ImGui::SetScrollHereY(1.0f);
    } else if (app.running) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.40f, 0.40f, 1.0f));
        ImGui::TextUnformatted("Processing...");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.22f, 0.22f, 1.0f));
        ImGui::TextUnformatted("Select an input file and click Deobfuscate.");
        ImGui::PopStyleColor();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (app.hasResult) {
        std::lock_guard<std::mutex> lk(app.resultMtx);
        const auto& st = app.result.stats;

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.04f, 0.04f, 1.0f));
        ImGui::BeginChild("##statsbox", {contentW, 58}, true,
                          ImGuiWindowFlags_NoScrollbar);

        ImGui::Columns(4, "statsrow", false);
        auto stat = [](const char* lbl, std::string val) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.28f, 0.28f, 1.0f));
            ImGui::TextUnformatted(lbl);
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.85f, 0.85f, 1.0f));
            ImGui::TextUnformatted(val.c_str());
            ImGui::PopStyleColor();
        };

        stat("Strings decoded",    std::to_string(st.stringsDecoded));
        ImGui::NextColumn();
        stat("Constants folded",   std::to_string(st.constantsFolded));
        ImGui::NextColumn();
        stat("W() resolved",       std::to_string(st.wCallsResolved));
        ImGui::NextColumn();
        stat("Output",             formatBytes(st.outputBytes));
        ImGui::Columns(1);

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();
        if (!app.result.success) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::Text("Error: %s", app.result.errorMessage.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.90f, 0.45f, 1.0f));
            ImGui::TextUnformatted("File written successfully.");
            ImGui::PopStyleColor();
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_CLASSDC;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"AzazelDeobfuscator";
    wc.hIcon         = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    wc.hIconSm       = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    g_hwnd = CreateWindowExW(
        0,
        L"AzazelDeobfuscator",
        L"Azazel Deobfuscator",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        860, 720,
        nullptr, nullptr,
        hInst, nullptr
    );

    {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(g_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }

    if (!createDeviceD3D(g_hwnd)) {
        cleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, hInst);
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    applyTheme();
    loadLogoTexture();

    const ImVec4 clearCol = {0.039f, 0.027f, 0.027f, 1.0f};
    AppState app{};

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) goto done;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        renderUI(app);

        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRTV, nullptr);
        float cc[4] = {clearCol.x, clearCol.y, clearCol.z, clearCol.w};
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRTV, cc);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

done:
    if (g_logoSRV) { g_logoSRV->Release(); g_logoSRV = nullptr; }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    cleanupDeviceD3D();
    DestroyWindow(g_hwnd);
    UnregisterClassW(wc.lpszClassName, hInst);
    return 0;
}

static bool createDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd            = {};
    sd.BufferCount                     = 2;
    sd.BufferDesc.Format               = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags                           = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                     = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                    = hWnd;
    sd.SampleDesc.Count                = 1;
    sd.Windowed                        = TRUE;
    sd.SwapEffect                      = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl;
    D3D_FEATURE_LEVEL fls[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, fls, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
            &g_pd3dDevice, &fl, &g_pd3dDeviceContext) != S_OK)
        return false;

    createRenderTarget();
    return true;
}

static void createRenderTarget() {
    ID3D11Texture2D* pBack = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBack));
    if (pBack) {
        g_pd3dDevice->CreateRenderTargetView(pBack, nullptr, &g_mainRTV);
        pBack->Release();
    }
}

static void cleanupRenderTarget() {
    if (g_mainRTV) { g_mainRTV->Release(); g_mainRTV = nullptr; }
}

static void cleanupDeviceD3D() {
    cleanupRenderTarget();
    if (g_pSwapChain)        { g_pSwapChain->Release();        g_pSwapChain        = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice)        { g_pd3dDevice->Release();        g_pd3dDevice        = nullptr; }
}
