require "scripts.core.ability"

Item = Ability;

--[[
require "scripts.variableholder";


Item = class(VariableHolder);

function Item:SetCooldown(cooldown)
	self:RegisterVariable("cooldown",cooldown);
	self:RegisterVariable("cooldown_current",cooldown);
	self:RegisterDependentVariable("cooldown_timer");
end

function Item:GetCosts()
	return AbilityRequestBus.Event.GetCosts(self.entityId);
end
 
function Item:SetCosts(costs)
	if( type(costs) == 'table' ) then
		local vector = vector_Cost();
		for k,v in pairs(costs) do
			if(type(v)=='number') then
				local cost=Cost(k,v);
				vector:push_back(cost);
			end
		end
		return AbilityRequestBus.Event.SetCosts(self.entityId,vector);
	else
		return AbilityRequestBus.Event.SetCosts(self.entityId,costs);
	end
end

function Item:OnCreated() end

function Item:OnActivate()
	Debug.Log("Item:OnActivate() id is ["..tostring(self:GetId()).."]");
	self.AbilityNotificationBusHandler = AbilityNotificationBus.Connect(self,self.entityId);
	self:OnCreated();
end

function Item:GetName()
	return AbilityRequestBus.Event.GetAbilityName(self.entityId) or "";
end

function Item:SetName(name)
	AbilityRequestBus.Event.SetAbilityName(self.entityId,name);
end

function Item:GetCastingBehavior()
	return AbilityRequestBus.Event.GetCastingBehavior(self.entityId);
end

function Item:SetCastingBehavior(behavior)
	AbilityRequestBus.Event.SetCastingBehavior(self.entityId,behavior);
end

function Item:GetCaster()
	local unit_id = CastContextRequestBus.Event.GetCaster(self.entityId); 
	if(unit_id:IsValid()) then return Unit({entityId=unit_id}); else return nil; end
end

function Item:GetCursorTarget()
	local unit_id = CastContextRequestBus.Event.GetCursorTarget(self.entityId); 
	if(unit_id:IsValid()) then return Unit({entityId=unit_id}); else return nil; end
end

function Item:GetCursorPosition()
	return CastContextRequestBus.Event.GetCursorPosition(self.entityId); 
end

function Item:IsItem()
	return true --CastContextRequestBus.Event.IsItem(self.entityId); 
end

function Item:OnItemPhaseStart()
	return true;
end

function Item:OnSpellStart()

end


--]]


