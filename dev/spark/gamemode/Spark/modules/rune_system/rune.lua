require "scripts.GameUtils"
require "scripts/library/timers"

--require("gamemode.spark.abilities...runes.bounty_rune")

local Rune = 
{	
	Properties =
	{
		rune_type = {default = "bounty_rune", description = ""},
	},
}

function Rune:OnActivate()
	Debug.Log("Rune:OnActivate")

	CreateTimer(function() 
		self:Init() 
	end,0.1);

end

function Rune:Init()
	self.rightClickHandler = OnRightClickedNotificationBus.Connect(self,self.entityId)
	self.pickupAbility = CreateAbility("runes/"..self.Properties.rune_type)
end


function Rune:OnRightClickedFilter()
	Debug.Log("Rune:OnRightClicked()");

	FilterResult(FilterResult.FILTER_PREVENT);

	local unitId = SelectionRequestBus.Broadcast.GetMainSelectedUnit()

	if not HasTag(unitId,"hero") then return end

	local castContext = CastContext()
	castContext.Caster = unitId
	castContext.Target = self.entityId
	castContext.Ability = GetId(self.pickupAbility)
	castContext.BehaviorUsed = CastingBehavior(CastingBehavior.UNIT_TARGET)
	
	UnitRequestBus.Event.NewOrder(unitId,CastOrder(castContext),false);	
end

function Rune:OnDeactivate()
	if self.pickupAbility then
		self.pickupAbility:DetachAndDestroy();
		self.rightClickHandler:Disconnect()
	end
end

return Rune;
