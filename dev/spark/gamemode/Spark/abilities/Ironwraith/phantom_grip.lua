require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_hooked";
require "gamemode.Spark.modifiers.modifier_hooking";

-- hook ability
PhantomGrip = class(Ability)

local Z_OFFSET = 3

function PhantomGrip:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.25)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));

	self.OnTick = self.OnTick
end

function PhantomGrip:OnSpellStart ()
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	self.finished = false

	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()
	self.isReturning = false

	local direction = (target - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	local destination = origin + (direction * self:GetSpecialValue("range"))

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetSpecialValue("speed"),
		particle = "hookwars.fire_ball",
		triggerRadius = 3
	});

	self.hookingModifier = caster:AddNewModifier(caster, self, "modifier_hooking", {})

	self.unlisten = self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.attachedUnit = Unit({ entityId = entityId})
		AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_hit")
		
		if UnitRequestBus.Event.GetTeamId(entityId) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.damage = self:GetSpecialValue("damage");
			self.attachedUnit:ApplyDamage(damage)
		end
		
		if self.attachedUnit:FindModifierByTypeId("modifier_hooked") then
			self.attachedUnit:Kill()
			self.attachedUnit = nil
		elseif not self.attachedUnit:IsAlive() then
			self.attachedUnit = nil
		else
			self.hookedModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_hooked", {})
		end

		if not self.tickHandler then
			self.tickHandler = TickBus.Connect(self)
		end
		self:Return()
	end)

	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self:Return()
	end)

	self.projectile:Fire()
end

function PhantomGrip:OnTick ()
	if self.finished or not self.attachedUnit or not self.attachedUnit:IsAlive() then
		self.tickHandler:Disconnect()
		self.tickHandler = nil
		self.attachedUnit = nil
		self.hookedModifier = nil
		return
	end

	local origin = self.projectile:GetPosition()
	origin.z = origin.z - Z_OFFSET

	self.attachedUnit:SetPosition(origin)
end

function PhantomGrip:Return ()
	self.unlisten()
	if self.hookingModifier then
		self.hookingModifier:Destroy()
		self.hookingModifier = nil
	end

	local origin = self:GetCaster():GetPosition()
	local currentLocation = self.projectile:GetPosition()
	local direction = currentLocation - origin
	direction.z = 0
	direction = direction:GetNormalized()

	origin = origin + direction
	origin.z = origin.z + Z_OFFSET
	if not self.isReturning then
		self.isReturning = true
		self.projectile:Fire({
			targetPosition = origin,
		})
	else
		self.finished = true
		self.projectile:Release()
		self.projectile = nil
		self.attachedUnit = nil
		if self.hookedModifier then
			self.hookedModifier:Destroy()
			self.hookedModifier = nil
		end
		if self.tickHandler then
			self.tickHandler:Disconnect()
			self.tickHandler = nil
		end
	end
end

return PhantomGrip
