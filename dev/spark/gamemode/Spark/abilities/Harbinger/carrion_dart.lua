require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_reduce_movement_speed";

-- Poke ability
Carrion_Dart = class(Ability)

local Z_OFFSET = 3

function Carrion_Dart:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.25)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function Carrion_Dart:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	
	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()
	if UnitRequestBus.Event.GetTeamId(target:GetId()) == UnitRequestBus.Event.GetTeamId(caster:GetId()) then
		return
	end
	
	local direction = (target:GetPosition() - origin)
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
		triggerRadius = 1
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		if entityId ~= target:GetId() then
			return
		end
		
		-- need to see how to trigger normal damage and anything else that might be triggered here
		local damage = Damage();
		damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
		damage.damage = self:GetCaster():GetValue("damage")
		damage.source = GetId(caster)
		self.targetUnit:ApplyDamage(damage)
		
		self.slowModifier = self.targetUnit:AddNewModifier(target, self, "modifier_reduce_movement_speed", {duration = self:GetSpecialValue("slow_duration")})
		
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()
	
end

return Carrion_Dart
