
local TeamAndScore = 
{
	TeamMembers = {};
	SpawnTicket = {};
	Properties = 
	{
		Score = {default = EntityId()},
		TeamPanel = {default = EntityId()},
		Team = {default = ""},
	},
}

function TeamAndScore:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()
	self:UpdateScore()
end

function TeamAndScore:OnActivate (gameMode)
	self.tickBusHandler = TickBus.Connect(self);
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function TeamAndScore:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function TeamAndScore:Init()
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)	
	end

	if #self.TeamMembers == 0 then
		self.TeamMembers = FilterArray( GetAllUnits(), function (unit)
			return unit:GetTeamId() == GameManagerRequestBus.Broadcast.GetTeamIdByName(self.Properties.Team) and HasTag(unit,"hero")
		end);
		self:GetTeamMembers()
		self:SetInitScore()
	end
end

function TeamAndScore:SetInitScore()
	UiTextBus.Event.SetText(self.Properties.Score, GetGameManager():GetValue(tostring(self.Properties.Team) .. "_score"))
end

function TeamAndScore:UpdateScore()
	local score = GetGameManager():GetValue(tostring(self.Properties.Team) .. "_score")
	if tostring(score) == UiTextBus.Event.GetText(self.Properties.Score) then
		return
	end	
	if score ~= UiTextBus.Event.GetText(self.Properties.Score) then
		UiTextBus.Event.SetText(self.Properties.Score, score)
	end
end

function TeamAndScore:GetTeamMembers()
	if self.Properties.Team ~= "" then
		local team = self.Properties.Team
		local teamId = GameManagerRequestBus.Broadcast.GetTeamIdByName(team)
		
		--get all the heroes of the team
		self.TeamMembers = FilterArray( GetAllUnits(), function (unit)
			return unit:GetTeamId() == teamId and HasTag(unit,"hero")
		end);
		
		if #self.TeamMembers ~= 0 then
			self:SetTeamMemberPanel(self.TeamMembers)
		end
	end
end

function TeamAndScore:SetTeamMemberPanel(teamHeroes)
	
	for i=1,#teamHeroes do
		self.SpawnTicket[i] = UiSpawnerBus.Event.Spawn(self.Properties.TeamPanel)
	end
end

return TeamAndScore