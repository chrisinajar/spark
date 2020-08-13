require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_Stun";

local DebugRenderer = Require("debugRenderer")

Quasar = class(Ability)

local Z_OFFSET = 3

function Quasar:OnCreated ()
	self:RegisterVariable("max_range", 20)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.5) 
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("speed", 0.0)
	self:RegisterVariable("damage", self:GetSpecialValue("damage"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function Quasar:OnSpellStart ()
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCursorPosition()

	Debug.Log('Origin is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	Debug.Log('Destination is ' .. tostring(target))

	CreateTimer(function()
		local duration = 15
		local thinker = ModifiersHandler:CreateModifierThinker(caster,self,"modifier_empty",{duration=duration,radius=10},origin)
		
		AttachParticleSystem(thinker, "heroes.astromage.w_quasar")
		DebugRenderer.DrawAura(thinker,Color(1,0,0,0.4),duration)
	end,0.5);

	--local ps = GameManagerRequestBus.Broadcast.CreateParticleSystem("heroes.astromage.w_quasar")

	--local pos = TransformBus.Event.GetLocalTranslation(GetId(thinker))
	--Debug.Log("\n\n\nthinker pos is "..tostring(pos))

	--TransformBus.Event.SetWorldTranslation(ps,origin)--)

	--local ps = GameManagerRequestBus.Broadcast.CreateParticleSystem("heroes.astromage.w_quasar")
	--local position = TransformBus.Event.GetWorldTranslation(GetId(caster))
	--TransformBus.Event.SetWorldTranslation(ps,origin)

	--[[local direction = (target - origin)
	--direction.z = 0
	origin.z = origin.z + Z_OFFSET
	--direction = direction:GetNormalized()
	--Debug.Log('Direction is ' .. tostring(direction))
	--local destination = target
	--destination.z = destination.z + Z_OFFSET
	--Debug.Log('Final travel point is ' .. tostring(target))

	self.projectile = Projectile({
		startingPosition = origin,
		targetPosition = origin,
		speed = self:GetValue("speed"),
		particle = "heroes.astromage.w_quasar",
		triggerRadius = 10
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		local damage = Damage();
		damage.type = Damage.DAMAGE_TYPE_MAGICAL;
		damage.damage = self:GetValue("damage");
		self.targetUnit:ApplyDamage(damage)
		
		if self.targetUnit:FindModifierByTypeId("modifier_Stun") then
			self.targetUnit:Kill()
			self.targetUnit = nil
		elseif not self.targetUnit:IsAlive() then
			self.targetUnit = nil
		else
			self.stunModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_Stun", {duration = self:GetSpecialValue("stun_duration")})
		end
	end)
	
	self.projectile:OnProjectileHit(function ()
		-- hit the end position without hitting any heroes
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()]]
end

return Quasar