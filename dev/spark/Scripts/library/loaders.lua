require "scripts.core.ability"
require "scripts.core.item"
require "scripts.library.ebus"
require "scripts.library.globals"

CreateGlobalVariable("LuaLoaders", {
	currentTypeId = 1,
	registeredTypes = {},
	instances = {},
	factories = {}
})

function GetTypeId (typeName, factory)
	if LuaLoaders.registeredTypes[typeName] == nil then
		LuaLoaders.registeredTypes[typeName] = LuaLoaders.currentTypeId
		LuaLoaders.instances[LuaLoaders.currentTypeId] = {}
		LuaLoaders.currentTypeId = LuaLoaders.currentTypeId + 1
	end
	if factory then
		LuaLoaders.factories[LuaLoaders.registeredTypes[typeName]] = factory
	end

	return LuaLoaders.registeredTypes[typeName]
end

function RemoveEntityInstance (handle, typeId)
	if not LuaLoaders.instances[typeId] then
		return
	end
	LuaLoaders.instances[typeId][tostring(handle.entityId)] = nil
end

function SetEntityInstance (handle, typeId)
	VariableHolder.RegisterVariable(handle, "type", typeId)
	VariableHolder.SetValue(handle, "type", typeId)
	Debug.Log('Type was set to ' .. typeId)

	if not LuaLoaders.instances[typeId] then
		LuaLoaders.instances[typeId] = {}
	end
	LuaLoaders.instances[typeId][tostring(handle.entityId)] = handle
end

function GetEntityInstance (entityId)
	local typeId = VariableHolder.GetValue({ entityId = entityId }, "type")
	if not typeId or typeId == 0 then
		Debug.Log('Entity does not have a type id registered')
		return nil
	end

	if not LuaLoaders.instances[typeId] then
		LuaLoaders.instances[typeId] = {}
	end

	if not LuaLoaders.instances[typeId][tostring(entityId)] then
		if not LuaLoaders.factories[typeId] then
			Debug.Log("There is no factory for type " .. tostring(typeId))
		else
			return LuaLoaders.factories[typeId](entityId)
		end
	end

	return LuaLoaders.instances[typeId][tostring(entityId)]
end



function LuaLoaders:Activate ()
	-- units
	self.unitTypeId = GetTypeId("unit", partial(self.OnUnitCreated, self))
	self.abilityTypeId = GetTypeId("ability", partial(self.OnAbilityCreated, self))
	self.itemTypeId = GetTypeId("item", partial(self.OnItemCreated, self))

	Debug.Log('Initializing lua loaders')
	self.unitsHandler = UnitsNotificationBus.Connect(LuaLoaders);
	self.abilitiesHandler = AbilitiesNotificationBus.Connect(LuaLoaders);
end

function LuaLoaders:Deactivate ()
	self.unitsHandler:Disconnect()
	self.abilitiesHandler:Disconnect()

	LuaLoaders.currentTypeId = 1
	LuaLoaders.registeredTypes = {}
	LuaLoaders.instances = {}
	LuaLoaders.factories = {}
end

function LuaLoaders:OnUnitCreated (entityId, typeName)
	Debug.Log('Unit was created! trying our best to do the things... ' .. tostring(typeName) .. ' / ' .. tostring(entityId))

	return self:CreateInstance(self.unitTypeId, entityId, "units", typeName, Unit.GetName)
end

function LuaLoaders:OnAbilityCreated (entityId, typeName)
	if Ability.IsItem({ entityId = entityId }) then
		return LuaLoaders:OnItemCreated(entityId, typeName)
	end
	return self:CreateInstance(self.abilityTypeId, entityId, "abilities", typeName, Ability.GetName)
end

function LuaLoaders:OnItemCreated (entityId, typeName)
	return self:CreateInstance(self.itemTypeId, entityId, "items", typeName, Item.GetName)
end

function LuaLoaders:CreateInstance (typeId, entityId, category, typeName, getName)

	local override_lua_file = StaticDataRequestBus.Event.GetValue(entityId, "lua-file")
	if override_lua_file and override_lua_file ~= "" then
		local Type = Require(override_lua_file)
		if Type==nil then return end

		Debug.Log('Creating instance id:' .. tostring(entityId) .. ' type:' .. tostring(Type))
		local instance = Type({ entityId = entityId })
		SetEntityInstance(instance, typeId)

		instance:OnActivate()
		instance:OnCreated()

		return instance
	end

	if typeName and typeName ~= "" then
		local Type = Require(category .. "." .. typeName)
		if Type==nil then return end

		Debug.Log('Creating instance ' .. tostring(entityId) .. '/' .. tostring(Type))
		local instance = Type({ entityId = entityId })
		SetEntityInstance(instance, typeId)

		instance:OnActivate()
		instance:OnCreated()

		return instance
	end
	local instance = VariableHolder({ entityId = entityId })

	local typeName = StaticDataRequestBus.Event.GetValue(instance.entityId, "type")
	if typeName and typeName ~= "" then
		local Type = Require(category .. "." .. getName(instance))
		if Type==nil then return end
		
		setmetatable(instance, Type)
	end

	return instance
end
