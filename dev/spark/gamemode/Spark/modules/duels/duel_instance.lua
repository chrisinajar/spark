require "scripts.library.event"
local SaveState = require "gamemode.spark.modules.duels.savestate"

local DuelInstance = class({}, function (obj, options)
	obj.options = options
	obj.endDuelEvent = Event()
	obj.startDuelEvent = Event()
	obj.winDuelEvent = Event()
end)

function DuelInstance:Start ()
	self.timeoutTimer = CreateTimer(partial(self.End, self), self.options.timeout or 90)

	local allPlayers = {}
	self.playerStates = {}
	self.leftAlivePlayers = {}
	self.rightAlivePlayers = {}
	self.playerSides = {}
	for _,playerId in ipairs(self.options.leftPlayers) do
		table.insert(allPlayers, playerId)
		local player = GetPlayer(playerId)
		local state = SaveState:SavePlayerState(player)
		self.playerStates[playerId] = state
		self.leftAlivePlayers[tostring(player.entityId)] = true
		self.playerSides[tostring(player.entityId)] = "left"
		self.leftTeam = player:GetTeamId()
	end
	for _,playerId in ipairs(self.options.rightPlayers) do
		table.insert(allPlayers, playerId)
		local player = GetPlayer(playerId)
		local state = SaveState:SavePlayerState(player)
		self.playerStates[playerId] = state
		self.rightAlivePlayers[tostring(player.entityId)] = true
		self.playerSides[tostring(player.entityId)] = "right"
		self.rightTeam = player:GetTeamId()
	end
	self.options.zones.duel:Enable()
	self.options.zones.duel:SetPlayers(allPlayers)
	self.options.zones.leftSpawn:SetPlayers(self.options.leftPlayers)
	self.options.zones.leftSpawn:MoveUnitsIntoZone()
	self.options.zones.rightSpawn:SetPlayers(self.options.rightPlayers)
	self.options.zones.rightSpawn:MoveUnitsIntoZone()

	for _,playerId in ipairs(allPlayers) do
		GetPlayer(playerId):LookAtHero()
	end

	self.OnUnitKilled = self.OnUnitKilled
	self.unitsHandler = UnitsNotificationBus.Connect(self);

	-- last
	self.startDuelEvent.broadcast()
end

function DuelInstance:End ()
	if self.hasCleanedUp then
		return
	end
	if self.timeoutTimer then
		self.timeoutTimer()
		self.timeoutTimer = nil
	end

	if self.unitsHandler then
		self.unitsHandler:Disconnect()
	end

	for playerId,state in pairs(self.playerStates) do
		local player = GetPlayer(playerId)
		SaveState:RestorePlayerState(player, state)
		player:LookAtHero()
	end

	self.options.zones.duel:Disable()
	self.options.zones.duel:SetPlayers({})
	self.options.zones.leftSpawn:SetPlayers({})
	self.options.zones.rightSpawn:SetPlayers({})

	self.endDuelEvent.unlistenAll()
	self.startDuelEvent.unlistenAll()
	self.winDuelEvent.unlistenAll()

	self.hasCleanedUp = true
	self.endDuelEvent.broadcast(self.winner)
end

function DuelInstance:OnDuelEnd (fn)
	return self.endDuelEvent.listen(fn)
end

function DuelInstance:OnDuelStart (fn)
	return self.startDuelEvent.listen(fn)
end

function DuelInstance:OnDuelWon (fn)
	return self.winDuelEvent.listen(fn)
end

function DuelInstance:OnUnitKilled (unitId, damage)
	local unit = GetEntityInstance(unitId)

	if not unit:IsHero() then
		return
	end
	Debug.Log('Hero kill during duel!')

	local player = unit:GetPlayerOwner()
	local side = self.playerSides[tostring(player.entityId)]

	self[side .. "AlivePlayers"][tostring(player.entityId)] = false

	self:CheckDuelFinished()
end

function DuelInstance:CheckDuelFinished ()
	if self.hasFinished then
		return
	end
	local shouldEnd = true
	local winner = nil
	for _,alive in pairs(self.rightAlivePlayers) do
		if alive then
			winner = 'right'
		end
	end
	for _,alive in pairs(self.leftAlivePlayers) do
		if alive then
			if winner == 'right' then
				shouldEnd = false
			end
			winner = 'left'
		end
	end
	if not shouldEnd then
		return
	end

	self.hasFinished = true
	self.winner = self[winner .. 'Team']
	Debug.Log('Duel ended and was won by ' .. winner)

	self.winDuelEvent.broadcast(self.winner)
end

return DuelInstance
