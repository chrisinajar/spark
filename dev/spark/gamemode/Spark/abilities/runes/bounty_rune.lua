require "scripts.core.ability"

local bounty_rune = class(Ability)


function bounty_rune:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function bounty_rune:OnSpellStart ()
	local bonus_gold = self:GetSpecialValue("bonus_gold");

    Debug.Log("bounty_rune:OnSpellStart giving "..bonus_gold.." bonus gold")
    
    local teamId = UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId())
    
    local teamHeroes = FilterArray( GetAllUnits(), function (unit)
		return unit:GetTeamId() == teamId and HasTag(unit,"hero")
	end);

	--reward them with the item
	for i=1,#teamHeroes do
		teamHeroes[i]:Give("gold",bonus_gold / #teamHeroes)
	end

	
	--self:GetCursorTarget():Destroy();
	GameManagerRequestBus.Broadcast.DestroyEntity(GetId(self:GetCursorTarget()))
	--self:DetachAndDestroy();
end

return bounty_rune
