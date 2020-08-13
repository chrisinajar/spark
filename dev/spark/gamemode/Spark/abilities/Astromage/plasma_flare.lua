require "scripts.core.ability"
require "scripts.core.projectile"
local DebugRenderer = Require("DebugRenderer")

PlasmaFlare = class(Ability)

local Z_OFFSET = 3

function PlasmaFlare:OnCreated ()
	self:RegisterVariable("max_range", 500)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0) 
	self:RegisterVariable("range", self:GetSpecialValue("range") or 10)
	self:RegisterVariable("speed", 10.0)
	self:RegisterVariable("damage", self:GetSpecialValue("damage"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end


function PlasmaFlare:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	Debug.Log('Firing PlasmaFlare!')

	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

	Debug.Log('Origin is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	Debug.Log('Destination is ' .. tostring(target))

	local direction = (target - origin)
	direction.z = 0
	direction = direction:GetNormalized()

	local projectile = ModifiersHandler:CreateModifierThinkerProjectile(caster,self,"modifier_empty",{duration=0.5,radius=0.1},origin+direction*0.2)
	projectile.speed = 40
	projectile.radius_speed = projectile.speed * 0.2
	projectile.direction = direction
	projectile.hit_entities ={}

	Debug.Log("projectile created: "..tostring(projectile.entityId))


	local ps = GameManagerRequestBus.Broadcast.CreateParticleSystem("heroes.astromage.q_plasma_flare")
	TransformBus.Event.SetWorldTranslation(ps,origin + direction * 5 + Vector3(0,0,3))
	TransformBus.Event.SetLocalRotation(ps,TransformBus.Event.GetWorldRotation(GetId(caster)))
	CreateTimer(function()
		Debug.Log("deleting particle system")
		GameEntityContextRequestBus.Broadcast.DestroyGameEntity(ps);
	end,1)

	projectile.OnTick = 
	function (self,deltaTime)
		self.time = (self.time or 0) + deltaTime 

		DebugRenderer.DrawAura(self)

		local position = NavigationEntityRequestBus.Event.GetPosition(self.entityId)
		position = position + self.direction * self.speed * deltaTime
		NavigationEntityRequestBus.Event.SetPosition(self.entityId,position)
		self:SetAuraRadius(self.time * projectile.radius_speed)
	end

	projectile.OnTriggerAreaEntered =
	function(self, unitId)
		if unitId == GetId(self:GetCaster()) then return end

		if projectile.hit_entities[tostring(unitId)]~=nil then --just hit once
			--Debug.Log("double hit")
			return
		end
		projectile.hit_entities[tostring(unitId)] = true

		--Debug.Log("projectile:OnTriggerAreaEntered ")
		local damage = Damage();
		damage.source = GetId(self:GetCaster())
		damage.type = Damage.DAMAGE_TYPE_MAGICAL;
		damage.damage = 70

		Unit.ApplyDamage({entityId=unitId},damage)
	end

	--[[ not really needed, just for testing multiple events of the same bus
	projectile.OnTriggerAreaExited =
	function(self, unitId)
		if unitId == GetId(self:GetCaster()) then return end
		Debug.Log("projectile:OnTriggerAreaExited ")
	end
	--]]

	projectile:ListenEvent("TickBus","OnTick")
	projectile:ListenEvent(TriggerAreaNotificationBus,"OnTriggerAreaEntered")
	--projectile:ListenEvent(TriggerAreaNotificationBus,"OnTriggerAreaExited",projectile.entityId)

	projectile:Fire()
end

return PlasmaFlare