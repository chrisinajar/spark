require "scripts.variableholder"

local ScoreBoardPlayer = 
{
	Unit = nil;
	Properties =
	{
		Image = {default = EntityId()},
		SteamName = {default = EntityId()},
		HeroName = {default = EntityId()},
		HeroIcon = {default = EntityId()},
		Level = {default = EntityId()},
		Gold = {default = EntityId()},
		Kills = {default = EntityId()},
		Deaths = {default = EntityId()},
		Assists = {default = EntityId()},
		Ult = {default = EntityId()},
		Mute = {default = EntityId()},
	},	
}

function ScoreBoardPlayer:Init()
	self.canvasNotificationHandler = nil

	self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)	
	if( self.canvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)
	end
	
	local parent = UiElementBus.Event.GetParent(self.entityId)
	local childIndex = UiElementBus.Event.GetIndexOfChildByEntityId(parent, self.entityId)
	local teamParent = UiElementBus.Event.GetParent(parent)
	local teamNamePanel = UiElementBus.Event.FindChildByName(teamParent, "TeamName+Score")
	local teamName = UiElementBus.Event.FindChildByName(teamNamePanel, "TeamName")
	local teamTextRef =  UiElementBus.Event.FindChildByName(teamName, "Text")
	
	--Debug.Log("UNIT TEAM IS : " .. tostring(UiTextBus.Event.GetText(teamTextRef)) .. "IN SCORE BOARD PLAYER !!")
	
	self.TeamMembers = FilterArray( GetAllUnits(), function (unit)
		return unit:GetTeamId() == GameManagerRequestBus.Broadcast.GetTeamIdByName(tostring(UiTextBus.Event.GetText(teamTextRef))) and HasTag(unit,"hero")
	end);
	self.Unit = self.TeamMembers[childIndex + 1]
	--Debug.Log("UNIT IS SET TO : " .. tostring(self.Unit) .. "IN SCORE BOARD PLAYER !!")
	self.VariableHandler = VariableHolderNotificationBus.Connect(self, GetId(self.Unit))
	self:SetInitValues(self.Unit)
	
end

function ScoreBoardPlayer:OnActivate()
	CreateTimer(function() 
		self:Init() 
	end,0.1);
end

function ScoreBoardPlayer:OnDeactivate()
	self.canvasNotificationHandler:Disconnect()
end

function ScoreBoardPlayer:SetInitValues(unit)
	local heroName = LocalizationRequestBus.Broadcast.LocalizeString("@npc_spark_hero_" .. tostring(UnitRequestBus.Event.GetName(GetId(unit)) .. "_name"))
	UiTextBus.Event.SetText(self.Properties.HeroName, tostring(heroName))
	--UiTextBus.Event.SetText(self.Properties.SteamName, tostring(unit:GetType())
	UiTextBus.Event.SetText(self.Properties.Level, VariableManagerRequestBus.Broadcast.GetValue(VariableId(GetId(unit), "level")))
	UiTextBus.Event.SetText(self.Properties.Gold, VariableManagerRequestBus.Broadcast.GetValue(VariableId(GetId(unit), "gold")))
	UiTextBus.Event.SetText(self.Properties.Kills, "0")
	UiTextBus.Event.SetText(self.Properties.Deaths, "0")
	UiTextBus.Event.SetText(self.Properties.Assists, "0")
	UiImageBus.Event.SetColor(self.Properties.Ult, Color(255/255, 255/255, 255/255))
	UiImageBus.Event.SetSpritePathname(self.Properties.Mute, "textures/core.dds")
	UiImageBus.Event.SetSpritePathname(self.Properties.HeroIcon, "textures/core.dds")
	UiImageBus.Event.SetSpritePathname(self.Properties.Image, "textures/core.dds")
	
end

function ScoreBoardPlayer:OnSetValue(id, value)
	if MapRequestBus.Broadcast.IsUnitVisible(GetId(self.Unit)) then
		if tostring(id.variableId) == "kills" then
			UiTextBus.Event.SetText(self.Properties.Kills, tostring(tonumber(math.floor(value))))
		end
		if tostring(id.variableId) == "deaths" then
			UiTextBus.Event.SetText(self.Properties.Deaths, tostring(tonumber(math.floor(value))))
		end
		if tostring(id.variableId) == "assists" then
			UiTextBus.Event.SetText(self.Properties.Assists, tostring(tonumber(math.floor(value))))
		end
		if tostring(id.variableId) == "gold" then
			UiTextBus.Event.SetText(self.Properties.Gold, tostring(tonumber(math.floor(value))))
		end
		if tostring(id.variableId) == "level" then
			UiTextBus.Event.SetText(self.Properties.Level, tostring(tonumber(math.floor(value))))
		end
	end
end

function ScoreBoardPlayer:UpdateScore()
	local score = GetGameManager():GetValue(tostring(self.Properties.Team) .. "_score")
	if tostring(score) == UiTextBus.Event.GetText(self.Properties.Score) then
		return
	end	
	if score ~= UiTextBus.Event.GetText(self.Properties.Score) then
		UiTextBus.Event.SetText(self.Properties.Score, score)
	end
end

function ScoreBoardPlayer:OnAction(entityId, actionName) 
	if actionName == "MutePlayer" then
		local PlayerParent = UiElementBus.Event.GetParent(entityId)
	end
end


return ScoreBoardPlayer;