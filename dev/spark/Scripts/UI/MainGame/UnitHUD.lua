local UnitHUD = 
{
	spawnTickets = {};
	damageListener = {};
	spawnDamageTickets = {};
	UnitsOnScreen = {};
	selectColor = nil;
	Units = {};
	Properties = 
	{
		HUD = {default = EntityId()},
		selfDamageColor = {default = Color(255/255, 0/255, 0/255)},
		dealtDamageColor = {default = Color(255/255, 255/255, 255/255)},
		differentiateColor = {default = Color(170/255, 255/255, 127/255)},
		normalColor = {default = Color(85/255, 255/255, 0/255)},
	},
}

function UnitHUD:OnActivate()
	self.spawnTickets = {}
	self.shouldDelete = {}
	self.uiByUnit = {}

	self.tickBusHandler = TickBus.Connect(self)
	self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.HUD)
end

function UnitHUD:OnDeactivate()
	if self.tickBusHandler then
		self.tickBusHandler:Disconnect()
	end
	self.spawnerHandler:Disconnect()
	if self.damageHandler ~= nil then
		self.damageHandler:Disconnect()
	end
	if self.tah ~= nil then
		self.tah:Disconnect()
	end
end

function UnitHUD:OnTick(deltaTime, timePoint)
	if not self.tah then
		self.TriggerArea = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("camera_frustum"))
		Debug.Log('Trying to find the camera_frustum, found ' .. #self.TriggerArea .. ' results')
		if #self.TriggerArea > 0 then
			self.tah = TriggerAreaNotificationBus.Connect(self, self.TriggerArea[1])
			self.tickBusHandler:Disconnect()
			self.tickBusHandler = nil
		end	
	end
end

function UnitHUD:AddUI(unit)
	Debug.Log("Creating for unit " .. tostring(unit))
	local spawnTicket = UiSpawnerBus.Event.Spawn(self.Properties.HUD)
	table.insert(self.spawnTickets, {
		ticket = spawnTicket,
		unit = unit
	})
	self.shouldDelete[tostring(unit)] = nil
end

function UnitHUD:RemoveUI(unit)
	Debug.Log("Removing for unit " .. tostring(unit))
	if self.uiByUnit[tostring(unit)] then
		self:SendEvent(unit, "Destroy")
		self.uiByUnit[tostring(unit)] = nil
	else
		self.shouldDelete[tostring(unit)] = true
	end
end

function UnitHUD:OnEntitySpawned(ticket, id)
	for i = 1, #self.spawnTickets do
		if self.spawnTickets[i].ticket == ticket then
			local unit = self.spawnTickets[i].unit
			Debug.Log("I found a unit hud we\'re trying to spawn! " .. tostring(ticket) .. " = " .. tostring(id) .. " for " .. tostring(unit))
			table.remove(self.spawnTickets, i)
			if self.shouldDelete[tostring(unit)] then
				Debug.Log("Quick delete!")
				self.shouldDelete[tostring(unit)] = nil
				CreateTimer(function ()
					self:SendEvent(id, "Destroy", unit)
				end, 0.01)
			elseif self.uiByUnit[tostring(unit)] then
				Debug.Log("Dupliate quick delete!")
				CreateTimer(function ()
					self:SendEvent(id, "Destroy", unit)
				end, 0.01)
			else
				Debug.Log('Setting unit...')
				self:SendEvent(id, "SetUnit", unit)
				self.uiByUnit[tostring(unit)] = id
			end
			-- stop looping
			return;
		end
	end
end

function UnitHUD:OnTriggerAreaEntered(unitId)
	Debug.Log('Something entered? ' .. tostring(unitId))
	if HasTag(unitId,"unit")then
		self:AddUI(unitId)
	end
end

function UnitHUD:OnTriggerAreaExited(entityId)
	if HasTag(entityId,"unit")then
		self:RemoveUI(entityId)
	end
end

function UnitHUD:OnSpawnEnd(ticket)

end

function UnitHUD:OnSpawnFailed(ticket)

end

function UnitHUD:SendEvent(entityId, event, data)
  local keyId = GameplayNotificationId(entityId, event, "EntityId")
  data = data or EntityId()
  GameplayNotificationBus.Event.OnEventBegin(keyId, data)
end

return UnitHUD;
