require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_shrine";

local Shrine = class(Ability)


function Shrine:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function Shrine:OnSpellStart ()
	Debug.Log("Shrine:OnSpellStart")
	
	self.shrineModifier = self:GetCaster():AddNewModifier(self:GetCaster(), self, "modifier_shrine", {duration = self:GetSpecialValue("duration")})

--[[local bonus_gold = self:GetSpecialValue("bonus_gold");

    Debug.Log("bounty_rune:OnSpellStart giving "..bonus_gold.." bonus gold")
    
    local teamId = UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId())
    
    local teamHeroes = FilterArray( GetAllUnits(), function (unit)
		return unit:GetTeamId() == teamId and HasTag(unit,"hero")
	end);

	--reward them with the item
	for i=1,#teamHeroes do
		teamHeroes[i]:Give("gold",bonus_gold / #teamHeroes)
	end]]
end

return Shrine
