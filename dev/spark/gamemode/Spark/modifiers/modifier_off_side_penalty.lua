require "scripts.modifier"
require "scripts.library.class"

modifier_off_side_penalty = class(Modifier)

LinkLuaModifier("modifier_off_side_penalty", modifier_off_side_penalty)

function modifier_off_side_penalty:OnCreated ()
end

function modifier_off_side_penalty:OnAttached()
	ModifierRequestBus.Event.SetIconTexture(self.entityId, "Textures/Core")
	self:RegisterVariable("current_stacked_amount", 0)
	self:RegisterVariable("InZone", 1)
	
	self:AddTimer(1.0,"1.0s timer")	
end

function modifier_off_side_penalty:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_off_side_penalty:OnTimerFinished(description)
	if self:GetValue("InZone") == 1 then
		self:SetValue("current_stacked_amount", self:GetValue("current_stacked_amount") + 1)
	elseif self:GetValue("InZone") == 0 then
		self:SetValue("current_stacked_amount", self:GetValue("current_stacked_amount") - 1)
	end
	
	if self:GetValue("InZone") == 0 and self:GetValue("current_stacked_amount") == 0 then
		self:Destroy(self.entityId)
	elseif self:GetValue("current_stacked_amount") >= 10 and self:GetValue("InZone") == 1 then
		local CurrentHP = self:GetParent():GetValue("hp")
		local MaxHP = self:GetParent():GetValue("hp_max")
		self:GetParent():SetValue("hp",CurrentHP - (MaxHP*0.05))
	end
	self:AddTimer(1.0,"1.0s timer")	
end

-- gamelift_host testhost 512map 2