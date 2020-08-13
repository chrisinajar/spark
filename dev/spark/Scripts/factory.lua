require("scripts.variableholder")
require("scripts.library.timers")

Factory = class(function (obj, options)
	obj.Properties = obj.Properties or { }
	obj.NetRPCs = obj.NetRPCs or { }
	obj.OnActivate = obj.OnActivate
	obj.OnDeactivate = obj.OnDeactivate
	return obj
end)

function Factory:OnActivate ()
	self.typeId = GetTypeId(self:GetFactoryName())
	-- SetEntityInstance(self, self.typeId)
	-- wait for json data to be available
	-- self:CheckActivateInstance()
end

function Factory:CheckActivateInstance ()
	if self.instance then
		return
	end

	local typename = StaticDataRequestBus.Event.GetValue(self.entityId, "type")

	Debug.Log("Type name is " .. typename)

	if not typename or typename == "" then
		CreateTimer(function ()
			self:CheckActivateInstance()
		end, 0.1);
		return
	end

	local Type = self:GetType()
	local instance = Type()

	self.instance = instance
	self.OnDeactivate = self.OnDeactivate

	setmetatable(self, getmetatable(instance))
	instance.init(self)

	self:OnActivate()
	self:OnCreated()
end

function Factory:OnDeactivate ()
	-- RemoveEntityInstance(self.entityId, self.typeId)

	if self.instance then
		if self.instance.OnDeactivate then
			self.instance:OnDeactivate()
		end
		self.instance:OnDestroyed()
	end
end

function Factory:GetType ()
	return VariableHolder
end

return Factory
