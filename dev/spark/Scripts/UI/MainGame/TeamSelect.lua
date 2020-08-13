require "scripts.library.ui_events"
require "gamemode.Spark.modules.team_select"

local TeamSelect = 
{

	Properties =
	{
		TimeText = {default = EntityId()},
		Teams = {default = EntityId()},
	},	
}

function TeamSelect:OnActivate()
	self.tickBusHandler = TickBus.Connect(self)
end

function TeamSelect:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.tickBusHandler = nil
	self.canvasNotificationHandler:Disconnect()
	self.canvasNotificationHandler = nil
end

function TeamSelect:UpdatePlayerList()
	local playerList = GetGameManager():GetPlayerList()

    self.leftPlayers = { }
    self.rightPlayers = { }

    for _,player in ipairs(playerList) do
        local team = player:GetTeamId()
        Debug.Log('This is a player? ' .. player:GetPlayerId() .. ' team  ' .. team)
        if GetTeamName(team) == "left" then
            table.insert(self.leftPlayers, player)
        elseif GetTeamName(team) == "right" then
            table.insert(self.rightPlayers, player)
        end
    end
    self:AssignTeamPlayers(self.leftPlayers, 0)
    self:AssignTeamPlayers(self.rightPlayers, 1)
end

function TeamSelect:AssignTeamPlayers(players, teamIndex)
	if #players > 0 then
		local team = UiElementBus.Event.GetChild(self.Properties.Teams, teamIndex)
		local teamMembers = UiElementBus.Event.FindChildByName(team, "TeamMembers")
		for i = 1, #players do
			local PlayerSlot = UiElementBus.Event.GetChild(teamMembers, i-1)
			local icon = UiElementBus.Event.FindDescendantByName(PlayerSlot, "Icon")
			local name = UiElementBus.Event.FindDescendantByName(PlayerSlot, "Name")
			
			UiTextBus.Event.SetText(UiElementBus.Event.GetChild(name,0), tostring("Player " .. tostring(i)))
		end
	end
end

function TeamSelect:OnTick(deltaTime, timePoint)
	local time = GetGameManager():GetValue("TeamSelectTime")
	if self.CurrentUiTime ~= time then
		self.CurrentUiTime = time
		self:SetGameTime(self.CurrentUiTime)
	end	

	if not self.CanvasEntityId then
	   	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
		if self.CanvasEntityId and not self.canvasNotificationHandler then
		    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
			self:UpdatePlayerList()
		end
	end

end

function TeamSelect:SetGameTime(CurrentTime)
	UiTextBus.Event.SetText(self.Properties.TimeText, CurrentTime)
end

function TeamSelect:ShuffleTeams()
	local playerList = GetGameManager():GetPlayerList()

    local leftPlayers = { }
    local rightPlayers = { }

    for _,player in ipairs(playerList) do
        local team = math.random(0,1)
        Debug.Log('This is a player? ' .. player:GetPlayerId() .. ' team  ' .. team)
        if GetTeamName(team) == "left" then
            table.insert(leftPlayers, player)
        elseif GetTeamName(team) == "right" then
            table.insert(rightPlayers, player)
        end
    end
    self:AssignTeamPlayers(leftPlayers, 0)
    self:AssignTeamPlayers(rightPlayers, 1)
end

-- In Game HUD Actions
function TeamSelect:OnAction(entityId, actionName)

	SendUIEvent(actionName, "")
	self:UpdatePlayerList()

	-- local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	
	-- if actionName == "LockInAndStart" then
	-- 	TeamSelection:LockedInTimer()
	-- end
	
	-- if actionName == "AddBot" then
	-- 	GetGameManager():CreateFakePlayer()
	-- end
	
	-- if actionName == "ShuffleTeams" then
	-- 	self:ShuffleTeams()
	-- end
	
	-- if actionName == "KickFromTeam" then
	
	-- end
	
	-- if actionName == "TestMode" then
	-- 	UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)
	-- 	GameManagerRequestBus.Broadcast.FinishGameConfigure()
	-- 	SendUIEvent("SelectHero", "astromage");
	-- end
end

return TeamSelect;
