require "scripts.core.ability"

ult_ward_behaviour = class(Ability)

function ult_ward_behaviour:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ult_ward_behaviour:OnSpellStart()
		local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(self:GetPosition(), self:GetSpecialValue("attack_range") or 70)
		local targetableUnits = {}
		for i = 1, #units do
			--if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) then
				table.insert(targetableUnits, units[i])
			--end
		end
		CreateTimer(function()
		self:FireProjectile(targetableUnits[1])
		end,0.2);
end

function ult_ward_behaviour:FireProjectile(unit)
	
	local target = unit
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

function ult_ward_behaviour:GetModifiers ()
	return {
		"modifier_ult_ward"
	}
end

return ult_ward_behaviour
