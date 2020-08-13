require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_mana_potion";

Mana_Potion = class(Item)

function Mana_Potion:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	
	self:RegisterVariable("max_range",1000)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);

	self:RegisterVariable("stack_size",1);
end

function Mana_Potion:OnSpellStart()
	local caster = self:GetCaster()
	
	self.attachedUnit = self:GetCursorTarget()

	
	self.potionModifier = self.attachedUnit:FindModifierByTypeId("modifier_mana_potion")
	
	if self.potionModifier then
		self.potionModifier:SetValue("cooldown_timer",self:GetSpecialValue("duration"))
	else
		self.attachedUnit:AddNewModifier(caster, self, "modifier_mana_potion", {duration = self:GetSpecialValue("duration")})
	end

	local stack_size = self:GetValue("stack_size");
	if stack_size > 1 then
		self:SetValue("stack_size",stack_size-1)
	else
		self:DetachAndDestroy()
	end
end


return Mana_Potion
