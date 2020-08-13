require "scripts.core.unit"
require "scripts.core.gamemanager"
require "scripts.library.ebus"
require "scripts.library.functional"

local ZoneControlComponent = {
	Properties =
	{
		Enabled = false,
		Mode = { default = "in", description = "Lock units 'in', 'out', or 'both'"},
		TeamIDs = { default = { -1 }, description="Team IDs of units which should be affected by this zone"},
		PlayerIDs = { default = { -1 }, min=1, step=1.0, description="Player IDs which should be affected by this zone" }
	}
}

function ZoneControlComponent:OnActivate ()
	if self.Properties.Mode ~= "in" and self.Properties.Mode ~= "out" and self.Properties.Mode ~= "both" then
		Debug.Error("Invalid zone control mode: " .. tostring(self.Properties.Mode))
	end
	self.timerTable = {}

	self.triggerAreaHandler = TriggerAreaNotificationBus.Connect(self, self.entityId);
	self.origin = TransformBus.Event.GetWorldTranslation(self.entityId)

	local teamIds = self.Properties.TeamIDs
	local playerIds = self.Properties.PlayerIDs

	self.Properties.TeamIDs = {}
	self.Properties.PlayerIDs = {}

	for _,id in ipairs(teamIds) do
		if id > 0 then
			table.insert(self.Properties.TeamIDs, id)
		end
	end
	for _,id in ipairs(playerIds) do
		if id > 0 then
			table.insert(self.Properties.PlayerIDs, id)
		end
	end

	self:ConnectToRPC()
end

function ZoneControlComponent:ConnectToRPC ()
	if not GetGameManagerId() then
		CreateTimer(partial(self.ConnectToRPC, self), 0.1)
		return false
	end
	self.rpc = {
		toggleId = GameplayNotificationId(self.entityId, "Toggle", "float"),
		playerId = GameplayNotificationId(self.entityId, "Player", "float"),
		teamId = GameplayNotificationId(self.entityId, "Team", "float"),
		removeTeamId = GameplayNotificationId(self.entityId, "RemoveTeam", "float"),
		setModeId = GameplayNotificationId(self.entityId, "SetMode", "float"),
		moveUnitsId = GameplayNotificationId(self.entityId, "MoveUnitsIntoZone", "float"),
		findZoneId = GameplayNotificationId(GetGameManagerId(), "FindZone", "float"),
		zoneResultId = GameplayNotificationId(GetGameManagerId(), "ZoneResult", "EntityId"),
	}
	local eventList = {
		"OnEventBegin", "OnEventUpdating", "OnEventEnd"
	}
	self.rpc.findZone = ConnectToEvents(GameplayNotificationBus, self.rpc.findZoneId, eventList)
	self.rpc.toggle = ConnectToEvents(GameplayNotificationBus, self.rpc.toggleId, eventList)
	self.rpc.player = ConnectToEvents(GameplayNotificationBus, self.rpc.playerId, eventList)
	self.rpc.team = ConnectToEvents(GameplayNotificationBus, self.rpc.teamId, eventList)
	self.rpc.setMode = ConnectToEvents(GameplayNotificationBus, self.rpc.setModeId, eventList)
	self.rpc.moveUnits = ConnectToEvents(GameplayNotificationBus, self.rpc.moveUnitsId, eventList)

	self.rpc.toggle.OnEventBegin(partial(self.Enable, self))
	self.rpc.toggle.OnEventEnd(partial(self.Disable, self))
	self.rpc.player.OnEventBegin(partial(self.AddPlayer, self))
	self.rpc.player.OnEventEnd(partial(self.RemovePlayer, self))
	self.rpc.player.OnEventUpdating(function (mode)
		if mode == -1 then
			self:SetPlayerIds({})
		end
	end)
	self.rpc.team.OnEventBegin(partial(self.AddTeam, self))
	self.rpc.team.OnEventEnd(partial(self.RemoveTeam, self))
	self.rpc.setMode.OnEventBegin(partial(self.SetMode, self))
	self.rpc.moveUnits.OnEventBegin(partial(self.MoveUnitsIntoZone, self))
	self.rpc.findZone.OnEventBegin(function (name)
		if Crc32(GameEntityContextRequestBus.Broadcast.GetEntityName(self.entityId)) == name then
			GameplayNotificationBus.Event.OnEventBegin(self.rpc.zoneResultId, self.entityId)
		end
	end)
end

function ZoneControlComponent:OnDeactivate ()
	self.triggerAreaHandler:Disconnect()
	if self.rpc then
		self.rpc.toggle()
		self.rpc.player()
		self.rpc.team()
		self.rpc.setMode()
		self.rpc.findZone()
		self.rpc = nil
	end
end

function ZoneControlComponent:SetMode (newMode)
	self.Properties.Mode = newMode
end

function ZoneControlComponent:Enable ()
	self.Properties.Enabled = true
end

function ZoneControlComponent:Disable ()
	self.Properties.Enabled = false
end

function ZoneControlComponent:SetTeamIds (newTeams)
	self.Properties.TeamIDs = newTeams
end

function ZoneControlComponent:AddTeam (teamId)
	table.insert(self.Properties.TeamIDs, teamId)
end

function ZoneControlComponent:RemoveTeam (teamId)
	table.insert(self.Properties.TeamIDs, teamId)
end

function ZoneControlComponent:SetPlayerIds (newPlayers)
	self.Properties.PlayerIDs = newPlayers
end

function ZoneControlComponent:AddPlayer (playerId)
	table.insert(self.Properties.PlayerIDs, playerId)
end

function ZoneControlComponent:RemovePlayer (playerId)
	table.insert(self.Properties.PlayerIDs, playerId)
end

function ZoneControlComponent:MoveUnitsIntoZone ()
	local units = GetAllUnits()
	for i=1,#units do
		local unit = units[i]
		if self:ShouldEnforceRulesOn(unit) then
			local zoneaabb = ShapeComponentRequestsBus.Event.GetEncompassingAabb(self.entityId)
			local x = math.random(zoneaabb.min.x, zoneaabb.max.x)
			local y = math.random(zoneaabb.min.y, zoneaabb.max.y)

			unit:SetPosition(Vector3(x, y, 0))
		end
	end
end

function ZoneControlComponent:OnTriggerAreaEntered (unitId)
	Debug.Log('Unit entered the zone control ' .. tostring(unitId))
	local unit = Unit({ entityId = unitId })

	local mode = self.Properties.Mode
	local force = false
	if mode == "both" then
		force = true
		if self:ShouldEnforceRulesOn(unit) then
			mode = "in"
		else
			mode = "out"
		end
	end

	if mode == "in" then
		if self.timerTable[unitId] then
			self.timerTable[unitId]()
			self.timerTable[unitId] = nil
		end
	elseif mode == "out" then
		self:EnforceRulesOnUnit(unit, force, mode)
	end
end

function ZoneControlComponent:OnTriggerAreaExited (unitId)
	Debug.Log('Unit exited the zone control ' .. tostring(unitId) .. tostring(self.Properties.Mode))
	local unit = Unit({ entityId = unitId })

	local mode = self.Properties.Mode
	local force = false
	if mode == "both" then
		force = true
		if self:ShouldEnforceRulesOn(unit) then
			mode = "in"
		else
			mode = "out"
		end
	end

	if mode == "in" then
		self:EnforceRulesOnUnit(unit, force, mode)
	elseif mode == "out" then
		if self.timerTable[unitId] then
			self.timerTable[unitId]()
			self.timerTable[unitId] = nil
		end
	end
end

-- iterate over all units and make sure everyone is locked in/out correctly
function ZoneControlComponent:EnforceRules ()
	if not self.Properties.Enabled then
		return
	end
end

-- enforce rules on just 1 unit
function ZoneControlComponent:EnforceRulesOnUnit (unit, force, mode)
	if not self.Properties.Enabled then
		return
	end
	if not force and not self:ShouldEnforceRulesOn(unit) then
		return
	end
	-- Debug.Log('Enforcing rules on ' .. tostring(unit:GetName()))
	mode = mode or self.Properties.Mode

	if mode == "in" then
		self:LockUnitIn(unit)
	elseif mode == "out" then
		self:LockUnitOut(unit)
	end

	self.timerTable[unit.entityId] = CreateTimer(partial(self.EnforceRulesOnUnit, self, unit, force, mode), 0.0)
end

function ZoneControlComponent:LockUnitIn (unit)
	local point = self:GetClosestPointInside(unit)
	if point then
		unit:SetPosition(point)
	end
end

function ZoneControlComponent:LockUnitOut (unit)
	local point = self:GetClosestPointOutside(unit)
	if point then
		unit:SetPosition(point)
	end
end

function ZoneControlComponent:ShouldEnforceRulesOn (unit)
	local unitTeamId = unit:GetTeamId()

	-- neutrals cannot be zone controlled
	if unitTeamId == GetTeamIdByName("neutrals") then
		return false
	end

	for _,teamId in ipairs(self.Properties.TeamIDs) do
		if unitTeamId == teamId then
			return true
		end
	end

	local unitPlayer = unit:GetPlayerOwner()
	local unitPlayerId = nil
	if unitPlayer then
		unitPlayerId = unitPlayer:GetPlayerId()
	end

	if unitPlayerId then
		for _,playerId in ipairs(self.Properties.PlayerIDs) do
			if unitPlayerId == playerId then
				return true
			end
		end
	end

	return false
end

function ZoneControlComponent:GetClosestPointInside (unit)
	local unitPosition = unit:GetPosition()
	if ShapeComponentRequestsBus.Event.IsPointInside(self.entityId, unitPosition) then
		return false
	end
	local baseDistance = ShapeComponentRequestsBus.Event.DistanceFromPoint(self.entityId, unitPosition)
	local checkDistance = math.max(1, baseDistance)
	local direction = (self.origin - unitPosition):GetNormalized()
	local hasEverPoint = false
	local loopCount = 0
	while loopCount < 50 do
		loopCount = loopCount + 1
		local result = (unitPosition + (direction * baseDistance))
		local isInShape = ShapeComponentRequestsBus.Event.IsPointInside(self.entityId, result)
		if isInShape then
			-- found it, start rolling backwards
			hasEverPoint = true
			checkDistance = checkDistance / 2
			if checkDistance < 0.1 then
				return result
			end
			baseDistance = baseDistance - checkDistance
		else
			-- only binary search once it's been found once
			if hasEverPoint then
				checkDistance = math.max(1, checkDistance / 2)
			else
				-- if we've not found the zone yet, get the distance from our current guess
				checkDistance = 1 + ShapeComponentRequestsBus.Event.DistanceFromPoint(self.entityId, result)
			end
			baseDistance = baseDistance + checkDistance
		end
	end
	Debug.Log('ZoneControlComponent:GetClosestPointInside - Reached maximum number of loops before giving up')
	return false
end

function ZoneControlComponent:GetClosestPointOutside (unit)
	local unitPosition = unit:GetPosition()
	if not ShapeComponentRequestsBus.Event.IsPointInside(self.entityId, unitPosition) then
		return false
	end
	local baseDistance = ShapeComponentRequestsBus.Event.DistanceFromPoint(self.entityId, unitPosition)
	local checkDistance = math.max(1, baseDistance)
	local direction = (unitPosition - self.origin):GetNormalized()
	local hasEverPoint = false
	local loopCount = 0
	while loopCount < 50 do
		loopCount = loopCount + 1
		local result = (unitPosition + (direction * baseDistance))
		local isOutShape = not ShapeComponentRequestsBus.Event.IsPointInside(self.entityId, result)
		if isOutShape then
			-- found it, start rolling backwards
			hasEverPoint = true
			checkDistance = checkDistance / 2
			if checkDistance < 0.1 then
				return result
			end
			baseDistance = baseDistance - checkDistance
		else
			-- only binary search once it's been found once in case it's 2x base distance away (which we'd asymptotically approach)
			if hasEverPoint then
				checkDistance = math.max(1, checkDistance / 2)
			end
			baseDistance = baseDistance + checkDistance
		end
	end
	Debug.Log('ZoneControlComponent:GetClosestPointOutside - Reached maximum number of loops before giving up')
	return false
end

return ZoneControlComponent
