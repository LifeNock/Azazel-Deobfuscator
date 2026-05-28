// anyone can fork this as long as there is credit, the ai will not work on other forks.
#include "deobfuscator.h"
#include "sandbox.h"
#include <regex>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>

static const std::unordered_map<char, int> ALPHA = {
    {'h',0},{'T',1},{'z',2},{'Y',3},{'K',4},{'A',5},{'H',6},{'2',7},{'t',8},{'4',9},
    {'j',10},{'v',11},{'P',12},{'S',13},{'Z',14},{'q',15},{'B',16},{'d',17},{'C',18},{'X',19},
    {'Q',20},{'D',21},{'y',22},{'O',23},{'u',24},{'U',25},{'n',26},{'6',27},{'E',28},{'c',29},
    {'r',30},{'l',31},{'a',32},{'9',33},{'V',34},{'e',35},{'s',36},{'1',37},{'f',38},{'w',39},
    {'b',40},{'5',41},{'M',42},{'F',43},{'G',44},{'o',45},{'3',46},{'R',47},{'+',48},{'x',49},
    {'L',50},{'7',51},{'N',52},{'m',53},{'p',54},{'g',55},{'k',56},{'0',57},{'/',58},{'J',59},
    {'I',60},{'i',61},{'W',62},{'8',63}
};

static std::string customB64Decode(const std::string& s) {
    std::string result;
    long long h = 0;
    int Q = 0;
    for (char ch : s) {
        if (ch == '=') {
            result += (char)((h / 65536) & 0xFF);
            if (Q >= 2) result += (char)(((h % 65536) / 256) & 0xFF);
            break;
        }
        auto it = ALPHA.find(ch);
        if (it == ALPHA.end()) continue;
        long long pw = 1;
        for (int i = 0; i < 3 - Q; i++) pw *= 64;
        h += it->second * pw;
        Q++;
        if (Q == 4) {
            Q = 0;
            result += (char)((h / 65536) & 0xFF);
            result += (char)(((h % 65536) / 256) & 0xFF);
            result += (char)(h % 256);
            h = 0;
        }
    }
    return result;
}

static std::string decEscape(const std::string& s) {
    std::string out;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '\\' && i + 1 < s.size() && std::isdigit((unsigned char)s[i + 1])) {
            size_t j = i + 1;
            while (j < s.size() && std::isdigit((unsigned char)s[j]) && (j - i) <= 3) j++;
            int val = std::stoi(s.substr(i + 1, j - i - 1));
            out += (char)(val & 0xFF);
            i = j;
        } else {
            out += s[i++];
        }
    }
    return out;
}

static std::string nncDecode(const std::string& s) {
    std::string out;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '\\' && i + 1 < s.size() && std::isdigit((unsigned char)s[i + 1])) {
            size_t j = i + 1;
            while (j < s.size() && std::isdigit((unsigned char)s[j])) j++;
            if (j < s.size() && std::islower((unsigned char)s[j])) {
                int nn = std::stoi(s.substr(i + 1, j - i - 1));
                int lo = s[j] - 'a';
                out += (char)(nn * 10 + lo);
                i = j + 1;
                continue;
            }
        }
        out += s[i++];
    }
    return out;
}

static std::string toPrintable(const std::string& s) {
    std::string out;
    for (char c : s)
        if ((unsigned char)c >= 32 && (unsigned char)c <= 126)
            out += c;
    return out;
}

static std::string luaEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            default:   out += c;
        }
    }
    return out;
}

static bool hasEscapeSeq(const std::string& s) {
    for (size_t i = 0; i + 1 < s.size(); i++)
        if (s[i] == '\\' && std::isdigit((unsigned char)s[i + 1]))
            return true;
    return false;
}

static bool hasNncSeq(const std::string& s) {
    std::regex r("\\\\[0-9]{1,2}[a-z]");
    return std::regex_search(s, r);
}

static std::optional<std::string> decodePrometheusString(const std::string& raw) {
    bool isNnc = hasNncSeq(raw);
    bool isDec = hasEscapeSeq(raw);
    if (!isNnc && !isDec) return std::nullopt;

    std::string stage1 = isNnc ? nncDecode(raw) : decEscape(raw);

    std::string decoded   = customB64Decode(stage1);
    std::string printable = toPrintable(decoded);
    if (printable.size() >= 2) return printable;

    if (isNnc) {
        std::string direct = toPrintable(stage1);
        if (direct.size() >= 2) return direct;
    }
    return std::nullopt;
}

// scan a double-quoted lua string starting at pos, fill inner, return end pos
static size_t readLuaStr(const std::string& s, size_t pos, std::string& inner) {
    inner.clear();
    size_t i = pos + 1;
    while (i < s.size()) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            inner += s[i]; inner += s[i + 1]; i += 2;
        } else if (s[i] == '"') {
            return i + 1;
        } else {
            inner += s[i++];
        }
    }
    return std::string::npos;
}

static std::string joinConcatenations(const std::string& code) {
    std::string result = code;
    bool changed = true;
    int passes = 0;
    while (changed && passes < 60) {
        changed = false;
        std::string next;
        next.reserve(result.size());
        size_t i = 0;
        while (i < result.size()) {
            if (result[i] != '"') { next += result[i++]; continue; }

            std::string inner1;
            size_t end1 = readLuaStr(result, i, inner1);
            if (end1 == std::string::npos) { next += result[i++]; continue; }

            size_t k = end1;
            while (k < result.size() && (result[k]==' '||result[k]=='\t'||result[k]=='\n'||result[k]=='\r')) k++;

            if (k + 1 < result.size() && result[k] == '.' && result[k+1] == '.'
                && (k + 2 >= result.size() || result[k+2] != '.')) {
                k += 2;
                while (k < result.size() && (result[k]==' '||result[k]=='\t'||result[k]=='\n'||result[k]=='\r')) k++;

                if (k < result.size() && result[k] == '"') {
                    std::string inner2;
                    size_t end2 = readLuaStr(result, k, inner2);
                    if (end2 != std::string::npos) {
                        next += '"'; next += inner1; next += inner2; next += '"';
                        i = end2;
                        changed = true;
                        continue;
                    }
                }
            }

            next.append(result, i, end1 - i);
            i = end1;
        }
        result = std::move(next);
        passes++;
    }
    return result;
}

static std::string decodeStrings(const std::string& code, int& count) {
    std::string result;
    result.reserve(code.size());
    size_t i = 0;
    while (i < code.size()) {
        if (code[i] != '"') { result += code[i++]; continue; }

        std::string inner;
        size_t end = readLuaStr(code, i, inner);
        if (end == std::string::npos) { result += code[i++]; continue; }

        auto decoded = decodePrometheusString(inner);
        if (decoded) {
            result += '"'; result += luaEscape(*decoded); result += '"';
            count++;
        } else {
            result.append(code, i, end - i);
        }
        i = end;
    }
    return result;
}

static std::optional<double> evalSimpleArith(const std::string& expr) {
    struct Term { double v; char op; };
    std::vector<Term> terms;
    char pendingOp = '+';
    size_t i = 0;
    const std::string& e = expr;

    while (i < e.size()) {
        while (i < e.size() && e[i] == ' ') i++;
        if (i >= e.size()) break;

        if ((e[i] == '+' || e[i] == '-') && !terms.empty()) {
            if (e[i] == '-' && i + 1 < e.size() &&
                (std::isdigit((unsigned char)e[i+1]) || e[i+1] == '(')) {
                pendingOp = '-';
                i++;
                continue;
            }
            pendingOp = e[i++];
            continue;
        }

        bool neg = false;
        if (e[i] == '-') { neg = true; i++; }
        while (i < e.size() && e[i] == ' ') i++;
        if (i >= e.size()) return std::nullopt;

        double v = 0;
        if (e[i] == '(') {
            int depth = 1;
            size_t j = i + 1;
            while (j < e.size() && depth > 0) {
                if (e[j] == '(') depth++;
                else if (e[j] == ')') depth--;
                j++;
            }
            if (depth != 0) return std::nullopt;
            auto sub = evalSimpleArith(e.substr(i + 1, j - i - 2));
            if (!sub) return std::nullopt;
            v = *sub;
            i = j;
        } else {
            if (!std::isdigit((unsigned char)e[i]) && e[i] != '.')
                return std::nullopt;
            size_t start = i;
            while (i < e.size() && (std::isdigit((unsigned char)e[i]) || e[i] == '.')) i++;
            if (i == start) return std::nullopt;
            v = std::stod(e.substr(start, i - start));
        }

        if (neg) v = -v;
        terms.push_back({ v, pendingOp });
        pendingOp = '+';
    }

    if (terms.empty()) return std::nullopt;
    double r = 0;
    for (auto& t : terms) r = (t.op == '+') ? r + t.v : r - t.v;
    return r;
}

static std::string foldArithmetic(const std::string& code, int& count) {
    std::regex pat("-?[0-9]+(?:\\.[0-9]+)?(?:\\s*[+\\-]\\s*-?[0-9]+(?:\\.[0-9]+)?)+");
    std::string result;
    result.reserve(code.size());
    size_t last = 0;
    auto begin = std::sregex_iterator(code.begin(), code.end(), pat);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const std::smatch& m = *it;
        result.append(code, last, (size_t)m.position() - last);
        std::string expr = m[0].str();
        auto val = evalSimpleArith(expr);
        if (val) {
            double v = *val;
            if (v == std::floor(v) && std::abs(v) < 1e15) {
                result += std::to_string((long long)v);
            } else {
                std::ostringstream oss;
                oss << v;
                result += oss.str();
            }
            count++;
        } else {
            result += expr;
        }
        last = (size_t)m.position() + (size_t)m.length();
    }
    result.append(code, last, std::string::npos);
    return result;
}

static std::vector<std::string> extractGlobalTable(const std::string& code) {
    std::vector<std::string> table;
    const std::string marker = "local g";
    size_t p = 0;
    while ((p = code.find(marker, p)) != std::string::npos) {
        size_t q = p + marker.size();
        while (q < code.size() && (code[q]==' '||code[q]=='\t')) q++;
        if (q >= code.size() || code[q] != '=') { p++; continue; }
        q++;
        while (q < code.size() && (code[q]==' '||code[q]=='\t')) q++;
        if (q >= code.size() || code[q] != '{') { p++; continue; }

        size_t blockStart = q + 1;
        size_t blockEnd = std::string::npos;
        size_t s = blockStart;
        while ((s = code.find('}', s)) != std::string::npos) {
            size_t after = s + 1;
            while (after < code.size() && (unsigned char)code[after] <= ' ') after++;
            if (code.compare(after, 14, "local function") == 0) { blockEnd = s; break; }
            s++;
        }
        if (blockEnd == std::string::npos) { p++; continue; }

        size_t i = blockStart;
        while (i < blockEnd) {
            if (code[i] == '"') {
                std::string inner;
                size_t end = readLuaStr(code, i, inner);
                if (end != std::string::npos && end <= blockEnd + 1) {
                    table.push_back(std::move(inner));
                    i = end;
                    continue;
                }
            }
            i++;
        }

        if (!table.empty()) return table;
        p++;
    }
    return table;
}

static int findOffset(const std::string& code) {
    const std::string needle = "return g[W+(";
    size_t p = code.find(needle);
    if (p == std::string::npos) return 19027;
    size_t i = p + needle.size();
    bool neg = (i < code.size() && code[i] == '-');
    if (neg) i++;
    size_t start = i;
    while (i < code.size() && std::isdigit((unsigned char)code[i])) i++;
    if (i == start) return 19027;
    return (neg ? -1 : 1) * std::stoi(code.substr(start, i - start));
}

static std::string resolveWCalls(const std::string& code,
                                  const std::vector<std::string>& gTable,
                                  int offset, int& count) {
    std::string result;
    result.reserve(code.size());
    size_t i = 0;
    while (i < code.size()) {
        if (code[i] == 'W' && i + 1 < code.size() && code[i+1] == '(') {
            bool prevOk = (i == 0 || (!std::isalnum((unsigned char)code[i-1]) && code[i-1] != '_'));
            if (prevOk) {
                size_t argStart = i + 2;
                int depth = 1;
                size_t j = argStart;
                while (j < code.size() && depth > 0) {
                    if      (code[j] == '(') depth++;
                    else if (code[j] == ')') depth--;
                    if (depth > 0) j++;
                }
                if (depth == 0 && j - argStart <= 80) {
                    std::string expr = code.substr(argStart, j - argStart);
                    auto val = evalSimpleArith(expr);
                    if (val) {
                        int idx = (int)std::round(*val) + offset;
                        if (idx >= 1 && idx <= (int)gTable.size()) {
                            result += '"';
                            result += luaEscape(gTable[idx - 1]);
                            result += '"';
                            count++;
                            i = j + 1;
                            continue;
                        }
                    }
                }
            }
        }
        result += code[i++];
    }
    return result;
}

DeobfuscateResult deobfuscate(const std::string& source, const DeobfuscateOptions& opts) {
    DeobfuscateResult res;
    res.stats.inputBytes = (int)source.size();

    auto log = [&](const std::string& msg) { res.log.push_back(msg); };

    log("Input: " + std::to_string(source.size()) + " bytes");

    if (opts.useSandbox) {
        log("Running sandbox analysis...");
        auto sr = runSandboxDeobfuscate(source);
        if (sr.success) {
            log("Lua: " + sr.luaExePath);
            res.output        = sr.output;
            res.stats.outputBytes = (int)sr.output.size();
            res.success       = true;
            log("Done. Output: " + std::to_string(sr.output.size()) + " bytes");
            return res;
        } else {
            log("Sandbox failed: " + sr.error);
            log("Falling back to static analysis...");
        }
    }

    std::string code = source;

    auto tryPass = [&](const char* name, auto fn) {
        try { fn(); }
        catch (const std::exception& e) { log(std::string(name) + " failed: " + e.what()); }
        catch (...) { log(std::string(name) + " failed."); }
    };

    if (opts.joinConcatenations) {
        log("Joining concatenated string literals...");
        tryPass("join", [&]{ code = joinConcatenations(code); });
    }

    if (opts.decodeStrings) {
        log("Decoding string constants...");
        tryPass("decode", [&]{ code = decodeStrings(code, res.stats.stringsDecoded); });
        log("Decoded " + std::to_string(res.stats.stringsDecoded) + " strings");
    }

    if (opts.foldArithmetic) {
        log("Folding arithmetic constants...");
        tryPass("fold", [&]{ code = foldArithmetic(code, res.stats.constantsFolded); });
        log("Folded " + std::to_string(res.stats.constantsFolded) + " constants");
    }

    if (opts.resolveWCalls) {
        log("Extracting global string table...");
        std::vector<std::string> gTable;
        tryPass("extract", [&]{ gTable = extractGlobalTable(code); });
        res.stats.globalTableSize = (int)gTable.size();
        if (!gTable.empty()) {
            log("Found " + std::to_string(gTable.size()) + " table entries");
            int offset = findOffset(code);
            log("W() offset: " + std::to_string(offset));
            log("Resolving W() calls...");
            tryPass("resolve", [&]{ code = resolveWCalls(code, gTable, offset, res.stats.wCallsResolved); });
            log("Resolved " + std::to_string(res.stats.wCallsResolved) + " calls");
        } else {
            log("No Prometheus table found.");
        }
    }

    res.output           = code;
    res.stats.outputBytes = (int)code.size();
    res.success          = true;
    log("Done. Output: " + std::to_string(code.size()) + " bytes");
    return res;
}
