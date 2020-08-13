require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_take_damage";

-- Stun Bolt ability
ForestFriend = class(Ability)

local Z_OFFSET = 3

function ForestFriend:OnCreated ()
	self:RegisterVariable("max_range", 9999)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.45)
	self:RegisterVariable("range", 9999)
	self:RegisterVariable("speed", self:GetSpecialValue("speed"))
	self:RegisterVariable("damage_duration", self:GetSpecialValue("damage_duration"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
	self.OnTick = self.OnTick
	self.currentStack = nil;
end

function ForestFriend:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	Debug.Log('Firing StunBolt!')
	if not self.tickHandler then
		self.tickHandler = TickBus.Connect(self)
	end
	
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

	Debug.Log('Origin of hook is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	Debug.Log('Destination is ' .. tostring(target))

	local direction = (target - origin)
	direction.z = 0
	origin.z = origin.z + Z_OFFSET
	direction = direction:GetNormalized()
	Debug.Log('Direction is ' .. tostring(direction))
	local destination = origin + (direction * self:GetValue("range"))
	destination.z = destination.z + Z_OFFSET
	Debug.Log('Final travel point is ' .. tostring(target))

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = destination,
		speed = self:GetValue("speed"),
		particle = "heroes.muse.W_Forest_Friend_projectile",
		triggerRadius = 15
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		if UnitRequestBus.Event.GetTeamId(entityId) == UnitRequestBus.Event.GetTeamId(caster:GetId()) then
			return
		end
		--[[
		local damage = Damage()
		damage.type = Damage.DAMAGE_TYPE_MAGICAL
		damage.damage = self:GetSpecialValue("min_damage")
		damage.source = GetId(caster)
		damage.ability = self.entityId
		self.targetUnit:ApplyDamage(damage)
		
		if not self.targetUnit:FindModifierByTypeId("modifier_take_damage") then
			modifier:SetValue("current_stacked_amount", self.currentStack + 1)
			self.damageModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_take_damage", {duration = self:GetValue("damage_duration")})
		elseif not self.targetUnit:IsAlive() then
			self.targetUnit = nil
		else
			
		end]]

	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()
end

function ForestFriend:OnTick ()
	

end

return ForestFriend
