require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_line_stun";

-- Stun Bolt ability
LineStun = class(Ability)

local Z_OFFSET = 3

function LineStun:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self.OnTick = self.OnTick
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function LineStun:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");

	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

	local direction = (target - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	local destination = origin + (direction * self:GetValue("range"))
	destination.z = destination.z + Z_OFFSET

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetSpecialValue("speed"),
		particle = "hookwars.fire_ball",
		triggerRadius = 2--self:GetSpecialValue("projectile_width")
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		
			self.targetUnit = Unit({ entityId = entityId})
			if not self.targetUnit:FindModifierByTypeId("modifier_hooked") then
				local damage = Damage();
				damage.type = Damage.DAMAGE_TYPE_MAGICAL;
				damage.damage = self:GetSpecialValue("damage");
				damage.source = GetId(caster)
				self.targetUnit:ApplyDamage(damage)
			
				self.stunModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_line_stun", {duration = self:GetSpecialValue("stun_duration")})
			end
	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		if not self.tickHandler then
			self.tickHandler = TickBus.Connect(self)
		end
		self:Return()
	end)

	self.projectile:Fire()
end

function LineStun:OnTick ()
	if self.projectile then
		local origin = self.projectile:GetPosition()
		origin.z = origin.z - Z_OFFSET
	
		if self.finished then
			self.tickHandler:Disconnect()
			self.tickHandler = nil
			return
		end
	end
end

function LineStun:Return ()
	local origin = self:GetCaster():GetPosition()
	local currentLocation = self.projectile:GetPosition()
	local direction = currentLocation - origin
	direction.z = 0
	direction = direction:GetNormalized()

	origin = origin + direction
	origin.z = origin.z + Z_OFFSET
	Debug.Log('Returning projectile to ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()))
	if not self.isReturning then
		self.isReturning = true
		self.projectile:Fire({
			targetPosition = origin,
		})
	else
		Debug.Log('Releasing!')
		self.finished = true
		self.projectile:Release()
		self.projectile = nil
	end
end

return LineStun
