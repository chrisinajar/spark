require "scripts.GameUtils"
require "scripts/library/timers"

local Shrine = class(Unit);

function Shrine:OnActivate()
	

	CreateTimer(function() 
		self:Init() 
	end,0.1);

end

function Shrine:Init()
	Debug.Log("Shrine:Init")

    self.rightClickHandler = OnRightClickedNotificationBus.Connect(self,self.entityId)
    
    self.ability = GetId(CreateAbility("shrines/shrine"))  

    UnitAbilityRequestBus.Event.SetAbilityInSlot(self.entityId,Slot(Slot.Ability,0),self.ability); --so it's visible in the hud

    --prevents it from moving
    self:AddNewModifier(self,self.ability,"modifier_building_behavior")
    self:SetValue("movement_speed", 0);

    self:RegisterVariable("hp_percentage", 1);--health percentage: 1 is 100%
	self:RegisterVariable("hp_max",1000);
	self:RegisterDependentVariable("hp");

	self:RegisterVariable("base_armor", 1);
	self:RegisterDependentVariable("armor");
	self:RegisterDependentVariable("armor_reduction");
end


function Shrine:OnRightClickedFilter()
	Debug.Log("Shrine:OnRightClicked()");

	local unitId = SelectionRequestBus.Broadcast.GetMainSelectedUnit()


    if UnitRequestBus.Event.GetTeamId(unitId) == self:GetTeamId() then

        FilterResult(FilterResult.FILTER_PREVENT);

        local castContext = CastContext()
        castContext.Caster = unitId
        castContext.Target = self.entityId
        castContext.Ability = GetId(self.ability)
        castContext.BehaviorUsed = CastingBehavior(CastingBehavior.UNIT_TARGET)
        
        UnitRequestBus.Event.NewOrder(unitId,CastOrder(castContext),false);
    end    
    
    --if an enemy righ clicked go with the default behavior: attack it
end

function Shrine:OnDeactivate()
	self.rightClickHandler:Disconnect()
end

return Shrine;
