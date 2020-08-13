local ScoreBoard = 
{
	TeamMembers = {};
	SpawnTicket = {};
	Properties =
	{
		TeamName = {default = EntityId()},
		Score = {default = EntityId()},
		TeamPanel = {default = EntityId()},
		Team = {default = ""},
	},	
}

function ScoreBoard:Init()
	self.canvasNotificationHandler = nil

	self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)	
	if( self.canvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)
	end
	self:GetTeamMembers()
	self:SetInitScore()
end

function ScoreBoard:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()
	self:UpdateScore()
end

function ScoreBoard:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function ScoreBoard:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function ScoreBoard:SetInitScore()
	UiTextBus.Event.SetText(self.Properties.Score, GetGameManager():GetValue(tostring(self.Properties.Team) .. "_score"))
	UiTextBus.Event.SetText(self.Properties.TeamName, tostring(self.Properties.Team))
end

function ScoreBoard:UpdateScore()
	local score = GetGameManager():GetValue(tostring(self.Properties.Team) .. "_score")
	if tostring(score) == UiTextBus.Event.GetText(self.Properties.Score) then
		return
	end	
	if score ~= UiTextBus.Event.GetText(self.Properties.Score) then
		UiTextBus.Event.SetText(self.Properties.Score, score)
	end
end

function ScoreBoard:GetTeamMembers()
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

function ScoreBoard:SetTeamMemberPanel(teamHeroes)
	self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.TeamPanel)
	for i=1,#teamHeroes do
		self.SpawnTicket[i] = UiSpawnerBus.Event.Spawn(self.Properties.TeamPanel)
	end
end

function ScoreBoard:OnEntitySpawned(ticket, id)

end

function ScoreBoard:OnSpawnEnd(ticket)

end

function ScoreBoard:OnSpawnFailed(ticket)

end

return ScoreBoard;