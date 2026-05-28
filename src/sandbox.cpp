// anyone can fork this as long as there is credit, the ai will not work on other forks.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "sandbox.h"
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>

// embedded sandbox.lua (generic version — httpget returns "" so url is logged only)
static const char* SANDBOX_LUA = R"SANDBOX(
local LOG = {}
local function log(tag, ...)
    local args = {...}
    for i,v in ipairs(args) do args[i] = tostring(v) end
    local msg = "[" .. tag .. "] " .. table.concat(args, " | ")
    table.insert(LOG, msg)
end

local _real_load = load
local _real_loadstring = loadstring

local function safe_loadstring(code, chunkname)
    local codestr = tostring(code or "")
    log("LOADSTRING", "len=" .. #codestr, "preview=" .. codestr:sub(1,120))
    if #codestr < 4 then return function() return nil end end
    local chunk, err = _real_load(codestr, chunkname or "loaded")
    if not chunk then
        log("LOADSTRING_COMPILE_ERR", tostring(err))
        return function() return nil end
    end
    return function(...)
        local ok, result = pcall(chunk, ...)
        if not ok then log("LOADSTRING_RUNTIME_ERR", tostring(result):sub(1,200)) end
        return result
    end
end
loadstring = safe_loadstring
load = safe_loadstring

local _real_io_open = io.open
io.write = function(...) end
io.open  = function(path, mode)
    if mode and mode:find("w") then return nil, "blocked" end
    return nil, "not allowed"
end
io.popen = function() return nil, "blocked" end
os.execute = function(cmd) log("OS_EXEC_BLOCK", tostring(cmd)); return nil end
os.remove  = function(f)   return nil end

local _real_require = require
require = function(name)
    local safe = {math=true,string=true,table=true,io=true,os=true,bit=true,bit32=true,utf8=true}
    if safe[name] then return _real_require(name) end
    log("REQUIRE_BLOCK", tostring(name))
    return {}
end

local function stub_val(name)
    local t = {}
    setmetatable(t, {
        __call    = function(self,...) log("STUB_CALL",name,...); return stub_val(name.."()") end,
        __index   = function(self,k)  return stub_val(name.."."..tostring(k)) end,
        __newindex= function(self,k,v) rawset(self,k,v) end,
        __tostring= function(self) return name end,
        __len=function() return 0 end,
        __add=function(a,b) return a end, __sub=function(a,b) return a end,
        __mul=function(a,b) return a end, __div=function(a,b) return a end,
        __unm=function(a) return a end,   __eq=function(a,b) return false end,
        __lt=function(a,b) return false end, __le=function(a,b) return false end,
        __concat=function(a,b) return tostring(a)..tostring(b) end,
    })
    return t
end

local function make_roblox_instance(class)
    local inst = {ClassName=class}
    local mt = {
        __index = function(t,k)
            if k=="GetService" then
                return function(self,svc)
                    log("GET_SERVICE",svc)
                    local svcs={Players=Players_inst,Workspace=workspace,
                        HttpService=HttpService,StarterGui=StarterGui,CoreGui=CoreGui}
                    return svcs[svc] or make_roblox_instance(svc)
                end
            elseif k=="HttpGet" then
                return function(self,url,...) log("HTTP_GET",url); return "" end
            elseif k=="GetAsync" then
                return function(self,url,...) log("HTTP_GET_ASYNC",url); return "" end
            elseif k=="PostAsync" or k=="SendAsync" then
                return function(self,url,body,...)
                    log("HTTP_POST",url,tostring(body):sub(1,200)); return ""
                end
            elseif k=="WaitForChild" then
                return function(self,name,...) log("WAIT_FOR_CHILD",class,name); return make_roblox_instance(name) end
            elseif k=="FindFirstChild" then
                return function(self,name,...) return make_roblox_instance(name) end
            elseif k=="FindFirstChildOfClass" then
                return function(self,cls,...) return make_roblox_instance(cls) end
            elseif k=="GetChildren" or k=="GetDescendants" then
                return function(self) return {} end
            elseif k=="IsDescendantOf" then return function() return false end
            elseif k=="IsA" then return function(self,cls) return self.ClassName==cls end
            elseif k=="Connect" or k=="connect" then
                return function(self,fn) log("CONNECT",class,k); return {Disconnect=function() end} end
            elseif k=="Disconnect" then return function() end
            elseif k=="Fire" or k=="FireServer" or k=="FireClient" then
                return function(self,...) log("REMOTE_FIRE",...) end
            elseif k=="Invoke" or k=="InvokeServer" then
                return function(self,...) log("REMOTE_INVOKE",...); return nil end
            elseif k=="Kick" then return function(self,msg) log("KICK",tostring(msg)) end
            elseif k=="BreakJoints" then return function(self) log("BREAK_JOINTS",class) end
            elseif k=="SetStateEnabled" then
                return function(self,state,en) log("SET_STATE",tostring(state),tostring(en)) end
            elseif k=="Destroy" then return function(self) end
            elseif k=="MoveTo" then return function(self,pos) end
            elseif k=="Clone" then return function(self) return make_roblox_instance(class) end
            elseif k=="GetPlayingAnimationTracks" then return function() return {} end
            elseif k=="SetAttribute" or k=="GetAttribute" then return function() return nil end
            elseif k=="new" then
                return function(...)
                    log("INSTANCE_NEW",...)
                    return make_roblox_instance(tostring(...))
                end
            else return stub_val(class.."."..tostring(k)) end
        end,
        __newindex=function(t,k,v) rawset(t,k,v) end,
        __tostring=function(t) return "RobloxInstance<"..t.ClassName..">" end,
        __call=function(t,...) log("INSTANCE_CALL",t.ClassName,...); return make_roblox_instance(t.ClassName) end,
        __len=function() return 0 end,
        __add=function(a,b) return a end, __mul=function(a,b) return a end,
        __concat=function(a,b) return tostring(a)..tostring(b) end,
    }
    setmetatable(inst,mt)
    return inst
end

game=make_roblox_instance("DataModel")
workspace=make_roblox_instance("Workspace")
local Character=make_roblox_instance("Model")
local LocalPlayer=make_roblox_instance("Player")
rawset(LocalPlayer,"Name","TargetPlayer")
rawset(LocalPlayer,"UserId",12345678)
rawset(LocalPlayer,"Character",Character)
rawset(LocalPlayer,"PlayerGui",make_roblox_instance("PlayerGui"))
rawset(LocalPlayer,"Backpack",make_roblox_instance("Backpack"))
rawset(LocalPlayer,"RespawnTime",5)
rawset(LocalPlayer,"GetMouse",function(self) return make_roblox_instance("Mouse") end)
local Players_inst=make_roblox_instance("Players")
rawset(Players_inst,"LocalPlayer",LocalPlayer)
rawset(Players_inst,"GetPlayers",function(self) return {LocalPlayer} end)
rawset(Players_inst,"GetPlayerFromCharacter",function(self,c) return LocalPlayer end)
Players=Players_inst
script=make_roblox_instance("LocalScript")
shared={}
_G={}
Vector3={new=function(x,y,z) return {x=x or 0,y=y or 0,z=z or 0} end,fromNormalId=function(...) return Vector3.new() end}
Vector2={new=function(x,y) return {x=x or 0,y=y or 0} end}
CFrame={new=function(...) return setmetatable({},{__mul=function(a,b) return a end,__add=function(a,b) return a end}) end,Angles=function(...) return CFrame.new() end,lookAt=function(...) return CFrame.new() end}
Color3={new=function(r,g,b) return {r=r or 0,g=g or 0,b=b or 0} end,fromRGB=function(r,g,b) return Color3.new(r/255,g/255,b/255) end}
UDim2={new=function(...) return {} end}
UDim={new=function(...) return {} end}
TweenInfo={new=function(...) return {} end}
Enum=setmetatable({},{__index=function(t,k) return setmetatable({},{__index=function(t2,k2) return {Name=k2,Value=0} end}) end})
Instance={new=function(cls,parent) log("INSTANCE_NEW",cls); return make_roblox_instance(cls) end}
NumberSequenceKeypoint={new=function(...) return {} end}
ColorSequenceKeypoint={new=function(...) return {} end}
NumberSequence={new=function(...) return {} end}
ColorSequence={new=function(...) return {} end}
PhysicalProperties={new=function(...) return {} end}
BodyVelocity={new=function() return make_roblox_instance("BodyVelocity") end}
BodyGyro={new=function() return make_roblox_instance("BodyGyro") end}
BodyPosition={new=function() return make_roblox_instance("BodyPosition") end}
BodyAngularVelocity={new=function() return make_roblox_instance("BodyAngularVelocity") end}
AlignPosition={new=function() return make_roblox_instance("AlignPosition") end}
AlignOrientation={new=function() return make_roblox_instance("AlignOrientation") end}
Attachment={new=function() return make_roblox_instance("Attachment") end}
RaycastParams={new=function() return make_roblox_instance("RaycastParams") end}
StarterGui=make_roblox_instance("StarterGui")
CoreGui=make_roblox_instance("CoreGui")
task={
    spawn=function(fn,...) local ok,err=pcall(fn,...) end,
    wait=function(t) return t or 0 end,
    delay=function(t,fn) return {} end,
    cancel=function(t) end,
}
wait=function(t) return t or 0 end
spawn=function(fn) end
delay=function(t,fn) end
HttpService=make_roblox_instance("HttpService")
newproxy=function(mt) return setmetatable({},mt and {} or nil) end

io.write=function(...) end
local script_path=arg and arg[1] or ""
local f,err=_real_io_open(script_path,"r")
if not f then
    print("ERROR: " .. tostring(err))
    os.exit(1)
end
local source=f:read("*a")
f:close()
unpack=table.unpack
getfenv=function(level) return _ENV end
setfenv=function(level,env) end
if not bit32 then
    bit32={
        bxor=function(a,b) return a~b end,
        band=function(a,b) return a&b end,
        bor=function(a,b) return a|b end,
        bnot=function(a) return ~a end,
        rshift=function(a,b) return a>>b end,
        lshift=function(a,b) return a<<b end,
    }
end
local chunk,compile_err=_real_load(source,"script")
if not chunk then
    local compat="getfenv=function(l)return _ENV or {} end\nsetfenv=function(l,e) end\nunpack=table.unpack\n"
    chunk,compile_err=_real_load(compat..source,"script_compat")
end
if chunk then
    local ok,runtime_err=pcall(chunk)
end
print("=== SANDBOX LOG ===")
for _,entry in ipairs(LOG) do print(entry) end
print("=== END LOG ===")
)SANDBOX";

// find lua.exe in common locations
std::string findLuaExe() {
    static const char* candidates[] = {
        "C:/Users/lifenock/AppData/Local/Programs/Lua/bin/lua.exe",
        "C:/Program Files/Lua/lua.exe",
        "C:/Program Files (x86)/Lua/lua.exe",
        "lua.exe",
        nullptr
    };
    for (int i = 0; candidates[i]; i++) {
        if (GetFileAttributesA(candidates[i]) != INVALID_FILE_ATTRIBUTES)
            return candidates[i];
    }
    // search PATH
    char buf[MAX_PATH];
    if (SearchPathA(nullptr, "lua.exe", nullptr, MAX_PATH, buf, nullptr))
        return std::string(buf);
    return "";
}

static std::string runCapture(const std::string& exe, const std::string& sandboxPath, const std::string& scriptPath) {
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return "";
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hWrite;
    si.hStdError  = hWrite;
    si.dwFlags    = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::string cmd = "\"" + exe + "\" \"" + sandboxPath + "\" \"" + scriptPath + "\"";
    std::vector<char> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back('\0');

    PROCESS_INFORMATION pi = {};
    if (!CreateProcessA(nullptr, cmdBuf.data(), nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        CloseHandle(hRead); CloseHandle(hWrite);
        return "";
    }
    CloseHandle(hWrite);

    std::string output;
    char buf[4096];
    DWORD read;
    while (ReadFile(hRead, buf, sizeof(buf) - 1, &read, nullptr) && read > 0)
        output.append(buf, read);

    WaitForSingleObject(pi.hProcess, 30000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);
    return output;
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

static std::vector<std::string> splitPipe(const std::string& s) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string part;
    while (std::getline(ss, part, '|'))
        parts.push_back(trim(part));
    return parts;
}

static std::string logToLua(const std::string& rawLog) {
    std::istringstream stream(rawLog);
    std::string line;

    bool inLog = false;
    std::vector<std::string> services;
    std::set<std::string>    seenSvc;
    std::vector<std::string> remoteFires;
    std::string              remoteEventName;
    std::vector<std::string> httpUrls;
    int                      loadstringCount = 0;
    std::vector<std::string> instances;
    std::vector<std::string> connections;
    std::vector<std::string> chatMessages;
    std::vector<std::string> waitForChildCalls;
    std::set<std::string>    seenWFC;

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line == "=== SANDBOX LOG ===") { inLog = true; continue; }
        if (line == "=== END LOG ===")      { inLog = false; continue; }
        if (!inLog || line.empty())          continue;

        if (line.size() > 14 && line.substr(0, 14) == "[GET_SERVICE] ") {
            std::string svc = line.substr(14);
            if (!seenSvc.count(svc)) { seenSvc.insert(svc); services.push_back(svc); }
        } else if (line.size() > 14 && line.substr(0, 14) == "[REMOTE_FIRE] ") {
            auto parts = splitPipe(line.substr(14));
            if (!parts.empty()) {
                if (remoteEventName.empty()) remoteEventName = "cmd";
                remoteFires.push_back(line.substr(14));
            }
        } else if (line.size() > 11 && line.substr(0, 11) == "[HTTP_GET] ") {
            httpUrls.push_back(line.substr(11));
        } else if (line.size() > 17 && line.substr(0, 17) == "[HTTP_GET_ASYNC] ") {
            httpUrls.push_back(line.substr(17));
        } else if (line.size() > 13 && line.substr(0, 13) == "[LOADSTRING] ") {
            loadstringCount++;
        } else if (line.size() > 15 && line.substr(0, 15) == "[INSTANCE_NEW] ") {
            instances.push_back(line.substr(15));
        } else if (line.size() > 13 && line.substr(0, 13) == "[WAIT_FOR_CHILD] ") {
            // [WAIT_FOR_CHILD] | parentClass | childName
            auto parts = splitPipe(line.substr(13));
            if (parts.size() >= 2) {
                std::string entry = parts[0] + " -> " + parts[1];
                if (!seenWFC.count(entry)) { seenWFC.insert(entry); waitForChildCalls.push_back(entry); }
            }
        } else if (line.size() > 12 && line.substr(0, 12) == "[STUB_CALL] ") {
            std::string body = line.substr(12);
            if (body.find("SendAsync") != std::string::npos) {
                size_t last = body.rfind(" | ");
                if (last != std::string::npos) chatMessages.push_back(body.substr(last + 3));
            } else if (body.find(".Connect") != std::string::npos || body.find("Connect") != std::string::npos) {
                // extract event name
                size_t pipe = body.find(" | ");
                std::string chain = (pipe != std::string::npos) ? body.substr(0, pipe) : body;
                connections.push_back(chain);
            }
        } else if (line.size() > 9 && line.substr(0, 9) == "[CONNECT]") {
            auto parts = splitPipe(line.substr(9));
            if (parts.size() >= 2) connections.push_back(parts[0] + "." + parts[1]);
        }
    }

    std::ostringstream lua;
    lua << "-- deobfuscated by lifenock\n";
    lua << "-- Azazel Deobfuscator - sandbox analysis\n\n";

    // services
    if (!services.empty()) {
        lua << "-- services\n";
        for (auto& s : services)
            lua << "local " << s << " = game:GetService(\"" << s << "\")\n";
        lua << "\n";
    }

    // remote event setup + fires
    if (!remoteFires.empty()) {
        lua << "-- remote events\n";
        lua << "local cmd = ReplicatedStorage:WaitForChild(\"cmd\")\n";
        for (auto& f : remoteFires) {
            auto parts = splitPipe(f);
            lua << "cmd:FireServer(";
            for (size_t i = 0; i < parts.size(); i++) {
                if (i) lua << ", ";
                lua << "\"" << parts[i] << "\"";
            }
            lua << ")\n";
        }
        lua << "\n";
    }

    // http + loadstring
    if (!httpUrls.empty()) {
        lua << "-- http / loadstring\n";
        for (size_t i = 0; i < httpUrls.size(); i++) {
            lua << "local payload_" << (i + 1) << " = game:GetService(\"HttpService\"):GetAsync(\""
                << httpUrls[i] << "\")\n";
        }
        if (loadstringCount > 0)
            lua << "loadstring(payload_1)()\n";
        lua << "\n";
    }

    // instances
    if (!instances.empty()) {
        lua << "-- instances created\n";
        std::map<std::string, int> counters;
        for (auto& cls : instances) {
            counters[cls]++;
            std::string var = cls;
            if (!var.empty()) var[0] = (char)tolower((unsigned char)var[0]);
            if (counters[cls] > 1) var += "_" + std::to_string(counters[cls]);
            lua << "local " << var << " = Instance.new(\"" << cls << "\")\n";
        }
        lua << "\n";
    }

    // event connections
    if (!connections.empty()) {
        lua << "-- event connections\n";
        for (auto& c : connections) {
            size_t dot = c.rfind('.');
            std::string eventName = (dot != std::string::npos) ? c.substr(dot + 1) : c;
            lua << "-- " << eventName << ":Connect(function() end)\n";
        }
        lua << "\n";
    }

    // chat
    if (!chatMessages.empty()) {
        lua << "-- chat messages\n";
        for (auto& msg : chatMessages)
            lua << "TextChatService.TextChannels.RBXGeneral:SendAsync(\"" << msg << "\")\n";
        lua << "\n";
    }

    if (services.empty() && remoteFires.empty() && httpUrls.empty() && instances.empty()) {
        lua << "-- sandbox captured no events.\n";
        lua << "-- the script may not be Prometheus-obfuscated or may require a Roblox environment.\n";
    }

    return lua.str();
}

SandboxResult runSandboxDeobfuscate(const std::string& source) {
    SandboxResult result;

    result.luaExePath = findLuaExe();
    if (result.luaExePath.empty()) {
        result.error = "Lua 5.4 not found. Install from https://www.lua.org/download.html";
        return result;
    }

    // write sandbox lua to temp
    char tempDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);

    std::string sandboxPath = std::string(tempDir) + "azazel_sandbox.lua";
    std::string scriptPath  = std::string(tempDir) + "azazel_input.lua";

    {
        HANDLE h = CreateFileA(sandboxPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            result.error = "Cannot write sandbox to temp";
            return result;
        }
        DWORD written;
        WriteFile(h, SANDBOX_LUA, (DWORD)strlen(SANDBOX_LUA), &written, nullptr);
        CloseHandle(h);
    }
    {
        HANDLE h = CreateFileA(scriptPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            result.error = "Cannot write input to temp";
            return result;
        }
        DWORD written;
        WriteFile(h, source.data(), (DWORD)source.size(), &written, nullptr);
        CloseHandle(h);
    }

    result.rawLog = runCapture(result.luaExePath, sandboxPath, scriptPath);

    if (result.rawLog.find("=== SANDBOX LOG ===") == std::string::npos) {
        result.error = "Sandbox produced no log (Lua error or timeout).\n" + result.rawLog.substr(0, 500);
        return result;
    }

    result.output  = logToLua(result.rawLog);
    result.success = true;
    return result;
}
