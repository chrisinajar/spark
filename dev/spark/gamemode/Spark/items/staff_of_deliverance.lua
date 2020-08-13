require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_status_resistance"
require "gamemode.Spark.modifiers.modifier_all_stats"

Staff_Of_Deliverance = class(Item)

function Staff_Of_Deliverance:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function Staff_Of_Deliverance:OnSpellStart()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_vanish");
	local caster = self:GetCaster()	
	
    caster:ApplyDispel(Dispel())
    
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_appear");
end

function Staff_Of_Deliverance:GetModifiers ()
	return {
		"modifier_status_resistance",
		"modifier_all_stats"
	}
end

return Staff_Of_Deliverance
