require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_Stun";

-- Stun Bolt ability
StunBolt = class(Ability)

local Z_OFFSET = 3

function StunBolt:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
end

function StunBolt:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");

	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

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
		particle = "heroes.Simple_agility_support.Q_Single_target_stun",
		triggerRadius = 3
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		
		if entityId ~= target:GetId() then
		
			self.targetUnit = Unit({ entityId = entityId})
			
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.damage = self:GetSpecialValue("damage");
			damage.source = GetId(caster)
			self.targetUnit:ApplyDamage(damage)
			self.stunModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_stun", {duration = self:GetSpecialValue("stun_duration")})
			
			self.projectile:Release()
			self.projectile = nil
		end
	end)

	self.projectile:Fire()
end

return StunBolt
