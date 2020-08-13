
ScoreLimit = CreateModule("ScoreLimit")

local OnLimitChangedEvent = Event()
local OnScoreChangedEvent = Event()
ScoreLimit.onLimitChanged = OnLimitChangedEvent.listen
ScoreLimit.onScoreChanged = OnScoreChangedEvent.listen


function ScoreLimit:Init()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("score_limit", 40)
	self.gameManager:RegisterVariable("left_score", 0)
	self.gameManager:RegisterVariable("right_score", 0)
	self.hasGameEnded = false
	self.unitsHandler = UnitsNotificationBus.Connect(self);
end

function ScoreLimit:OnUnitKilled(unitId, damage)
	if not damage.source or not damage.target then
		return
	end
	if UnitRequestBus.Event.GetTeamId(damage.source) ~= UnitRequestBus.Event.GetTeamId(unitId) and UnitRequestBus.Event.GetTeamId(damage.source) ~= GetTeamIdByName("neutrals") and UnitRequestBus.Event.GetTeamId(damage.target) ~= GetTeamIdByName("neutrals") then
		self:AddPoints(UnitRequestBus.Event.GetTeamId(damage.source), 1)
	end
end

function ScoreLimit:GetLimit()
	return self.gameManager:GetValue("score_limit")
end

function ScoreLimit:SetLimit(newLimit)
	self.gameManager:SetValue("score_limit", newLimit)
	OnLimitChangedEvent.broadcast(newLimit)
end

function ScoreLimit:GetPoints(team)
	if GetTeamName(team) == "left" then
		return self.gameManager:GetValue("left_score")
	elseif GetTeamName(team) == "right" then
		return self.gameManager:GetValue("right_score")
	end

	return 0
end

function ScoreLimit:SetPoints(team, amount)
	if team == "left" then
		self.gameManager:SetValue("left_score", amount)
	elseif team == "right" then
		self.gameManager:SetValue("right_score", amount)
	end
	OnScoreChangedEvent.broadcast(team, amount)
	self:CheckWinCondition(team, amount)
end

function ScoreLimit:CheckWinCondition(team, points)
	if self.hasGameEnded then
		return
	end
	
	if points >= self.gameManager:GetValue("score_limit") then
		self:SetWinner(team)
	end
end

function ScoreLimit:SetWinner(team)
	self.hasGameEnded = true
	Debug.Log("WINNER OF THE GAME IS : " ..tostring(team))
	self.gameManager:SetGamePhase(GAME_PHASE_POST_GAME)
	
	CreateTimer(function() 
        GameManagerSystemRequestBus.Broadcast.ExecuteConsoleCommand("gamelift_stop_server")
    end,5.0);
end

function ScoreLimit:AddPoints(team, amount)
	local leftPoints = self.gameManager:GetValue("left_score")
	local rightPoints = self.gameManager:GetValue("right_score")
	if GetTeamName(team) == "left" then
		self:SetPoints("left", leftPoints + amount)
	elseif GetTeamName(team) == "right" then
		self:SetPoints("right", rightPoints + amount)
	end
end

function ScoreLimit:IncreaseLimit(extend_amount)
	ScoreLimit:SetLimit(ScoreLimit:GetLimit() + extend_amount)
	for _,player in ipairs(playerList) do
		player:SendClientEvent("notificationText", {
			text = "SCORE LIMIT INCREASED BY " ..tostring(extend_amount), duration = 5, color = Color(255,255,255)
		})
	end
end