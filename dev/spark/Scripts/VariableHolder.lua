require "scripts.library.class"
require "scripts.library.listener"
require "scripts.common"

VariableHolder = class(EventListener);

function VariableHolder:OnActivate ()
	Debug.Log("VariableHolder:OnActivate()");
end

function VariableHolder:OnCreated () end
function VariableHolder:OnDestroyed () end

function VariableHolder:GetId ()
	return self.entityId;
end

--Variables
function VariableHolder:GetValue (id)
	return VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.entityId,id));
end

function VariableHolder:GetBaseValue (id)
	return VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.entityId,id));
end

function VariableHolder:GetBonusValue (id)
	return VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.entityId,id));
end

function VariableHolder:SetValue (id, value)
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(self.entityId,id),value);
end

function VariableHolder:Give (id, value)
	local varId = VariableId(self.entityId,id)
	VariableManagerRequestBus.Broadcast.SetValue(varId,VariableManagerRequestBus.Broadcast.GetValue(varId)+value);
end

function VariableHolder:Take (id, value)
	local varId = VariableId(self.entityId,id)
	VariableManagerRequestBus.Broadcast.SetValue(varId,VariableManagerRequestBus.Broadcast.GetValue(varId)-value);
end

function VariableHolder:HasValue (id)
	return VariableManagerRequestBus.Broadcast.VariableExists(VariableId(self.entityId,id));
end

function VariableHolder:AttachHandler (id, handler)
	local variableId=VariableId(self.entityId,id);

	--check if the variable exists
	if not VariableManagerRequestBus.Broadcast.VariableExists(variableId) then
		Debug.Log("error! To attach an handler to a variable the variable must exists")
		return;
	end

	self.variables = self.variables or {};
	self.VariableRequestBusHandler = self.VariableRequestBusHandler or {};

	if handler == nil then
		local filename = "gamemode." .. SystemGame:GetGameMode() ..  ".variables." .. id;
		--Check if the corrisponding file exists
		if not isModuleAvailable(filename) then 
			filename = "scripts.variables." .. id;
		end
		if isModuleAvailable(filename) then 
			handler = require(filename)();
		end
	end

	if handler == nil then
		Debug.Log("error! handler not specified and default one does not exist")
		return;
	end

	self.variables[id] = handler;
	self.variables[id].owner = self;

	--check if there is already an handler
	if (self.VariableRequestBusHandler[id]) then
		self.VariableRequestBusHandler[id]:Disconnect();
	end

	self.VariableRequestBusHandler[id] = VariableRequestBus.Connect(self.variables[id], variableId)
	
	--initialize it
	self:SetValue(id, self.variables[id]:GetValue(0,0))
end

function VariableHolder:RegisterDependentVariable(id,handler)
	local variableId=VariableId(self.entityId,id);
	local ok=false;
	--Register the variable
	ok=VariableManagerRequestBus.Broadcast.RegisterDependentVariable(variableId);

	if( ok )then
		self:AttachHandler(id,handler);
	else
		Debug.Log("error registering dependent variable with id="..id);
	end
end

function VariableHolder:RegisterVariable(id,initialValue)
	if not Game:IsAuthoritative() then
		return
	end

	local variableId=VariableId(self.entityId,id);

	if(VariableManagerRequestBus.Broadcast.VariableExists(variableId) == true)  then
		if( initialValue~=nil ) then
			self:SetValue(id,initialValue);
		end
		return;
	end
	
	local ok=false;
	--Register the variable
	ok=VariableManagerRequestBus.Broadcast.RegisterVariable(variableId);

	if( ok )then
		if(initialValue) then
			self:SetValue(id,initialValue);
		end
	else
		Debug.Log("error registering dependent variable with id="..id);
		return;
	end

	return variableId;
end

--[[
function VariableHolder:AttachVariableModifier(id)
	Debug.Log("VariableHolder:AttachVariableModifier()");
	self.variableBonusModifierBus = self.variableBonusModifierBus or {};
	if(self.variableBonusModifierBus[id] == nil) then
		Debug.Log("VariableHolder:AttachVariableModifier() attached");
		self.variableBonusModifierBus[id] = VariableBonusModifierBus.Connect(self,VariableId(self.entityId,id));	
	end
end

function VariableHolder:DetachVariableModifier(id)
	if(self.variableBonusModifierBus ~= nil and self.variableBonusModifierBus[id] ~= nil) then
		self.variableBonusModifierBus[id]:Disconnect();
	end
end

function VariableHolder:GetModifierBonus(id)
	if( self["GetModifierBonus_"..id] ~= nil) then
		return self["GetModifierBonus_"..id](self);
	else 
		return 0;
	end
end

--]]

function VariableHolder:OnDestroy()
	if self.VariableRequestBusHandler then
		for k,v in pairs(self.VariableRequestBusHandler) do
			self.VariableRequestBusHandler:Disconnect()
		end
	end
	EventListener.OnDestroy(self)
end
