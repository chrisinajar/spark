require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_reduce_armor";

-- Stun Bolt ability
ArmorWave = class(Ability)

local Z_OFFSET = 3

function ArmorWave:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function ArmorWave:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");

	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()


	local direction = (target - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	local destination = origin + (direction * self:GetSpecialValue("range"))
	destination.z = destination.z + Z_OFFSET

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetSpecialValue("speed"),
		particle = "hookwars.fire_ball",
		--particle = "heroes.Simple_agility_support.W_Long_range_scouting_nuke",
		triggerRadius = 3
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		if UnitRequestBus.Event.GetTeamId(entityId) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.source = GetId(caster)
			damage.damage = self:GetSpecialValue("damage");
			self.targetUnit:ApplyDamage(damage)
			
			if self.targetUnit:FindModifierByTypeId("modifier_reduce_armor") then
				self.targetUnit:Kill()
				self.targetUnit = nil
			elseif not self.targetUnit:IsAlive() then
				self.targetUnit = nil
			else
				self.armorModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_reduce_armor", {duration = self:GetSpecialValue("duration")})
			end
		end

	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()
end

return ArmorWave
