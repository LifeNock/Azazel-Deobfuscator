-- Unobfuscated By Azazel Deobfuscator

local Players           = game:GetService("Players")
local RunService        = game:GetService("RunService")
local UserInputService  = game:GetService("UserInputService")
local TweenService      = game:GetService("TweenService")
local TextChatService   = game:GetService("TextChatService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")

local lp = Players.LocalPlayer

local server = ReplicatedStorage:WaitForChild("01_server")
server:FireServer("cmd", "-rs")
server:FireServer("cmd", "-r6")
server:FireServer("cmd", "-gh",
    5316479641, 5316539421, 5699795428,
    5316549755, 5268602207, 5268555719, 82742704984803)
server:FireServer("cmd", "-sh")
server:FireServer("cmd", "-net")

local REANIMATE_URL = "https://raw.githubusercontent.com/oldhacfard/The-script/refs/heads/main/Reanimate.luau"
local reanimateSource = game:HttpGet(REANIMATE_URL)

local function runReanimate()
    local fn, err = loadstring(reanimateSource)
    if fn then
        local ok, runErr = pcall(fn)
        if not ok then
            warn("[FeLolitaGlitcher] reanimate error: " .. tostring(runErr))
        end
    else
        warn("[FeLolitaGlitcher] loadstring failed: " .. tostring(err))
    end
end

runReanimate()

local character = lp.Character or lp.CharacterAdded:Wait()
local humanoid  = character:WaitForChild("Humanoid")
local hrp       = character:WaitForChild("HumanoidRootPart")
local torso     = character:WaitForChild("Torso")
local head      = character:WaitForChild("Head")

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
if animScript then
    animScript.Disabled = true
end

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
screenGui.Name            = "FeLolitaGlitcher"
screenGui.ResetOnSpawn    = false
screenGui.ZIndexBehavior  = Enum.ZIndexBehavior.Sibling
screenGui.IgnoreGuiInset  = true
screenGui.Parent          = lp.PlayerGui

local mainFrame = Instance.new("Frame")
mainFrame.Name             = "MainFrame"
mainFrame.Size             = UDim2.new(0, 220, 0, 170)
mainFrame.Position         = UDim2.new(0.5, -110, 1, 10)
mainFrame.BackgroundColor3 = Color3.fromRGB(15, 10, 25)
mainFrame.BorderSizePixel  = 0
mainFrame.Parent           = screenGui

local corner = Instance.new("UICorner")
corner.CornerRadius = UDim.new(0, 10)
corner.Parent       = mainFrame

local gradient = Instance.new("UIGradient")
gradient.Color = ColorSequence.new({
    ColorSequenceKeypoint.new(0,   Color3.fromRGB(40, 0, 70)),
    ColorSequenceKeypoint.new(0.5, Color3.fromRGB(15, 5, 40)),
    ColorSequenceKeypoint.new(1,   Color3.fromRGB(0,  0, 25)),
})
gradient.Rotation = 90
gradient.Parent   = mainFrame

local header = Instance.new("Frame")
header.Size             = UDim2.new(1, 0, 0, 32)
header.BackgroundColor3 = Color3.fromRGB(80, 0, 130)
header.BorderSizePixel  = 0
header.Parent           = mainFrame

local headerCorner = Instance.new("UICorner")
headerCorner.CornerRadius = UDim.new(0, 10)
headerCorner.Parent       = header

local imgPositions = {
    UDim2.new(0, 4,   0, 6),
    UDim2.new(0, 22,  0, 6),
    UDim2.new(1, -22, 0, 6),
    UDim2.new(1, -40, 0, 6),
}
for _, pos in ipairs(imgPositions) do
    local img = Instance.new("ImageLabel")
    img.Size                   = UDim2.new(0, 18, 0, 18)
    img.Position               = pos
    img.BackgroundTransparency = 1
    img.Image                  = "rbxassetid://7072725342"
    img.ImageColor3            = Color3.fromRGB(200, 100, 255)
    img.Parent                 = header
end

local titleLabel = Instance.new("TextLabel")
titleLabel.Size               = UDim2.new(1, -90, 1, 0)
titleLabel.Position           = UDim2.new(0, 44, 0, 0)
titleLabel.BackgroundTransparency = 1
titleLabel.Text               = "FeLolitaGlitcher"
titleLabel.TextColor3         = Color3.fromRGB(255, 255, 255)
titleLabel.TextSize           = 14
titleLabel.Font               = Enum.Font.GothamBold
titleLabel.TextXAlignment     = Enum.TextXAlignment.Left
titleLabel.Parent             = header

local statusLabel = Instance.new("TextLabel")
statusLabel.Name              = "Status"
statusLabel.Size              = UDim2.new(1, -10, 0, 18)
statusLabel.Position          = UDim2.new(0, 5, 0, 34)
statusLabel.BackgroundTransparency = 1
statusLabel.Text              = "✓ Reanimated"
statusLabel.TextColor3        = Color3.fromRGB(140, 255, 140)
statusLabel.TextSize          = 11
statusLabel.Font              = Enum.Font.Gotham
statusLabel.Parent            = mainFrame

local creditLabel = Instance.new("TextLabel")
creditLabel.Size              = UDim2.new(1, -10, 0, 14)
creditLabel.Position          = UDim2.new(0, 5, 1, -16)
creditLabel.BackgroundTransparency = 1
creditLabel.Text              = "by Exire | leaked by C00l_Ch4os"
creditLabel.TextColor3        = Color3.fromRGB(100, 80, 120)
creditLabel.TextSize          = 9
creditLabel.Font               = Enum.Font.Gotham
creditLabel.Parent             = mainFrame

local BUTTON_DEFS = {
    { name = "Reanimate", color = Color3.fromRGB(120, 0,  180) },
    { name = "Noclip",    color = Color3.fromRGB(0,   80, 160) },
    { name = "Fling",     color = Color3.fromRGB(160, 0,  60)  },
}
local buttons = {}
for i, def in ipairs(BUTTON_DEFS) do
    local btn = Instance.new("TextButton")
    btn.Name             = def.name
    btn.Size             = UDim2.new(0, 195, 0, 28)
    btn.Position         = UDim2.new(0, 12, 0, 54 + (i - 1) * 34)
    btn.BackgroundColor3 = def.color
    btn.BorderSizePixel  = 0
    btn.Text             = def.name
    btn.TextColor3       = Color3.fromRGB(255, 255, 255)
    btn.TextSize         = 13
    btn.Font             = Enum.Font.GothamSemibold
    btn.AutoButtonColor  = true
    local c2 = Instance.new("UICorner")
    c2.CornerRadius = UDim.new(0, 6)
    c2.Parent       = btn
    btn.Parent      = mainFrame
    buttons[def.name] = btn
end

local billboard = Instance.new("BillboardGui")
billboard.Name        = "GlitcherTag"
billboard.Size        = UDim2.new(0, 140, 0, 26)
billboard.StudsOffset = Vector3.new(0, 3.5, 0)
billboard.AlwaysOnTop = true
billboard.Adornee     = hrp
billboard.Parent      = character

local billboardLabel = Instance.new("TextLabel")
billboardLabel.Size                   = UDim2.new(1, 0, 1, 0)
billboardLabel.BackgroundTransparency = 1
billboardLabel.Text                   = "lolita skid thing"
billboardLabel.TextColor3             = Color3.fromRGB(255, 80, 255)
billboardLabel.TextStrokeTransparency = 0.5
billboardLabel.TextStrokeColor3       = Color3.fromRGB(80, 0, 100)
billboardLabel.TextSize               = 13
billboardLabel.Font                   = Enum.Font.GothamBold
billboardLabel.Parent                 = billboard

TweenService:Create(mainFrame,
    TweenInfo.new(0.45, Enum.EasingStyle.Back, Enum.EasingDirection.Out),
    { Position = UDim2.new(0.5, -110, 0.5, -85) }
):Play()

local launchSound = Instance.new("Sound")
launchSound.SoundId = "rbxassetid://6042053626"
launchSound.Volume  = 0.4
launchSound.Parent  = mainFrame
launchSound:Play()

local dragging  = false
local dragStart = Vector2.new()
local frameStart

header.InputBegan:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging   = true
        dragStart  = Vector2.new(input.Position.X, input.Position.Y)
        frameStart = mainFrame.Position
    end
end)

header.InputEnded:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.MouseButton1 then
        dragging = false
    end
end)

UserInputService.InputChanged:Connect(function(input)
    if not dragging then return end
    if input.UserInputType ~= Enum.UserInputType.MouseMovement then return end
    local delta = Vector2.new(input.Position.X, input.Position.Y) - dragStart
    mainFrame.Position = UDim2.new(
        frameStart.X.Scale,
        frameStart.X.Offset + delta.X,
        frameStart.Y.Scale,
        frameStart.Y.Offset + delta.Y
    )
end)

local noclipEnabled = false
local reanimating   = false

buttons["Reanimate"].MouseButton1Click:Connect(function()
    if reanimating then return end
    reanimating = true
    statusLabel.Text       = "Reanimating..."
    statusLabel.TextColor3 = Color3.fromRGB(255, 220, 80)

    character = lp.Character or lp.CharacterAdded:Wait()
    humanoid  = character:WaitForChild("Humanoid", 5)
    hrp       = character:WaitForChild("HumanoidRootPart", 5)

    runReanimate()

    for _, stateType in pairs({
        Enum.HumanoidStateType.GettingUp,
        Enum.HumanoidStateType.FallingDown,
        Enum.HumanoidStateType.Ragdoll,
    }) do
        pcall(humanoid.SetStateEnabled, humanoid, stateType, false)
    end

    local anim = character:FindFirstChild("Animate")
    if anim then anim.Disabled = true end

    for _, track in pairs(humanoid:GetPlayingAnimationTracks()) do
        track:Stop(0)
    end

    grabJoints()

    statusLabel.Text       = "✓ Reanimated"
    statusLabel.TextColor3 = Color3.fromRGB(140, 255, 140)
    reanimating = false
end)

buttons["Noclip"].MouseButton1Click:Connect(function()
    noclipEnabled = not noclipEnabled
    if noclipEnabled then
        buttons["Noclip"].BackgroundColor3 = Color3.fromRGB(0, 160, 60)
        buttons["Noclip"].Text             = "Noclip ON"
        statusLabel.Text                   = "Noclip enabled"
        statusLabel.TextColor3             = Color3.fromRGB(100, 255, 100)
    else
        buttons["Noclip"].BackgroundColor3 = Color3.fromRGB(0, 80, 160)
        buttons["Noclip"].Text             = "Noclip"
        statusLabel.Text                   = "Noclip off"
        statusLabel.TextColor3             = Color3.fromRGB(180, 180, 180)
    end
end)

buttons["Fling"].MouseButton1Click:Connect(function()
    local nearest, nearestDist = nil, math.huge

    for _, player in pairs(Players:GetPlayers()) do
        if player ~= lp and player.Character then
            local targetHRP = player.Character:FindFirstChild("HumanoidRootPart")
            if targetHRP then
                local dist = (targetHRP.Position - hrp.Position).Magnitude
                if dist < nearestDist then
                    nearest     = player
                    nearestDist = dist
                end
            end
        end
    end

    if nearest and nearest.Character then
        statusLabel.Text       = "Flinging " .. nearest.Name
        statusLabel.TextColor3 = Color3.fromRGB(255, 100, 100)

        local targetChar = nearest.Character
        local targetHRP2 = targetChar:FindFirstChild("HumanoidRootPart")
        if targetHRP2 then
            hrp.CFrame = CFrame.new(targetHRP2.Position + Vector3.new(0, 2, 0))
        end

        task.wait(0.1)
        pcall(function()
            targetChar:BreakJoints()
        end)

        task.wait(1)
        statusLabel.Text       = "✓ Reanimated"
        statusLabel.TextColor3 = Color3.fromRGB(140, 255, 140)
    else
        statusLabel.Text       = "No target in range"
        statusLabel.TextColor3 = Color3.fromRGB(255, 180, 80)
    end
end)

UserInputService.InputBegan:Connect(function(input, gameProcessed)
    if gameProcessed then return end
    if input.KeyCode == Enum.KeyCode.F then
        buttons["Fling"].MouseButton1Click:Fire()
    elseif input.KeyCode == Enum.KeyCode.N then
        buttons["Noclip"].MouseButton1Click:Fire()
    elseif input.KeyCode == Enum.KeyCode.R then
        buttons["Reanimate"].MouseButton1Click:Fire()
    end
end)

RunService.Stepped:Connect(function()
    if not noclipEnabled then return end
    if not character or not character.Parent then return end
    for _, part in pairs(character:GetDescendants()) do
        if part:IsA("BasePart") then
            part.CanCollide = false
        end
    end
end)

RunService.Heartbeat:Connect(function()
    if not screenGui.Parent then
        screenGui:Destroy()
    end
end)

humanoid.Died:Connect(function()
    statusLabel.Text       = "Died — reanimating..."
    statusLabel.TextColor3 = Color3.fromRGB(255, 80, 80)
    task.wait(0.5)
    local newChar = lp.CharacterAdded:Wait()
    character = newChar
    humanoid  = newChar:WaitForChild("Humanoid", 5)
    hrp       = newChar:WaitForChild("HumanoidRootPart", 5)
    torso     = newChar:WaitForChild("Torso", 5)
    head      = newChar:WaitForChild("Head", 5)
    runReanimate()
    grabJoints()
    billboard.Adornee      = hrp
    statusLabel.Text       = "✓ Reanimated"
    statusLabel.TextColor3 = Color3.fromRGB(140, 255, 140)
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
