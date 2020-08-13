require "scripts.core.item"

Core_1 = class(Item)

function Core_1:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE,CastingBehavior.IMMEDIATE));
end

function Core_1:OnSpellStart()
	local caster = self:GetCaster()
	
end


return Core_1