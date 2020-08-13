require "scripts.modifier";

modifier_push = class(Modifier)

LinkLuaModifier("modifier_push", modifier_push)

function modifier_push:OnCreated (options)
	self.speed = options.speed
	self.direction = self:GetParent():GetDirection()
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_push_1");
	self.OnTick = self.OnTick
	
	if not self.tickHandler then
		self.tickHandler = TickBus.Connect(self)
	end
end

function modifier_push:OnTick (deltaTime)
	local parent = self:GetParent()
	local origin = parent:GetPosition()
	parent:SetPosition(origin + (self.direction * self.speed * deltaTime))
end

function modifier_push:GetStatus ()
	return {
		UNIT_STATUS_ROOTED
	}
end

function modifier_push:OnDestroyed ()
	self.tickHandler:Disconnect()
end

return modifier_push
