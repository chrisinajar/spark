modifier_thrill_seeker = class(Modifier)

LinkLuaModifier("modifier_thrill_seeker", modifier_thrill_seeker)

function modifier_thrill_seeker:OnCreated ()
	self:SetVisible(true)
	self.OnTick = self.OnTick
end

function modifier_thrill_seeker:OnAttached()
	self:AttachVariableModifier("movement_speed")
	self.enemyHeroes = FilterArray( GetAllUnits(), function (unit)
		return unit:GetTeamId() ~= self:GetParent():GetTeamId() and HasTag(unit,"hero")
	end);
	self.EnemiesEffectedCount = 0
	self.effectedEnemies = {}
	if not self.tickHandler then
		self.tickHandler = TickBus.Connect(self)
	end
end

function modifier_thrill_seeker:OnDestroyed()
	if self.tickHandler then
		self.tickHandler:Disconnect()
		self.tickHandler = nil
	end
end

function modifier_thrill_seeker:OnTick ()
	for i = 1, #self.enemyHeroes do
		if self.enemyHeroes[i] and self.enemyHeroes[i]:IsAlive() then
			local enemy = self.enemyHeroes[i]
			if enemy:GetValue("hp") < enemy:GetValue("hp_max") then
				--Debug.Log("ENEMY HP IS LESS THAN MAX HP")
				self.EnemiesEffectedCount = self.EnemiesEffectedCount + 1
				if #self.effectedEnemies ~= self.EnemiesEffectedCount then
					table.insert(self.effectedEnemies, enemy)
					self.boost = ((self:GetParent():GetValue("movement_speed") / 100) * self:GetAbility():GetSpecialValue("max_movement_speed_boost_percentage")) * #self.effectedEnemies
				end
				if enemy:GetValue("hp") <= (enemy:GetValue("hp_max")/100) * self:GetParent():GetValue("enemy_reveal_health_percentage") then
					-- do reveal of enemy
				else
					-- undo reveal of enemy
				end
			else
				if self.EnemiesEffectedCount > 0 then
					self.EnemiesEffectedCount = self.EnemiesEffectedCount - 1
					if #self.effectedEnemies ~= self.EnemiesEffectedCount then
						table.remove(self.effectedEnemies, enemy)
					end
				end
			end
		end
	end
end

function modifier_thrill_seeker:GetModifierBonus_movement_speed()
	return self.boost or 0
end