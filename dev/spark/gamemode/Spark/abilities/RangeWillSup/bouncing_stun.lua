require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_stun";

-- Stun Bolt ability
BouncingStun = class(Ability)

local Z_OFFSET = 3

function BouncingStun:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.35)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
	
	self.OnTick = self.OnTick
end

function BouncingStun:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	self.BouncesAllowed = self:GetSpecialValue("bounces")

	local target = self:GetCursorTarget()
	self.currentTarget = target
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()
	
	local direction = (self:GetCursorTarget():GetPosition() - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	local destination = target:GetPosition()
	destination.z = destination.z + Z_OFFSET

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetSpecialValue("speed"),
		particle = "hookwars.fire_ball",
		triggerRadius = 0.1
	});
	if not self.tickHandler then
		--self.tickHandler = TickBus.Connect(self)
	end
	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		if UnitRequestBus.Event.GetTeamId(entityId) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.damage = self:GetSpecialValue("damage");
			damage.source = GetId(self:GetCaster())
			self.targetUnit:ApplyDamage(damage)

			if not self.targetUnit:IsAlive() then
				self.targetUnit = nil
			else
				self.stunModifier = self.targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_stun", {duration = self:GetSpecialValue("stun_duration")})
				self.projectile:Release()
				self.projectile = nil
				if self.tickHandler then
					self.tickHandler:Disconnect()
					self.tickHandler = nil
				end
				self:Timer()
			end
		end
		
	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self.projectile:Release()
		self.projectile = nil
		self.tickHandler:Disconnect()
		self.tickHandler = nil
	end)

	self.projectile:Fire()
end

function BouncingStun:OnTick ()
	if self.projectile then 
		local origin = self.projectile:GetPosition()
		origin.z = origin.z - Z_OFFSET
		if self.targetUnit then
			self.targetUnit:SetPosition(origin,true)
			self.targetUnit:SetPosition(origin,2)
		end
	else
		self.tickHandler:Disconnect()
		self.tickHandler = nil
	end

	
end

function BouncingStun:FindTargets()
	local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(self.currentTarget:GetPosition(), self:GetSpecialValue("bounces_distance"))
	local targetableUnits = {}
	if #units > 0 then 
		for i = 1, #units do
			if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) and self.currentTarget ~= units[i] then
				table.insert(targetableUnits, units[i])
			end
		end
		if #targetableUnits > 0 then
			local unit = math.random(1, #targetableUnits)
			local targetUnit = Unit({ entityId = targetableUnits[unit]})
			return targetUnit
		else
			return nil
		end
	else
		return nil
	end
end

function BouncingStun:BounceToTarget()
	local target = self:FindTargets(self.currentTarget)
	local caster = self:GetCaster()
	local origin = self.currentTarget
	local originPos = self.currentTarget:GetPosition()
	
	local direction = (target:GetPosition() - origin:GetPosition())
	direction.z = 0
	originPos.z = originPos.z + Z_OFFSET
	direction = direction:GetNormalized()
	local destination = target:GetPosition()
	destination.z = destination.z + Z_OFFSET

	self.projectile = Projectile({
		startingPosition = originPos,
		targetPosition = destination,
		speed = self:GetSpecialValue("speed"),
		particle = "hookwars.fire_ball",
		triggerRadius = 0.1
	});
	if not self.tickHandler then
		--self.tickHandler = TickBus.Connect(self)
	end

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		if UnitRequestBus.Event.GetTeamId(entityId) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.damage = self:GetSpecialValue("damage");
			damage.source = GetId(self:GetCaster())
			self.targetUnit:ApplyDamage(damage)

			if not self.targetUnit:IsAlive() then
				self.targetUnit = nil
			else
				self.BouncesAllowed = self.BouncesAllowed - 1
				self.stunModifier = self.targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_stun", {duration = self:GetSpecialValue("stun_duration")})
				self.currentTarget = target
				self.projectile:Release()
				self.projectile = nil
				if self.tickHandler then
					self.tickHandler:Disconnect()
					self.tickHandler = nil
				end
				self:Timer()
			end
		end

		
	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self.projectile:Release()
		self.projectile = nil
		self.tickHandler:Disconnect()
		self.tickHandler = nil
	end)

	self.projectile:Fire()
end

function BouncingStun:Timer()
	if self.BouncesAllowed > 0 then
		CreateTimer(function()
		self:BounceToTarget()
		end,0.75);
		
	end
end

return BouncingStun