local Effect={
	Properties =
	{
		TeamId = 0
	},
} 

function Effect:OnActivate()
	Debug.Log("Effect:OnActivate")
	
	self.tah= TriggerAreaNotificationBus.Connect(self,self.entityId);
end

function Effect:OnTriggerAreaEntered(unitId)	
	if not self:IsAuthoritative() then return end

	-- do checking team ID here 
	if  UnitRequestBus.Event.GetTeamId(unitId) == self.Properties.TeamId then
		--AudioRequestBus.Broadcast.PlaySound("");
		
		local targetUnit = Unit({ entityId = unitId})

		local modifier = targetUnit:FindModifierByTypeId("modifier_well_effect")

		if modifier == nil then
			modifier = targetUnit:AddNewModifier(targetUnit, self, "modifier_well_effect")	
		end		
		
	end
end

function Effect:OnTriggerAreaExited(entityId)
	if not self:IsAuthoritative() then return end
	
	local targetUnit = Unit({ entityId = entityId})
	
	if targetUnit:FindModifierByTypeId("modifier_well_effect") then
		local modifier = targetUnit:FindModifierByTypeId("modifier_well_effect")
		modifier:Destroy()
		modifier = nil
	end	
end

return Effect;
