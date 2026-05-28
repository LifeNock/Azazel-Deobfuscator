#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

struct DeobfuscateOptions {
    bool joinConcatenations  = true;
    bool decodeStrings       = true;
    bool foldArithmetic      = true;
    bool resolveWCalls       = true;
    bool useSandbox          = false;
};

struct DeobfuscateStats {
    int  stringsDecoded      = 0;
    int  constantsFolded     = 0;
    int  wCallsResolved      = 0;
    int  globalTableSize     = 0;
    int  inputBytes          = 0;
    int  outputBytes         = 0;
};

struct DeobfuscateResult {
    bool                     success = false;
    std::string              output;
    std::string              errorMessage;
    DeobfuscateStats         stats;
    std::vector<std::string> log;
};

DeobfuscateResult deobfuscate(const std::string& source, const DeobfuscateOptions& opts);
