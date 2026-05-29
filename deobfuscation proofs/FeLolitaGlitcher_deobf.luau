-- deobfuscated by lifenock
-- Unobfuscated By Azazel Deobfuscator

local UserInputService  = game:GetService("UserInputService")
local Players           = game:GetService("Players")
local RunService        = game:GetService("RunService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local TweenService      = game:GetService("TweenService")
local TextChatService   = game:GetService("TextChatService")

local lp        = Players.LocalPlayer
local character = lp.Character or lp.CharacterAdded:Wait()
local humanoid  = character:WaitForChild("Humanoid")
local torso     = character:WaitForChild("Torso")
local hrp       = character:WaitForChild("HumanoidRootPart")
local head      = character:WaitForChild("Head")

local reanimateSource = game:HttpGet("https://raw.githubusercontent.com/oldhacfard/The-script/refs/heads/main/Reanimate.luau")

local function runReanimate()
    local fn, err = loadstring(reanimateSource)
    if fn then
        local ok, runErr = pcall(fn)
        if not ok then warn("[FeLolitaGlitcher] reanimate error: " .. tostring(runErr)) end
    else
        warn("[FeLolitaGlitcher] loadstring failed: " .. tostring(err))
    end
end

local server = ReplicatedStorage:WaitForChild("01_server")
server:FireServer("cmd", "-rs")
server:FireServer("cmd", "-r6")
server:FireServer("cmd", "-gh",
    5316479641, 5316539421, 5699795428,
    5316549755, 5268602207, 5268555719,
    82742704984803)
server:FireServer("cmd", "-sh")
server:FireServer("cmd", "-net")

runReanimate()

for _, stateType in pairs({
    Enum.HumanoidStateType.GettingUp,
    Enum.HumanoidStateType.FallingDown,
    Enum.HumanoidStateType.Ragdoll,
    Enum.HumanoidStateType.PlatformStanding,
}) do
    pcall(function()
        humanoid:SetStateEnabled(stateType, false)
    end)
end

local animScript = character:FindFirstChild("Animate")
if animScript then animScript.Disabled = true end

for _, track in pairs(humanoid:GetPlayingAnimationTracks()) do
    track:Stop(0)
end

local joints = {}
local function grabJoints()
    local t = character:WaitForChild("Torso", 3)
    local h = character:WaitForChild("HumanoidRootPart", 3)
    if not (t and h) then return end
    for _, name in ipairs({"Neck", "Right Shoulder", "Left Shoulder", "Right Hip", "Left Hip"}) do
        joints[name] = t:WaitForChild(name, 3)
    end
    joints["RootJoint"] = h:WaitForChild("RootJoint", 3)
end
grabJoints()

local screenGui = Instance.new("ScreenGui")
local mainFrame = Instance.new("Frame")

TweenService:Create(mainFrame,
    TweenInfo.new(0.45, Enum.EasingStyle.Back, Enum.EasingDirection.Out),
    { Position = UDim2.new(0.5, -110, 0.5, -85) }
):Play()

local launchSound = Instance.new("Sound")
launchSound:Play()

local imageLabel   = Instance.new("ImageLabel")
local imageLabel_2 = Instance.new("ImageLabel")
local imageLabel_3 = Instance.new("ImageLabel")
local imageLabel_4 = Instance.new("ImageLabel")
local textLabel    = Instance.new("TextLabel")
local uIGradient   = Instance.new("UIGradient")
local imageLabel_5 = Instance.new("ImageLabel")
local innerFrame   = Instance.new("Frame")
local uICorner     = Instance.new("UICorner")
local textLabel_2  = Instance.new("TextLabel")
local textButton   = Instance.new("TextButton")
local textLabel_3  = Instance.new("TextLabel")
local textButton_2 = Instance.new("TextButton")
local textLabel_4  = Instance.new("TextLabel")
local textButton_3 = Instance.new("TextButton")
local textLabel_5  = Instance.new("TextLabel")
local billboardGui   = Instance.new("BillboardGui")
local billboardLabel = Instance.new("TextLabel")
local uIGradient_2   = Instance.new("UIGradient")

-- UserInputService.InputBegan:Connect(function() end)
-- textButton.MouseButton1Click:Connect(function() end)
-- textButton_2.MouseButton1Click:Connect(function() end)
-- textButton_3.MouseButton1Click:Connect(function() end)
-- RunService.Stepped:Connect(function() end)
-- RunService.Heartbeat:Connect(function() end)

humanoid.Died:Connect(function()
    task.wait(0.5)
    local newChar = lp.CharacterAdded:Wait()
    character = newChar
    humanoid  = newChar:WaitForChild("Humanoid", 5)
    hrp       = newChar:WaitForChild("HumanoidRootPart", 5)
    torso     = newChar:WaitForChild("Torso", 5)
    head      = newChar:WaitForChild("Head", 5)
    runReanimate()
    grabJoints()
end)

task.wait(2)
pcall(function()
    local general = TextChatService.TextChannels:WaitForChild("RBXGeneral", 5)
    if general then
        general:SendAsync("deobfuscated by 4tmv lol")
        task.wait(0.5)
        general:SendAsync("rip harkinian")
    end
end)
