
Variable = class()

function Variable:GetValue()
	return -1;
end

function Variable:GetMinValue()
	return 0;
end

function Variable:GetMaxValue()
	return -1;
end

function Variable:OnActivate(variableId)
	--Debug.Log(variableId);
	--if( self.VariableRequestBusHandler == nil ) then
	--	self.VariableRequestBusHandler = VariableRequestBus.Connect(self,variableId)
	--end
	
	--unit.entityId = self.entityId;
end

function Variable:OnDeactivate()
	Debug.Log("Variable OnDeactivate()");
	if( self.VariableRequestBusHandler ~= nil ) then
		self.VariableRequestBusHandler:Disconnect();
		self.VariableRequestBusHandler=nil;
	end
	
	--unit.entityId = self.entityId;
end

function Variable:GetId()
	return self.entityId;
end
