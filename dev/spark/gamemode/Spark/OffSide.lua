require "scripts.common"

local OffSide={
	Properties =
	{
		TeamId = 0
	},
} 

function OffSide:OnActivate()
	Debug.Log("OffSide:OnActivate")
	
	self.tah= TriggerAreaNotificationBus.Connect(self,self.entityId);
end

function OffSide:OnTriggerAreaEntered(unitId)	

	require "gamemode.Spark.modifiers.modifier_off_side_penalty";
	
	-- do checking team ID here 
	if UnitRequestBus.Event.GetTeamId(unitId) ~= self.Properties.TeamId and HasTag(unitId,"hero") then
		--AudioRequestBus.Broadcast.PlaySound("");
		
		local targetUnit = Unit({ entityId = unitId})

		local modifier = targetUnit:FindModifierByTypeId("modifier_off_side_penalty")

		if modifier == nil then
			modifier = targetUnit:AddNewModifier(targetUnit, self, "modifier_off_side_penalty")	
		end		
		
		modifier:SetValue("InZone",1)
	end
end

function OffSide:OnTriggerAreaExited(entityId)
	local targetUnit = Unit({ entityId = entityId})
	
	if targetUnit:FindModifierByTypeId("modifier_off_side_penalty") then
		local modifier = targetUnit:FindModifierByTypeId("modifier_off_side_penalty")
		modifier:SetValue("InZone",0)
	end	
end

return OffSide;
