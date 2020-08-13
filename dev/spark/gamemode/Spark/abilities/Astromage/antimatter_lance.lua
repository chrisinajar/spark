require "scripts.core.ability"
require "scripts.core.projectile"

AntimatterLance = class(Ability)

local Z_OFFSET = 3

function AntimatterLance:OnCreated ()
	self:RegisterVariable("max_range", 500)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("speed", self:GetSpecialValue("speed"))
	self:RegisterVariable("damage", self:GetSpecialValue("damage"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
end

function AntimatterLance:OnSpellStart ()
	Debug.Log('Firing AntimatterLance!')

	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

	Debug.Log('Origin is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	Debug.Log('Destination is ' .. tostring(target))

	local direction = (target:GetPosition() - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	Debug.Log('Direction is ' .. tostring(direction))
	local destination = target:GetPosition()
	destination.z = destination.z + Z_OFFSET
	Debug.Log('Final travel point is ' .. tostring(target))

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetValue("speed"),
		particle = "heroes.astromage.R_Antimatter_Lance_projectile",
		triggerRadius = 1
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		local damage = Damage();
		damage.type = Damage.DAMAGE_TYPE_MAGICAL;
		damage.damage = self:GetValue("damage");
		damage.source = GetId(caster)
		damage.ability = self.entityId
		self.targetUnit:ApplyDamage(damage)
		
		--self.stunModifier = self.targetUnit:AddNewModifier(target, self, "modifier_Stun", {duration = self:GetSpecialValue("stun_duration")})
		
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()
end

return AntimatterLance