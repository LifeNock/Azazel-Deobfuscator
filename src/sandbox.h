#pragma once
#include <string>

struct SandboxResult {
    bool        success    = false;
    std::string luaExePath;
    std::string rawLog;
    std::string output;
    std::string error;
};

std::string   findLuaExe();
SandboxResult runSandboxDeobfuscate(const std::string& source);
