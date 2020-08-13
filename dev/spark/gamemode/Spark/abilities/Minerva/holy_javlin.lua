require "scripts.core.ability"
require "scripts.core.projectile"

Holy_Javlin = class(Ability)

local Z_OFFSET = 3

function Holy_Javlin:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.25)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("speed", self:GetSpecialValue("speed"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
end

function Holy_Javlin:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	Debug.Log('Firing Holy Javlin!')

	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()
	local casterTeam = UnitRequestBus.Event.GetTeamId(caster:GetId())
	local targetTeam = UnitRequestBus.Event.GetTeamId(target:GetId())
		
	local damage = Damage();
	damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
	damage.damage = self:GetValue("self_damage");
	caster:ApplyDamage(damage)

	Debug.Log('Origin of hook is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
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
		particle = "heroes.minerva.Q_Holy_Javelin_projectile",
		triggerRadius = 3
	});
	
	caster:Take("hp",self:GetSpecialValue("self_damage_amount"))
	
	self.projectile:OnEntityEnter(function (entityId)
		if entityId == self:GetCaster():GetId() then
			return
		end
		if entityId ~= target:GetId() then
			return
		end
		self.targetUnit = Unit({ entityId = entityId})
		
		-- add in hit projectile here
		
		-- check here if enemy or friend
		if casterTeam == targetTeam then
		
			target:Give("hp",self:GetSpecialValue("heal_amount"))
			self.projectile:Release()
			self.projectile = nil
			
		-- if enemy then do
		elseif casterTeam ~= targetTeam then
		
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.source = GetId(caster)
			damage.damage = self:GetSpecialValue("damage_amount");
			self.targetUnit:ApplyDamage(damage)
			
			self.projectile:Release()
			self.projectile = nil
		end
	end)

	self.projectile:Fire()
end

return Holy_Javlin