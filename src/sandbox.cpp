#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "sandbox.h"
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

static const char SANDBOX_LUA_A[] = R"SBA(
local LOG = {}
local cb_depth = 0

local function log(tag, ...)
    local args = {...}
    for i,v in ipairs(args) do args[i] = tostring(v) end
    table.insert(LOG, "[" .. tag .. "] " .. table.concat(args, "\t"))
end

local _real_load = load
local _real_io_open = io.open

local function safe_loadstring(code, name)
    local s = tostring(code or "")
    log("LOADSTRING", #s, s:sub(1,80))
    if #s < 4 then return function() return nil end end
    local chunk, err = _real_load(s, name or "loaded")
    if not chunk then log("LOADSTRING_ERR", tostring(err)); return function() return nil end end
    return function(...)
        local ok, r = pcall(chunk, ...)
        if not ok then log("LOADSTRING_RUNTIME_ERR", tostring(r):sub(1,200)) end
        return r
    end
end
loadstring = safe_loadstring
load = safe_loadstring

io.write  = function() end
io.open   = function(p,m) if m and m:find("w") then return nil,"blocked" end return nil,"not allowed" end
io.popen  = function() return nil,"blocked" end
os.execute = function(cmd) log("OS_EXEC", tostring(cmd)); return nil end
os.remove  = function() return nil end
require    = function(n)
    local safe={math=true,string=true,table=true,io=true,os=true,bit=true,bit32=true,utf8=true}
    if safe[n] then return _real_load("return require")()(n) end
    log("REQUIRE_BLOCK", n); return {}
end

Color3 = {
    new = function(r,g,b)
        r,g,b = (r or 0)*255,(g or 0)*255,(b or 0)*255
        return setmetatable({r=r,g=g,b=b},{__tostring=function(s) return ("Color3.fromRGB(%g,%g,%g)"):format(s.r,s.g,s.b) end})
    end,
    fromRGB = function(r,g,b)
        return setmetatable({r=r or 0,g=g or 0,b=b or 0},{__tostring=function(s) return ("Color3.fromRGB(%g,%g,%g)"):format(s.r,s.g,s.b) end})
    end,
    fromHSV = function(h,s,v)
        return setmetatable({},{__tostring=function() return ("Color3.fromHSV(%g,%g,%g)"):format(h or 0,s or 0,v or 0) end})
    end,
}
UDim2 = {new=function(xs,xo,ys,yo)
    return setmetatable({xs=xs or 0,xo=xo or 0,ys=ys or 0,yo=yo or 0},{
        __tostring=function(s) return ("UDim2.new(%g,%g,%g,%g)"):format(s.xs,s.xo,s.ys,s.yo) end,
        __add=function(a,b) return a end, __sub=function(a,b) return a end,
    })
end}
UDim = {new=function(s,o)
    return setmetatable({s=s or 0,o=o or 0},{__tostring=function(self) return ("UDim.new(%g,%g)"):format(self.s,self.o) end})
end}
Vector3 = {
    new=function(x,y,z) return setmetatable({x=x or 0,y=y or 0,z=z or 0},{
        __tostring=function(s) return ("Vector3.new(%g,%g,%g)"):format(s.x,s.y,s.z) end,
        __add=function(a,b) return a end,__sub=function(a,b) return a end,
        __mul=function(a,b) return a end,__unm=function(a) return a end,
        __index=function(t,k) return 0 end,
    }) end,
    fromNormalId=function() return Vector3.new() end,
    zero=setmetatable({x=0,y=0,z=0},{__tostring=function() return "Vector3.zero" end}),
    one=setmetatable({x=1,y=1,z=1},{__tostring=function() return "Vector3.one" end}),
}
Vector2 = {new=function(x,y) return setmetatable({x=x or 0,y=y or 0},{
    __tostring=function(s) return ("Vector2.new(%g,%g)"):format(s.x,s.y) end,
    __add=function(a,b) return a end,__sub=function(a,b) return a end,
    __index=function(t,k) return 0 end,
}) end}
CFrame = {
    new=function(...)
        local a,p={...},{}; for _,v in ipairs(a) do p[#p+1]=tostring(v) end
        local r="CFrame.new("..table.concat(p,",")..")"
        return setmetatable({},{__tostring=function() return r end,
            __mul=function(a,b) return a end,__add=function(a,b) return a end,
            __index=function(t,k) return setmetatable({},{__tostring=function() return r.."."..k end,__call=function() return CFrame.new() end}) end})
    end,
    Angles=function(...) return CFrame.new() end,
    lookAt=function(...) return CFrame.new() end,
    fromEulerAnglesXYZ=function(...) return CFrame.new() end,
}
TweenInfo = {new=function(t,es,ed,rc,rev,dt)
    return setmetatable({t=t,es=es,ed=ed,rc=rc,rev=rev,dt=dt},{
        __tostring=function(s) return ("TweenInfo.new(%g,%s,%s,%g,%s,%g)"):format(
            s.t or 0,tostring(s.es),tostring(s.ed),s.rc or 0,tostring(s.rev or false),s.dt or 0) end
    })
end}
Enum = setmetatable({},{__index=function(t,k)
    return setmetatable({},{__index=function(t2,k2)
        return setmetatable({Name=k2,Value=0},{
            __tostring=function() return "Enum."..k.."."..k2 end,
            __eq=function(a,b) return rawequal(a,b) end,
        })
    end})
end})
NumberSequenceKeypoint={new=function(t,v,e) return setmetatable({},{__tostring=function() return ("NumberSequenceKeypoint.new(%g,%g,%g)"):format(t or 0,v or 0,e or 0) end}) end}
ColorSequenceKeypoint={new=function(t,c) return setmetatable({},{__tostring=function() return ("ColorSequenceKeypoint.new(%g,%s)"):format(t or 0,tostring(c)) end}) end}
NumberSequence={new=function(v)
    if type(v)=="table" then
        local p={}
        for _,k in ipairs(v) do p[#p+1]=tostring(k) end
        local joined=table.concat(p,",")
        return setmetatable({},{__tostring=function() return "NumberSequence.new({"..joined.."})" end})
    end
    return setmetatable({},{__tostring=function() return "NumberSequence.new("..tostring(v)..")" end})
end}
ColorSequence={new=function(v)
    if type(v)=="table" then
        local p={}
        for _,k in ipairs(v) do p[#p+1]=tostring(k) end
        local joined=table.concat(p,",")
        return setmetatable({},{__tostring=function() return "ColorSequence.new({"..joined.."})" end})
    end
    return setmetatable({},{__tostring=function() return "ColorSequence.new("..tostring(v)..")" end})
end}

PhysicalProperties={new=function(...) local a,p={...},{} for _,v in ipairs(a) do p[#p+1]=tostring(v) end
    return setmetatable({},{__tostring=function() return "PhysicalProperties.new("..table.concat(p,",")..")" end}) end}
BrickColor={new=function(v) return setmetatable({},{__tostring=function() return 'BrickColor.new('..tostring(v)..')' end}) end,
    White=function() return BrickColor.new("White") end}

local inst_counter = 0
local LOGGING = false
local _players, _replicatedStorage, _lp, _char, _pgui

local function make_inst(class, user_new)
    inst_counter = inst_counter + 1
    local myid = inst_counter
    if LOGGING and user_new then log("INST_NEW", myid, class) end
    local props = {ClassName=class, _id=myid}
    local proxy = {}
    local mt = {}

    mt.__index = function(t, k)
        if k == "Parent"    then return props["Parent"] end
        if props[k] ~= nil  then return props[k] end
        if k == "_id"       then return myid end
        if k == "ClassName" then return class end

        if class == "TweenService" and k == "Create" then
            return function(self, inst, info, ptbl)
                local iid = (type(inst)=="table") and (pcall(function() return inst._id end) and inst._id or 0) or 0
                local pp={}
                if type(ptbl)=="table" then for pk,pv in pairs(ptbl) do pp[#pp+1]=tostring(pk).."="..tostring(pv) end end
                log("TWEEN_CREATE", iid, tostring(info), table.concat(pp,";"))
                local tw = make_inst("Tween")
                rawset(tw,"Play",function(s2) log("TWEEN_PLAY",iid) end)
                rawset(tw,"Cancel",function() end)
                return tw
            end
        end

        if k=="GetService" then return function(self,svc)
            log("GET_SERVICE",svc)
            if svc=="Players" then return _players end
            if svc=="ReplicatedStorage" then return _replicatedStorage end
            return make_inst(svc)
        end
        elseif k=="HttpGet" or k=="GetAsync" then return function(self,url,...) log("HTTP_GET",url); return "" end
        elseif k=="PostAsync" then return function(self,url,body,...) log("HTTP_POST",url,tostring(body):sub(1,80)); return "" end
        elseif k=="SendAsync" then return function(self,msg,...) log("CHAT_SEND",tostring(msg)) end
        elseif k=="WaitForChild" then return function(self,name,t2) log("WAIT_FOR_CHILD",class,name); return make_inst(name) end
        elseif k=="FindFirstChild" or k=="FindFirstChildOfClass" or k=="FindFirstChildWhichIsA" then
            return function(self,name,...) return make_inst(name) end
        elseif k=="FindFirstAncestorOfClass" or k=="FindFirstAncestor" then
            return function(self,name,...) return make_inst(name) end
        elseif k=="GetChildren" or k=="GetDescendants" then return function() return {} end
        elseif k=="GetPlayers" then return function() return {} end
        elseif k=="GetPlayingAnimationTracks" then return function() return {} end
        elseif k=="GetMouse" then return function() return make_inst("Mouse") end
        elseif k=="IsDescendantOf" or k=="IsAncestorOf" then return function() return false end
        elseif k=="IsA" then return function(self,cls) return class==cls end
        elseif k=="Connect" or k=="connect" then
            return function(self, fn)
                log("CONNECT_BEGIN", myid, class, "?")
                if type(fn)=="function" then
                    cb_depth=cb_depth+1
                    pcall(fn, make_inst("InputObject"), false)
                    cb_depth=cb_depth-1
                end
                log("CONNECT_END", myid, "?")
                return {Disconnect=function() end}
            end
        elseif k=="Wait" then return function() return make_inst(class.."_w") end
        elseif k=="Fire" or k=="FireServer" or k=="FireClient" or k=="FireAllClients" then
            return function(self,...)
                local p={}; for _,v in ipairs({...}) do p[#p+1]=tostring(v) end
                log("REMOTE_FIRE", table.unpack(p))
            end
        elseif k=="Invoke" or k=="InvokeServer" then return function(self,...) log("REMOTE_INVOKE",...); return nil end
        elseif k=="Kick" then return function(self,msg) log("KICK",tostring(msg)) end
        elseif k=="BreakJoints" then return function() log("BREAK_JOINTS",class) end
        elseif k=="SetStateEnabled" then return function(self,s,e) log("SET_STATE",tostring(s),tostring(e)) end
        elseif k=="Destroy" then return function() log("DESTROY",myid,class) end
        elseif k=="MoveTo" then return function() end
        elseif k=="Clone" then return function() return make_inst(class) end
        elseif k=="SetAttribute" or k=="RemoveAttribute" then return function() end
        elseif k=="GetAttribute" then return function() return nil end
        elseif k=="GetPropertyChangedSignal" then return function(self,p2) return make_inst("Signal_"..p2) end
        elseif k=="Play" then return function() log("PLAY",myid,class) end
        elseif k=="Stop" or k=="Pause" or k=="Remove" then return function() end
        elseif k=="LoadAnimation" then return function() return make_inst("AnimationTrack") end
        elseif k=="Lerp" then return function(self,...) return self end
        elseif k=="Disconnect" then return function() end
        elseif k=="SetCore" or k=="GetCore" or k=="SetCoreGuiEnabled" then return function() end
        elseif k=="new" then return function(...) return make_inst(tostring(...)) end
        else
            local sig = k
            return setmetatable({},{
                __index = function(s2, k2)
                    if k2=="Connect" or k2=="connect" or k2=="Once" then
                        return function(s3, fn)
                            log("CONNECT_BEGIN", myid, sig)
                            if type(fn)=="function" then
                                cb_depth=cb_depth+1
                                pcall(fn, make_inst("InputObject"), false)
                                cb_depth=cb_depth-1
                            end
                            log("CONNECT_END", myid, sig)
                            return {Disconnect=function() end}
                        end
                    elseif k2=="Wait" then return function() return make_inst(sig.."_w") end
                    end
                    return make_inst(class.."."..sig.."."..k2)
                end,
                __call=function(s2,...) log("STUB_CALL",class.."."..sig,...); return make_inst(sig.."_r") end,
                __newindex=function(s2,k2,v) rawset(s2,k2,v) end,
                __tostring=function() return class.."."..sig end,
                __len=function() return 0 end,
                __add=function(a,b) return a end,
                __concat=function(a,b) return tostring(a)..tostring(b) end,
                __eq=function(a,b) return rawequal(a,b) end,
                __lt=function() return false end, __le=function() return false end,
            })
        end
    end

    mt.__newindex = function(t, k, v)
        if k == "Parent" then
            local pid,pcls = 0,"?"
            if type(v)=="table" then
                local ok1,id1 = pcall(function() return v._id end)
                if ok1 and type(id1)=="number" then pid=id1 end
                local ok2,c2  = pcall(function() return v.ClassName end)
                if ok2 and type(c2)=="string"  then pcls=c2 end
            end
            if LOGGING then log("PROP_PARENT", myid, pid, pcls) end
            props["Parent"]=v; return
        end
        if LOGGING then
            if cb_depth==0 then log("PROP_SET", myid, k, tostring(v))
            else               log("CB_PROP_SET", myid, k, tostring(v)) end
        end
        props[k]=v
    end

    mt.__tostring = function() return "RobloxInstance<"..class..":"..myid..">" end
    mt.__call=function(t,...) log("INST_CALL",myid,class,...); return make_inst(class) end
    mt.__len=function() return 0 end
    mt.__add=function(a,b) return a end
    mt.__mul=function(a,b) return a end
    mt.__concat=function(a,b) return tostring(a)..tostring(b) end
    mt.__eq=function(a,b) return rawequal(a,b) end
    mt.__lt=function() return false end
    mt.__le=function() return false end

    setmetatable(proxy, mt)
    return proxy
end
)SBA";

static const char SANDBOX_LUA_B[] = R"SBB(
game      = make_inst("DataModel")
workspace = make_inst("Workspace")

_char              = make_inst("Model")
_lp                = make_inst("Player")
_players           = make_inst("Players")
_pgui              = make_inst("PlayerGui")
local _backpack    = make_inst("Backpack")
_replicatedStorage = make_inst("ReplicatedStorage")

_lp.Name        = "TargetPlayer"
_lp.UserId      = 12345678
_lp.Character   = _char
_lp.PlayerGui   = _pgui
_lp.Backpack    = _backpack
_lp.RespawnTime = 5
_players.LocalPlayer = _lp
_players.RespawnTime = 5
_char.Name = "TargetPlayer"

Players = _players
script  = make_inst("LocalScript")
shared  = {}
_G      = {}

task = {
    spawn  = function(fn,...) if type(fn)=="function" then pcall(fn,...) end end,
    wait   = function(t) return t or 0 end,
    delay  = function(t,fn,...) end,
    cancel = function() end,
    defer  = function(fn,...) if type(fn)=="function" then pcall(fn,...) end end,
}
wait   = function(t) return t or 0 end
spawn  = function(fn,...) if type(fn)=="function" then pcall(fn,...) end end
delay  = function(t,fn,...) end

coroutine = {
    wrap   = function(fn) return function(...) local ok,r=pcall(fn,...); return r end end,
    create = function(fn) return fn end,
    resume = function(co,...) if type(co)=="function" then return pcall(co,...) end return true end,
    yield  = function(...) return ... end,
    status = function() return "dead" end,
    isyieldable = function() return false end,
    running = function() return nil,true end,
}

getfenv  = function() return _ENV end
setfenv  = function() end
unpack   = table.unpack
newproxy = function(mt) return setmetatable({},mt and {} or nil) end
rawlen   = rawlen or function(t) return #t end

if not bit32 then
    bit32={
        bxor=function(a,b) return a~b end, band=function(a,b) return a&b end,
        bor=function(a,b) return a|b end,  bnot=function(a) return ~a end,
        rshift=function(a,b) return a>>b end, lshift=function(a,b) return a<<b end,
        arshift=function(a,b) return a>>b end,
        btest=function(...) return false end,
        rol=function(a,b) return a end, ror=function(a,b) return a end,
    }
end

Instance = {new=function(cls,parent)
    local inst = make_inst(cls, true)
    if parent then inst.Parent = parent end
    return inst
end}

LOGGING = true

local _path = arg and arg[1] or ""
local _f, _e = _real_io_open(_path, "r")
if not _f then print("ERROR: "..tostring(_e)); os.exit(1) end
local _src = _f:read("*a")
_f:close()

local _chunk, _cerr = _real_load(_src, "script")
if not _chunk then
    local compat = "getfenv=function()return _ENV end\nsetfenv=function()end\nunpack=table.unpack\n"
    _chunk, _cerr = _real_load(compat.._src, "script_compat")
end
if _chunk then
    local ok, rerr = pcall(_chunk)
    if not ok then log("RUNTIME_ERROR", tostring(rerr):sub(1,300)) end
end

print("=== SANDBOX LOG ===")
for _, e in ipairs(LOG) do print(e) end
print("=== END LOG ===")
)SBB";

static const std::string SANDBOX_LUA_STR = std::string(SANDBOX_LUA_A) + SANDBOX_LUA_B;
static const char* SANDBOX_LUA = SANDBOX_LUA_STR.c_str();

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

static std::vector<std::string> splitTab(const std::string& s, int maxParts = 99) {
    std::vector<std::string> parts;
    size_t pos = 0;
    int splits = 0;
    while (splits < maxParts - 1) {
        size_t found = s.find('\t', pos);
        if (found == std::string::npos) break;
        parts.push_back(trim(s.substr(pos, found - pos)));
        pos = found + 1;
        splits++;
    }
    parts.push_back(trim(s.substr(pos)));
    return parts;
}

static std::vector<std::string> splitPipe(const std::string& s) {
    std::vector<std::string> parts;
    std::istringstream ss(s);
    std::string part;
    while (std::getline(ss, part, '|'))
        parts.push_back(trim(part));
    return parts;
}

struct PropKV { std::string key, value; };

struct InstData {
    int id = 0;
    std::string cls;
    std::vector<PropKV> props;
    std::set<std::string> seenProps;
    int parentId = 0;
    std::string parentClass;
    std::string varName;
    bool isUserCreated = false;

    void addProp(const std::string& k, const std::string& v) {
        if (!seenProps.count(k)) {
            seenProps.insert(k);
            props.push_back({k, v});
        }
    }
    std::string getProp(const std::string& k) const {
        for (auto& p : props) if (p.key == k) return p.value;
        return "";
    }
};

static std::string toCamel(const std::string& s) {
    if (s.empty()) return "inst";
    std::string r;
    r += (char)tolower((unsigned char)s[0]);
    r += s.substr(1);
    std::string safe;
    for (char c : r) safe += (isalnum((unsigned char)c) || c == '_') ? c : '_';
    if (safe.empty() || isdigit((unsigned char)safe[0])) safe = "_" + safe;
    return safe;
}

static std::string parentExpr(int pid, const std::string& pcls,
                               const std::map<int,InstData>& insts) {
    if (pid > 0 && insts.count(pid)) return insts.at(pid).varName;
    if (pcls.find("PlayerGui")    != std::string::npos) return "lp.PlayerGui";
    if (pcls.find("StarterGui")  != std::string::npos) return "game:GetService(\"StarterGui\")";
    if (pcls.find("CoreGui")     != std::string::npos) return "game:GetService(\"CoreGui\")";
    if (pcls.find("SoundService")!= std::string::npos) return "game:GetService(\"SoundService\")";
    if (pcls.find("Lighting")    != std::string::npos) return "game:GetService(\"Lighting\")";
    if (pcls == "Model" || pcls.find("Character") != std::string::npos) return "character";
    if (pcls.find("Player")      != std::string::npos) return "lp";
    if (pcls == "DataModel")     return "game";
    if (pcls == "Workspace" || pcls.find("Workspace") != std::string::npos) return "workspace";
    return "nil";
}

static std::string instReprToExpr(const std::string& v) {
    size_t lt = v.find('<');
    size_t colon = v.find(':', lt);
    if (lt == std::string::npos || colon == std::string::npos) return "";
    std::string cls = v.substr(lt + 1, colon - lt - 1);
    if (cls == "Head")              return "head";
    if (cls == "HumanoidRootPart")  return "hrp";
    if (cls == "Humanoid")          return "humanoid";
    if (cls == "Torso")             return "torso";
    if (cls == "Model")             return "character";
    if (cls == "Player")            return "lp";
    if (cls.find("PlayerGui") != std::string::npos) return "lp.PlayerGui";
    return "character:FindFirstChild(\"" + cls + "\")";
}

static std::string quoteVal(const std::string& v) {
    if (v == "true" || v == "false") return v;
    if (!v.empty() && (isdigit((unsigned char)v[0]) || v[0] == '-')) {
        bool isNum = true;
        for (size_t i = (v[0]=='-'?1:0); i < v.size(); i++)
            if (!isdigit((unsigned char)v[i]) && v[i] != '.') { isNum = false; break; }
        if (isNum) return v;
    }
    if (v.find("RobloxInstance<") == 0) {
        std::string expr = instReprToExpr(v);
        return expr.empty() ? "nil" : expr;
    }
    if (v.find("Color3.") == 0 || v.find("UDim2.") == 0 || v.find("UDim.") == 0 ||
        v.find("Vector3.") == 0 || v.find("Vector2.") == 0 || v.find("CFrame.") == 0 ||
        v.find("TweenInfo.") == 0 || v.find("Enum.") == 0 || v.find("Number") == 0 ||
        v.find("Color") == 0 || v.find("BrickColor") == 0 || v.find("Physical") == 0)
        return v;
    std::string esc;
    esc += '"';
    for (char c : v) {
        if (c == '"') esc += "\\\"";
        else if (c == '\\') esc += "\\\\";
        else if (c == '\n') esc += "\\n";
        else esc += c;
    }
    esc += '"';
    return esc;
}

static std::string logToLua(const std::string& rawLog) {
    std::istringstream stream(rawLog);
    std::string line;
    bool inLog = false;

    std::vector<std::string> services;
    std::set<std::string> seenSvc;
    std::vector<std::string> remoteFires;
    std::string remoteEventName, remoteEventRaw;
    std::vector<std::string> httpUrls;
    int loadstringCount = 0;
    std::map<int, InstData> insts;
    std::vector<int> instOrder;
    std::set<std::string> seenWFC;

    struct TweenEntry { int instId; std::string info, props; };
    std::vector<TweenEntry> tweens;
    std::vector<std::string> chatMessages;

    struct ConnEntry { int instId; std::string signal; };
    std::vector<ConnEntry> connections;

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line == "=== SANDBOX LOG ===") { inLog = true; continue; }
        if (line == "=== END LOG ===")     { inLog = false; continue; }
        if (!inLog || line.empty()) continue;

        if (line.size() < 2 || line[0] != '[') continue;
        size_t rbr = line.find("] ");
        if (rbr == std::string::npos) continue;
        std::string tag  = line.substr(1, rbr - 1);
        std::string body = line.substr(rbr + 2);

        auto parts = splitTab(body);

        if (tag == "GET_SERVICE") {
            if (!seenSvc.count(body)) { seenSvc.insert(body); services.push_back(body); }
        } else if (tag == "REMOTE_FIRE") {
            if (!parts.empty()) remoteFires.push_back(body);
        } else if (tag == "HTTP_GET") {
            httpUrls.push_back(body);
        } else if (tag == "LOADSTRING") {
            loadstringCount++;
        } else if (tag == "INST_NEW") {
            if (parts.size() >= 2) {
                int id = std::stoi(parts[0]);
                InstData d; d.id = id; d.cls = parts[1]; d.isUserCreated = true;
                insts[id] = d;
                instOrder.push_back(id);
            }
        } else if (tag == "PROP_SET") {
            if (parts.size() >= 3) {
                int id = std::stoi(parts[0]);
                if (insts.count(id)) {
                    std::string val = parts[2];
                    for (size_t i = 3; i < parts.size(); i++) val += "\t" + parts[i];
                    insts[id].addProp(parts[1], val);
                }
            }
        } else if (tag == "PROP_PARENT") {
            if (parts.size() >= 3) {
                int id  = std::stoi(parts[0]);
                int pid = std::stoi(parts[1]);
                std::string pcls = parts[2];
                if (insts.count(id)) {
                    bool alreadyGood = !insts[id].parentClass.empty() && insts[id].parentClass != "?";
                    if (!alreadyGood) {
                        insts[id].parentId    = pid;
                        insts[id].parentClass = pcls;
                    }
                }
            }
        } else if (tag == "WAIT_FOR_CHILD") {
            if (parts.size() >= 2) {
                std::string entry = parts[0] + "->" + parts[1];
                if (!seenWFC.count(entry)) {
                    seenWFC.insert(entry);
                    if (remoteEventName.empty() && parts[0] == "ReplicatedStorage") {
                        remoteEventRaw = parts[1];
                        std::string var;
                        if (!remoteEventRaw.empty() && std::isdigit((unsigned char)remoteEventRaw[0]))
                            var = "_";
                        for (char c : remoteEventRaw)
                            var += (std::isalnum((unsigned char)c) || c == '_') ? c : '_';
                        remoteEventName = var;
                    }
                }
            }
        } else if (tag == "TWEEN_CREATE") {
            if (parts.size() >= 3) {
                TweenEntry te;
                te.instId = std::stoi(parts[0]);
                te.info   = parts[1];
                te.props  = parts[2];
                tweens.push_back(te);
            }
        } else if (tag == "CHAT_SEND") {
            chatMessages.push_back(body);
        } else if (tag == "CONNECT_BEGIN") {
            if (parts.size() >= 2) {
                ConnEntry ce;
                ce.instId = std::stoi(parts[0]);
                ce.signal = parts[1];
                connections.push_back(ce);
            }
        }
    }

    std::map<std::string, int> nameCounters;
    for (int id : instOrder) {
        if (!insts.count(id)) continue;
        auto& d = insts[id];
        std::string base = d.getProp("Name");
        if (base.empty()) base = d.cls;
        std::string camel = toCamel(base);
        nameCounters[camel]++;
        if (nameCounters[camel] == 1) {
            d.varName = camel;
        } else {
            d.varName = camel + "_" + std::to_string(nameCounters[camel] - 1);
        }
    }

    std::map<std::string, int> usedNames;
    for (int id : instOrder) {
        if (!insts.count(id)) continue;
        auto& d = insts[id];
        std::string base = d.getProp("Name");
        if (base.empty()) base = d.cls;
        std::string camel = toCamel(base);
        usedNames[camel]++;
        if (usedNames[camel] == 1) d.varName = camel;
        else d.varName = camel + "_" + std::to_string(usedNames[camel]);
    }

    std::ostringstream lua;
    lua << "-- Unobfuscated By Azazel Deobfuscator\n\n";

    if (!services.empty()) {
        for (auto& s : services)
            lua << "local " << s << " = game:GetService(\"" << s << "\")\n";
        lua << "\nlocal lp = Players.LocalPlayer\n\n";
    }

    if (!remoteFires.empty()) {
        if (remoteEventName.empty()) { remoteEventName = "remote"; remoteEventRaw = "remote"; }
        lua << "local " << remoteEventName
            << " = game:GetService(\"ReplicatedStorage\"):WaitForChild(\""
            << remoteEventRaw << "\")\n";
        for (auto& f : remoteFires) {
            auto fp = splitTab(f);
            lua << remoteEventName << ":FireServer(";
            for (size_t i = 0; i < fp.size(); i++) {
                if (i) lua << ", ";
                lua << quoteVal(fp[i]);
            }
            lua << ")\n";
        }
        lua << "\n";
    }

    if (!httpUrls.empty()) {
        for (size_t i = 0; i < httpUrls.size(); i++) {
            if (loadstringCount > 0 && i == 0)
                lua << "loadstring(game:HttpGet(\"" << httpUrls[i] << "\"))()\n";
            else
                lua << "-- game:HttpGet(\"" << httpUrls[i] << "\")\n";
        }
        lua << "\n";
    }

    if (!instOrder.empty()) {
        int firstScreenGuiId = 0, firstFrameId = 0;
        for (int id : instOrder) {
            if (!insts.count(id)) continue;
            if (!firstScreenGuiId && insts[id].cls == "ScreenGui") firstScreenGuiId = id;
            if (!firstFrameId    && insts[id].cls == "Frame")      firstFrameId = id;
        }
        for (int id : instOrder) {
            if (!insts.count(id)) continue;
            auto& d = insts[id];
            bool unresolved = (d.parentClass.empty() || d.parentClass == "?");
            if (!unresolved) continue;
            if (d.cls == "Sound") {
                if (firstFrameId)          { d.parentId = firstFrameId;     d.parentClass = "Frame"; }
                else if (firstScreenGuiId) { d.parentId = firstScreenGuiId; d.parentClass = "ScreenGui"; }
            } else if (d.cls == "BillboardGui") {
                d.parentId = 0; d.parentClass = "Model";
            }
        }

        for (int id : instOrder) {
            if (!insts.count(id)) continue;
            auto& d = insts[id];
            lua << "local " << d.varName << " = Instance.new(\"" << d.cls << "\")\n";
        }
        lua << "\n";
        for (int id : instOrder) {
            if (!insts.count(id)) continue;
            auto& d = insts[id];
            bool any = false;
            for (auto& kv : d.props) {
                if (kv.key == "ClassName") continue;
                lua << d.varName << "." << kv.key << " = " << quoteVal(kv.value) << "\n";
                any = true;
            }
            for (auto& tw : tweens) {
                if (tw.instId == id) {
                    lua << "TweenService:Create(" << d.varName << ", "
                        << tw.info << ", {" << tw.props << "}):Play()\n";
                }
            }
            if (any) lua << "\n";
        }
        lua << "-- parent assignments\n";
        for (int id : instOrder) {
            if (!insts.count(id)) continue;
            auto& d = insts[id];
            if (d.parentId != 0 || !d.parentClass.empty()) {
                std::string pexpr = parentExpr(d.parentId, d.parentClass, insts);
                lua << d.varName << ".Parent = " << pexpr << "\n";
            }
        }
        lua << "\n";
    }

    if (!connections.empty()) {
        lua << "-- event connections\n";
        for (auto& ce : connections) {
            std::string instName = insts.count(ce.instId) ? insts.at(ce.instId).varName : "inst_" + std::to_string(ce.instId);
            lua << "-- " << instName << "." << ce.signal << ":Connect(function() end)\n";
        }
        lua << "\n";
    }

    if (!chatMessages.empty()) {
        lua << "task.wait(2)\npcall(function()\n";
        lua << "    local general = TextChatService.TextChannels:WaitForChild(\"RBXGeneral\", 5)\n";
        for (auto& msg : chatMessages)
            lua << "    general:SendAsync(" << quoteVal(msg) << ")\n";
        lua << "end)\n";
    }

    if (services.empty() && remoteFires.empty() && httpUrls.empty() && instOrder.empty())
        lua << "-- sandbox captured no events (script may require Roblox executor)\n";

    return lua.str();
}

SandboxResult runSandboxDeobfuscate(const std::string& source) {
    SandboxResult result;

    result.luaExePath = findLuaExe();
    if (result.luaExePath.empty()) {
        result.error = "Lua 5.4 not found. Install from https://www.lua.org/download.html";
        return result;
    }

    char tempDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);

    std::string sandboxPath = std::string(tempDir) + "azazel_sandbox.lua";
    std::string scriptPath  = std::string(tempDir) + "azazel_input.lua";

    {
        HANDLE h = CreateFileA(sandboxPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) { result.error = "Cannot write sandbox"; return result; }
        DWORD written;
        WriteFile(h, SANDBOX_LUA, (DWORD)strlen(SANDBOX_LUA), &written, nullptr);
        CloseHandle(h);
    }
    {
        HANDLE h = CreateFileA(scriptPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) { result.error = "Cannot write input"; return result; }
        DWORD written;
        WriteFile(h, source.data(), (DWORD)source.size(), &written, nullptr);
        CloseHandle(h);
    }

    result.rawLog = runCapture(result.luaExePath, sandboxPath, scriptPath);

    if (result.rawLog.find("=== SANDBOX LOG ===") == std::string::npos) {
        result.error = "Sandbox produced no log.\n" + result.rawLog.substr(0, 500);
        return result;
    }

    result.output  = logToLua(result.rawLog);
    result.success = true;
    return result;
}
