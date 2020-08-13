require "scripts.library.class"
require "scripts.library.event"
require "scripts.library.promise"
require "scripts.library.ebus"

Projectile = class(function (obj, options)
	obj.options = options or {}

	obj.enterEvent = Event()
	obj.exitEvent = Event()
	obj.projectileHit = Event()
	obj.entityIdPromise = Promise()
end)

local unreleasedParticles = {}

function Projectile:OnEntityEnter (handler)
	return self.enterEvent.listen(handler)
end

function Projectile:OnEntityExit (handler)
	return self.exitEvent.listen(handler)
end

function Projectile:OnProjectileHit (handler)
	return self.projectileHit.listen(handler)
end

function Projectile:GetPosition()
	if not self.entityId then
		return nil
	end
	return TransformBus.Event.GetWorldTranslation(self.entityId)
end

function Projectile:Fire (options)
	if self.info then
		return self:FireAgain(options or {})
	end

	local info = ProjectileInfo()

	self.info = info
	for k,v in pairs(self.options) do
		self.info[k] = v
	end
	local id = ProjectileManagerRequestBus.Broadcast.CreateProjectile(self.info)
	self.info.projectileId = id
	self.notificationsHandler = ProjectileManagerNotificationBus.Connect({
		OnProjectileCreated = function (obj, options, entityId)
			self.notificationsHandler:Disconnect()
			self:SetProjectileEntity(entityId)
		end
	}, id)
	ProjectileManagerRequestBus.Broadcast.FireProjectile(self.info)

	Debug.Log('projectile id ' .. tostring(self.info.projectileId));
end

function Projectile:FireAgain (options)
	for k,v in pairs(options) do
		self.info[k] = v
	end
	self.entityIdPromise.value(function (entityId)
		self.info.startingPosition = TransformBus.Event.GetWorldTranslation(entityId)
		ProjectileRequestBus.Event.Fire(entityId, self.info)
	end)
end

function Projectile:Release ()
	Debug.Log('projectile id to be released ' .. tostring(self.info.projectileId));
	ProjectileManagerRequestBus.Broadcast.ReleaseProjectile(self.info.projectileId)

	if self.triggerAreaEvents then
		self.triggerAreaEvents()
		self.triggerAreaEvents = nil
	end
	if self.projectileEvents then
		self.projectileEvents()
		self.projectileEvents = nil
	end
	table.remove(unreleasedParticles, self.info.projectileId)
	self.info = nil
end

function Projectile:SetProjectileEntity (entityId)
	self.entityId = entityId
	self.projectileEvents = ConnectToEvents(ProjectileNotificationBus, self.entityId, {
		"OnProjectileHit",
	})
	self.triggerAreaEvents = ConnectToEvents(TriggerAreaNotificationBus, self.entityId, {
		"OnTriggerAreaEntered",
		"OnTriggerAreaExited",
	})

	-- this is cleaned up automatically by the release method's invokation of triggerAreaEvents()
	self.triggerAreaEvents.OnTriggerAreaEntered(self.enterEvent.broadcast)
	self.triggerAreaEvents.OnTriggerAreaExited(self.exitEvent.broadcast)
	self.projectileEvents.OnProjectileHit(self.projectileHit.broadcast)

	self.entityIdPromise.resolve(entityId)
end
