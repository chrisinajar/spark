Gold = CreateModule("Gold", GAME_PHASE_GAME)

function Gold:Init ()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("GOLD_CAP", 50000)
	self.gameManager:RegisterVariable("GPM_TICK_INTERVAL", 5)	
	
	self.Heroes = FilterArray( GetAllUnits(), function (unit)
		return HasTag(unit,"hero")
	end);
	
	CreateInterval(partial(self.PassiveGPM, self), 1.0)	

end

function Gold:ClearGold(playerId)
	self:SetGold(playerId, 0)
end

function Gold:SetGold(playerId, gold)
	local newGold = math.floor(gold)
	playerId:SetValue("gold",newGold)
end

function Gold:RemoveGold(playerId, gold)
	playerId:Take("gold",gold)
end

function Gold:GetGold(playerId)
	local currentGold = playerId:GetValue("gold")
	return math.floor(currentGold or 0)
end

function Gold:AddGold(playerId, gold)
	playerId:Give("gold",gold)
end

function Gold:PassiveGPM()
	local time = GetGameManager():GetValue("time")
	if time and time > 0 then
		local goldTick = math.floor(time/self.gameManager:GetValue("GPM_TICK_INTERVAL"))
		for i=1, #self.Heroes do
			local gpm = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.Heroes[i]:GetId(), "gpm"))
			self:AddGold(self.Heroes[i],tonumber(gpm/60))
		end
	end 
end

function Gold:IsGoldGenActive()
  return (not Duels:IsActive()) and HudTimer:GetGameTime() > 0
end

