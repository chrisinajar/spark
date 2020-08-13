require "scripts.library.class"
require "scripts.library.ebus"

ZoneControl = class(function (obj, zoneName)
	obj:Init(zoneName)
end)

function ZoneControl:Init (name)
	self.rpc = {
		findZoneId = GameplayNotificationId(GetGameManagerId(), "FindZone", "float"),
		zoneResultId = GameplayNotificationId(GetGameManagerId(), "ZoneResult", "EntityId"),
	}
	self.name = name
	self.zones = {}

	self.zoneResult = ConnectToEvents(GameplayNotificationBus, self.rpc.zoneResultId, { "OnEventBegin" })
	self.zoneResult.OnEventBegin(function (entityId)
		if GameEntityContextRequestBus.Broadcast.GetEntityName(entityId) == name then
			table.insert(self.zones, entityId)
		end
	end)

	GameplayNotificationBus.Event.OnEventBegin(self.rpc.findZoneId, Crc32(name))

	Debug.Log('Found ' .. #self.zones .. ' zones named ' .. name)

	self.zoneResult()
	self.zoneResult = nil

	self.rpc.toggleId = GameplayNotificationId(self.zones[1], "Toggle", "float")
	self.rpc.playerId = GameplayNotificationId(self.zones[1], "Player", "float")
	self.rpc.teamId = GameplayNotificationId(self.zones[1], "Team", "float")
	self.rpc.removeTeamId = GameplayNotificationId(self.zones[1], "RemoveTeam", "float")
	self.rpc.setModeId = GameplayNotificationId(self.zones[1], "SetMode", "float")
	self.rpc.moveUnitsId = GameplayNotificationId(self.zones[1], "MoveUnitsIntoZone", "float")
	self.rpc.findZoneId = GameplayNotificationId(GetGameManagerId(), "FindZone", "float")
	self.rpc.zoneResultId = GameplayNotificationId(GetGameManagerId(), "ZoneResult", "EntityId")
end

function ZoneControl:MoveUnitsIntoZone ()
	GameplayNotificationBus.Event.OnEventBegin(self.rpc.moveUnitsId, 0)
end

function ZoneControl:AddTeam (teamId)
	GameplayNotificationBus.Event.OnEventBegin(self.rpc.teamId, teamId)
end

function ZoneControl:SetMode (newMode)
	if newMode == "in" then
		newMode = 0
	elseif newMode == "out" then
		newMode = 1
	elseif newMode == "both" then
		newMode = 2
	end

	GameplayNotificationBus.Event.OnEventBegin(self.rpc.setModeId, newMode)
end

function ZoneControl:Enable ()
	GameplayNotificationBus.Event.OnEventBegin(self.rpc.toggleId, 0)
end

function ZoneControl:Disable ()
	GameplayNotificationBus.Event.OnEventEnd(self.rpc.toggleId, 0)
end

function ZoneControl:SetPlayers (newPlayers)
	-- clear list
	GameplayNotificationBus.Event.OnEventUpdating(self.rpc.playerId, -1)
	for _,playerId in ipairs(newPlayers) do
		self:AddPlayer(playerId)
	end
end

function ZoneControl:AddPlayer (playerId)
	GameplayNotificationBus.Event.OnEventBegin(self.rpc.playerId, playerId)
end
