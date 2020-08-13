require "scripts.core.item"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_status_root";

Staff_Of_Binding = class(Item)

local Z_OFFSET = 3

function Staff_Of_Binding:OnCreated ()
	self:RegisterVariable("max_range", 500)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("speed", self:GetSpecialValue("speed"))
	self:RegisterVariable("root_duration", self:GetSpecialValue("root_duration"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
end

function Staff_Of_Binding:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	Debug.Log('Firing Staff_Of_Binding!')

	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()

	Debug.Log('Origin of Staff_Of_Binding is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
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
		particle = "hookwars.fire_ball",
		triggerRadius = 1
	});

	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		self.rootModifier = self.targetUnit:AddNewModifier(target, self, "modifier_status_root", {duration = self:GetValue("root_duration")})
		
		self.projectile:Release()
		self.projectile = nil
	end)

	self.projectile:Fire()
end


return Staff_Of_Binding
