require "scripts.variableholder";
local json = require "scripts.json"

Player = class(VariableHolder)

function GetPlayer (playerId)
	if type(playerId) == "number" then
		return Player({ entityId = GamePlayerIdRequestBus.Event.GetPlayerEntity(playerId) })
	end
	return Player({ entityId = playerId })
end

function Player:GetPlayerId()
	return GamePlayerInterfaceBus.Event.GetPlayerId(self.entityId)
end

function Player:SetSelectedHero(hero)
	return GamePlayerInterfaceBus.Event.SetSelectedHero(self.entityId, hero)
end

function Player:GetName()

end

function Player:GetPlayerIcon()

end

function Player:GetSelectedHero()
	return GamePlayerInterfaceBus.Event.GetSelectedHero(self.entityId)
end

function Player:GetTeamId()
	return GamePlayerInterfaceBus.Event.GetTeamId(self.entityId)
end

function Player:GetHero()
	local heroEntity = GamePlayerInterfaceBus.Event.GetSelectedHeroEntity(self.entityId)
	if heroEntity then
		return GetEntityInstance(heroEntity)
	end
	return
end

function Player:SendClientEvent (name, data, entity)
	GamePlayerInterfaceBus.Event.SendClientEvent(self.entityId, name, json.stringify(data), entity or EntityId())
end

function Player:LookAtEntity(entity)
	self:SendClientEvent("LookAtEntity", {}, entity)
end

function Player:LookAtHero()
	self:LookAtEntity(GamePlayerInterfaceBus.Event.GetSelectedHeroEntity(self.entityId))
end

function Player:SelectUnit (entity)
	self:SendClientEvent("SelectUnit", {}, entity)
end

function Player:IsBot ()
	if GamePlayerInterfaceBus.Event.GetPeerId(self.entityId) < 20 then
		return true
	end
end
