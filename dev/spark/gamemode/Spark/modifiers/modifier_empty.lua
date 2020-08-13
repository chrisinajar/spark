modifier_empty = class(Modifier)

LinkLuaModifier("modifier_empty", modifier_empty)

function modifier_empty:OnCreated ()
	self:SetVisible(false)
end
