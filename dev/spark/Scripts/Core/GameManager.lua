require "scripts.variableholder"
require "scripts.core.player"

GameManagerType = class(VariableHolder)

function GetGameManagerId()
	return GameManagerRequestBus.Broadcast.GetGameManagerEntity()
end

function GetGameManager ()
	return GameManagerType({
		entityId = GetGameManagerId()
	})
end

-- do things with GameManagerType?
-- know that instances do not persist between requests
-- it's just a dumb proxy

function GameManagerType:GetId ()
	return self.entityId
end

function GameManagerType:GetLocalPlayer ()
	return Player({ entityId = GameNetSyncRequestBus.Broadcast.GetLocalPlayer() })
end

function GameManagerType:GetLocalPlayerId ()
	return self:GetLocalPlayer():GetPlayerId()
end

function GameManagerType:GetGamePhase ()
	return GameManagerRequestBus.Broadcast.GetGamePhase()
end

function GameManagerType:SetGamePhase (newPhase)
	if newPhase <= self:GetGamePhase() then
		-- phases cannot go backwards
		return
	end
	return GameManagerRequestBus.Broadcast.SetGamePhase(newPhase)
end

function GameManagerType:GetPlayerList ()
	local playerList = GameNetSyncRequestBus.Broadcast.GetPlayerList()
	local result = {}
	Debug.Log('Looking for players...')
	for i = 1, #playerList do
		Debug.Log('Found player ' .. i)
		table.insert(result, Player({ entityId = playerList[i] }))
	end

	return result
end

function GameManagerType:CreateFakePlayer ()
	local playerEntity = GameNetSyncRequestBus.Broadcast.CreateFakePlayer()
	return Player({ entityId = playerEntity })
end

function IsServer()
	return GameManagerRequestBus.Broadcast.IsServer()
end

function GameManagerType:IsServer ()
	return IsServer()
end