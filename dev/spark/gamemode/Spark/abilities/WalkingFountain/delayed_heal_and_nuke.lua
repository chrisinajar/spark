require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_reduce_armor";

-- Stun Bolt ability
DelayedHealAndNuke = class(Ability)

local Z_OFFSET = 3

function DelayedHealAndNuke:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function DelayedHealAndNuke:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");

	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	
	CreateTimer(function()
		local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(target, self:GetSpecialValue("aoe"))
		if #units > 0 then
			for i = 1, #units do
				if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) and units[i] ~= self:GetCaster():GetId() then
					self.targetUnit = Unit({ entityId = units[i]})
					local damage = Damage();
					damage.type = Damage.DAMAGE_TYPE_MAGICAL;
					damage.damage = self:GetSpecialValue("damage");
					damage.source = GetId(self:GetCaster())
					self.targetUnit:ApplyDamage(damage)
				elseif UnitRequestBus.Event.GetTeamId(units[i]) == UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) then
					self.targetUnit = Unit({ entityId = units[i]})
					self.targetUnit:Give("hp", self:GetSpecialValue("heal"))
				end
			end
		end
	end,self:GetSpecialValue("delay"));
end

return DelayedHealAndNuke
