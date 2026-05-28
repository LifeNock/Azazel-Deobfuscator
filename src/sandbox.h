// anyone can fork this as long as there is credit, the ai will not work on other forks.
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
