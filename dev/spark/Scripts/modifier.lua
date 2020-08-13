require "scripts.library.class"
require "scripts.common"
require "scripts.library.ebus"
require "scripts.variableHolder"

Modifier =  class(VariableHolder);--class();


function Modifier:OnAttached(unitId) 
	Debug.Log("Modifier[the base class]:OnAttached() "..tostring(unitId));

	self:OnCreated(self.options)

	if self:GetStatus() then
		Debug.Log("Modifier[the base class]:OnAttached() has status");

		local unitStatusModifiers = ModifiersHandler.unitStatusModifiers[tostring(unitId)] 
		
		if unitStatusModifiers==nil then
			Debug.Log("Modifier[the base class]:OnAttached() creating unit status modifier entry");
			unitStatusModifiers = {}
			unitStatusModifiers["unitId"] = unitId
			ModifiersHandler.unitStatusModifiers[tostring(unitId)] 	= unitStatusModifiers
		end
		
		unitStatusModifiers[tostring(self.entityId)] = self
		--ModifiersHandler.statusModifiers = ModifiersHandler.statusModifiers or {}
		--ModifiersHandler.statusModifiers[tostring(self.entityId)] = self
		--table.insert(ModifiersHandler.statusModifiers, self)
		ModifiersHandler:EnforceModifiers(unitId)
	end

	if self.GetParticle and self:GetParticle() then
		Debug.Log('ModifiersHandler.particleModifiers')
		table.insert(ModifiersHandler.particleModifiers, self)
	end
end
function Modifier:OnDetached(oldParent) 
	Debug.Log("Modifier[the base class]:OnDetached()");

	local parentId = oldParent

	local unitStatusModifiers = ModifiersHandler.unitStatusModifiers[tostring(parentId)]
	
	if unitStatusModifiers then
		Debug.Log("deleting object from ModifiersHandler.unitStatusModifiers , enforcing modifiers")
		unitStatusModifiers[tostring(self.entityId)] = nil
		ModifiersHandler:EnforceModifiers(parentId)
	end
end

function Modifier:IsDispellable(dispel) 
	Debug.Log("Modifier[the base class]:IsDispellable()");
	return true
end

function Modifier:OnCreated()
	Debug.Log("Modifier[the base class]:OnCreated()");
end

function Modifier:OnDestroyed()
end

function Modifier:IsCaster()
	return GetId(self:GetCaster()) == GetId(self:GetParent())
end

function Modifier:SetDuration(duration)
	VariableManagerRequestBus.Broadcast.RegisterVariableAndInizialize(VariableId(self.entityId,"cooldown_current"),duration);
	VariableManagerRequestBus.Broadcast.RegisterVariableAndInizialize(VariableId(self.entityId,"cooldown_timer"),duration);
end

function Modifier:GetModifierTypeId()
	return ModifierRequestBus.Event.GetModifierTypeId(self.entityId);
end

function Modifier:SetModifierTypeId(type)
	ModifierRequestBus.Event.SetModifierTypeId(self.entityId,type);
end

function Modifier:GetParent ()
	local unitId = ModifierRequestBus.Event.GetParent(self.entityId)
	if unitId then
		return Unit({ entityId = unitId })
	end
	return nil
end

function Modifier:SetParent(parent)
	return ModifierRequestBus.Event.SetParent(self.entityId,GetId(parent));
end

function Modifier:GetCaster()
	local unitId = ModifierRequestBus.Event.GetCaster(self.entityId)
	if unitId then
		return Unit({ entityId = unitId })
	end
	return nil
end

function Modifier:SetCaster(caster)
	return ModifierRequestBus.Event.SetCaster(self.entityId,GetId(caster));
end

function Modifier:GetAbility()
	local abilityId = ModifierRequestBus.Event.GetAbility(self.entityId);
	if abilityId then
		return Ability({ entityId = abilityId })
	end
	return nil
end

function Modifier:SetAbility(ability)
	ModifierRequestBus.Event.SetAbility(self.entityId,GetId(ability));
end

function Modifier:IsAura()
	return ModifierRequestBus.Event.IsAura(self.entityId);
end

function Modifier:GetAuraRadius()
	return ModifierRequestBus.Event.GetAuraRadius(self.entityId);
end

function Modifier:SetAuraRadius(radius)
	ModifierRequestBus.Event.SetAuraRadius(self.entityId,radius);
end

function Modifier:RemoveAura()
	return ModifierRequestBus.Event.RemoveAura(self.entityId);
end

function Modifier:IsVisible()
	return ModifierRequestBus.Event.IsVisible(self.entityId);
end

function Modifier:SetVisible(visible)
	ModifierRequestBus.Event.SetVisible(self.entityId,visible);
end



function Modifier:AttachVariableModifier(id)
	self.variableBonusModifierBus = self.variableBonusModifierBus or {};
	if(self.variableBonusModifierBus[id] == nil) then
		self.variableBonusModifierBus[id] = VariableBonusModifierBus.Connect(self,VariableId(GetId(self:GetParent()),id));
		Debug.Log("AttachVariableModifier end");
	end
end

function Modifier:DetachVariableModifier(id)
	if(self.variableBonusModifierBus ~= nil and self.variableBonusModifierBus[id] ~= nil) then
		self.variableBonusModifierBus[id]:Disconnect();
	end
end

function Modifier:GetModifierBonus(id)
	if( self["GetModifierBonus_"..id] ~= nil) then
		local result = tonumber(self["GetModifierBonus_"..id](self))
		return result or 0
	else 
		Debug.Log("GetModifierBonus_"..id.." undefined : RETURNING 0")
		return 0;
	end
end

function Modifier:ListenToAttackEvent(id)
	self.attackListener = self.attackListener or {};
	if(self.attackListener[id] == nil) then
		self.attackListener[id] = AttackEventsNotificationBus.Connect(self,VariableId(GetId(self:GetParent()),id));	
	end
end

function Modifier:UnlistenToAttackEvent(id)
	if(self.attackListener ~= nil and self.attackListener[id] ~= nil) then
		self.attackListener[id]:Disconnect();
	end
end

function Modifier:OnAttackEvent(id,info)
	if( self[id] ~= nil) then
		local result = self[id](self,info);
		if result~=nil and type(result)=='boolean' then
			if result == true then
				FilterResult(FilterResult.FILTER_MODIFY,id,info);
			else
				FilterResult(FilterResult.FILTER_PREVENT);
			end
		end
	end
end


function Modifier:ListenToDamageEvent(priority)
	priority = tonumber(priority) or 10  --listener with the highest priority get called first

	if not self.damageListener then
		self.damageListener=OnDamageTakenNotificationBus.Connect(self,GetId(self:GetParent()))
	end	
	OnDamageTakenNotificationBus.Event.SetPriority(GetId(self:GetParent()),priority)
end

function Modifier:UnlistenToDamageEvent()
	if self.damageListener then
		self.damageListener:Disconnect();
	end	
end

function Modifier:OnDamageTakenFilter_(damage)
	if( self.OnDamageTaken ~= nil) then
		local result = self:OnDamageTakenFilter(damage);
		if result~=nil and type(result)=='boolean' then
			if result == true then
				FilterResult(FilterResult.FILTER_MODIFY,damage);
			else
				FilterResult(FilterResult.FILTER_PREVENT,damage);
			end
		end
	end
end


function Modifier:ListenToEvent(eventName,priority)
	if type(eventName)~='string' then
		Debug.Warning("called Modifier:ListenToEvent, but eventName(first param) is not a string!")
		return
	end

	if eventName:find("Damage") then
		if eventName:find("Dealt") then
			if not self.damageDealtListener then
				self.damageDealtListener=OnDamageDealtNotificationBus.Connect(self,GetId(self:GetParent()))
			end	
		else
			self:ListenToDamageEvent(priority)
		end
	elseif eventName:find("Attack") then
		self:ListenToAttackEvent(eventName);
	end
end

function Modifier:UnlistenToEvent(eventName)
	if type(eventName)~='string' then
		Debug.Warning("called Modifier:ListenToEvent, but eventName(first param) is not a string!")
		return
	end

	if eventName:find("Damage") then
		if eventName:find("Dealt") then
			if self.damageDealtListener then
				self.damageDealtListener:Disconnect()
			end	
		else
			self:UnlistenToDamageEvent(priority)
		end
	elseif eventName:find("Attack") then
		self:UnlistenToAttackEvent(eventName);
	end
end


function Modifier:GetStatus () 
	return false
end

function Modifier:ModifyAbility (ability, value, bonusHandler)
	self.abilityModifiers = self.abilityModifiers or {}
	self.disconnectAbilityHandlers = self.disconnectAbilityHandlers or Event()
	local abilityId = tostring(GetId(ability))
	self.abilityModifiers[abilityId] = self.abilityModifiers[abilityId] or {}
	if not self.abilityModifiers[abilityId][value] then
		local state = {
			handlers = {},
		}

		state.handler = {
			GetModifierBonus = function (id)
				local bonus = 0
				for i,cb in ipairs(state.handlers) do
					bonus = bonus + cb()
				end
				return bonus
			end
		}
		state.connection = VariableBonusModifierBus.Connect(state.handler, VariableId(GetId(ability), value))

		self.abilityModifiers[abilityId][value] = state

		state.unlisten = self.disconnectAbilityHandlers.listen(function ()
			state.unlisten()
			if state.connection then
				state.connection:Disconnect()
				state.connection = nil
			end
		end)
	end

	table.insert(self.abilityModifiers[abilityId][value].handlers, bonusHandler)
end


function Modifier:Destroy() 
	ModifierRequestBus.Event.Destroy(self.entityId)
end

function Modifier:OnDestroy()
	Debug.Log("Modifier:OnDestroy()");

	self:OnDestroyed()

	if(self.variableBonusModifierBus ~= nil) then
		for k,v in pairs(self.variableBonusModifierBus) do
			if(type(k)=='string') then
				if(v.Disconnect) then v:Disconnect(); end
			end
		end
	end
	if(self.attackListener ~= nil) then
		for k,v in pairs(self.attackListener) do
			if(type(k)=='string') then
				if(v.Disconnect) then v:Disconnect(); end
			end
		end
	end
	if self.otherHandlers then
		for k,v in pairs(self.otherHandlers) do
			if(v.Disconnect) then v:Disconnect(); end
		end
	end

	ModifiersHandler:RemoveModifier(self)

	if self.disconnectAbilityHandlers then
		self.disconnectAbilityHandlers.broadcast()
		self.disconnectAbilityHandlers.unlistenAll()
		self.disconnectAbilityHandlers = nil
	end
	self.abilityModifiers = nil

	VariableHolder.OnDestroy(self)
end

function Modifier:GetParticle ()
	return
end

function Modifier:SetParticle (particle)
	ModifierRequestBus.Event.SetParticle(self.entityId, particle)
end



ModifiersHandler = {
	activeModifiers = {},
	modifierLibrary = {},
	unitStatusModifiers = {},
};

function ModifiersHandler:OnActivate()
	ModifiersHandler.activeModifiers={};
	ModifiersHandler.modifierLibrary={};
	
	for k,v in pairs(ModifiersHandler.modifierLibrary) do
		Debug.Log(k);
	end
		
	ModifiersHandler.modifiersNotificationBusHandler = ModifiersNotificationBus.Connect(ModifiersHandler);

	ModifiersHandler.TickBusHandler = TickBus.Connect(ModifiersHandler);
	Debug.Log("called ModifiersHandler:OnActivate()");
end

function ModifiersHandler:OnTick(deltaTime, timePoint)
--[[	ModifiersHandler.activeModifiers=ModifiersHandler.activeModifiers or {};
	for k,v in ipairs(ModifiersHandler.activeModifiers) do
		if(type(v)=='table' and v.duration ~= nil and type(v.duration)=='number') then
			v.duration=v.duration-deltaTime;
			if(v.duration <= 0) then
				Debug.Log(k.." modifier detached");
				v:Destroy();
				table.remove(ModifiersHandler.activeModifiers,k);
			end
		end
	end
--]]

	self.timer = self.timer or 0
	self.timer = self.timer + deltaTime

	if( self.timer > 2.0) then
		-- Debug.Log("ModifiersHandler:OnTick()")
		self.timer = 0
	end

	--self:EnforceModifiers()
	--ModifiersHandler:EnforceModifiersForAllUnits()
end


function ModifiersHandler:EnforceModifiersForAllUnits ()
	for k,v in pairs(ModifiersHandler.unitStatusModifiers) do
		ModifiersHandler:EnforceModifiers(v.unitId)
	end
end


function ModifiersHandler:EnforceModifiers (unit)

	local unitId = GetId(unit)

	local unitStatusModifiers = ModifiersHandler.unitStatusModifiers[tostring(unitId)]

	if unitStatusModifiers==nil then return end

	local last_status = unitStatusModifiers["last_status"] or 0
	local old_status = UnitRequestBus.Event.GetStatus(unitId) or 0
	local new_status = 0

	for k,modifier in pairs(unitStatusModifiers) do
		local statuses = (type(modifier)=='table') and modifier.GetStatus and modifier:GetStatus() or {} 

		for i,status in ipairs(statuses) do
			new_status = bor(new_status, status)
		end
	end

	UnitRequestBus.Event.SetStatus(unitId,new_status)
	

	--if ModifiersHandler.statusModifiers[tostring(self.entityId)] then
		


	--[[
	ModifiersHandler.statusModifiers = ModifiersHandler.statusModifiers or {}
	ModifiersHandler.particleModifiers = ModifiersHandler.particleModifiers or {}
	ModifiersHandler.unitStatusChecks = ModifiersHandler.unitStatusChecks or {}
	local statusUnits = {}
	local unitIds = {}

	
		

	for k,modifier in ipairs(ModifiersHandler.statusModifiers) do
		local unitId = GetId(modifier:GetParent())
		if unitId and unitId:IsValid() then
			local unitStr = tostring(unitId)
			statusUnits[unitStr] = statusUnits[unitStr] or {}
			unitIds[unitStr] = unitId
			for i,st in ipairs(modifier:GetStatus()) do
				statusUnits[unitStr][st] = true
			end
		end
	end

	for k,modifier in ipairs(ModifiersHandler.particleModifiers) do
		local particle = modifier:GetParticle()
		if particle ~= modifier.__lastParticle then
			modifier.__lastParticle = particle
			modifier:SetParticle(particle)
		end
	end

	for k,unitId in ipairs(ModifiersHandler.unitStatusChecks) do
		local unitStr = tostring(unitId)
		statusUnits[unitStr] = statusUnits[unitStr] or {}
		unitIds[unitStr] = unitId
	end

	ModifiersHandler.unitStatusChecks = {}

	for unitIdStr,status in pairs(statusUnits) do
		local unit = Unit({ entityId = unitIds[unitIdStr] })
		if unit:IsAlive() then
			local statusValue = 0
			for k,v in pairs(status) do
				statusValue = statusValue + tonumber(k)
			end
			local oldStatus = unit:GetStatus()
			if oldStatus ~= statusValue then
				Debug.Log('Setting status to ' .. statusValue .. ' from ' .. oldStatus)
				unit:SetStatus(statusValue)
			end
		end
	end
	]]
end

function ModifiersHandler:OnDeactivate()
	Debug.Log("ModifiersHandler:OnDeactivate()");

	if ModifiersHandler.TickBusHandler ~= nil then
		ModifiersHandler.TickBusHandler:Disconnect();
	end
	
	for k,v in pairs(ModifiersHandler.activeModifiers) do
		if type(v)=='table' then
			if(v.Destroy) then v:Destroy(); end
			if v.notificationHandler and v.notificationHandler.Disconnect then
				v.notificationHandler:Disconnect()
				v.notificationHandler = nil
			end
		end
	end

	if ModifiersHandler.modifiersNotificationBusHandler then ModifiersHandler.modifiersNotificationBusHandler:Disconnect(); end
	
	ModifiersHandler.modifierLibrary = nil;
	ModifiersHandler.activeModifiers = nil;
	ModifiersHandler.statusModifiers = nil;
	ModifiersHandler.particleModifiers = nil;
end


function ModifiersHandler:OnModifierCreated(modifierId,modifierTypeId)
	Debug.Log("ModifiersHandler:OnModifierCreated")
	ModifiersHandler.modifierLibrary = ModifiersHandler.modifierLibrary or {};
	ModifiersHandler.activeModifiers = ModifiersHandler.activeModifiers or {};
	ModifiersHandler.statusModifiers = ModifiersHandler.statusModifiers or {};
	ModifiersHandler.particleModifiers = ModifiersHandler.particleModifiers or {};

	if( not ModifiersHandler.modifierLibrary[modifierTypeId] ) then
		Debug.Log("ModifiersHandler:OnModifierCreated  modifierTypeId:"..modifierTypeId.." is not valid")
		for k,v in pairs(ModifiersHandler.modifierLibrary) do
			Debug.Log(k)
		end
		Debug.Error("ModifiersHandler:OnModifierCreated  modifierTypeId:"..modifierTypeId.." is not valid")
		return;
	end

	local modifier = ModifiersHandler.modifierLibrary[modifierTypeId]({
		entityId = modifierId
	});

	-- setup methods to make sure they work for notifications
	modifier.OnCreated = modifier.OnCreated
	modifier.OnAttached = modifier.OnAttached
	modifier.OnDetached = modifier.OnDetached
	modifier.OnDestroy = modifier.OnDestroy
	modifier.GetModifierBonus = modifier.GetModifierBonus
	modifier.OnAttackEvent = modifier.OnAttackEvent
	modifier.OnDamageTakenFilter = modifier.OnDamageTakenFilter
	modifier.IsDispellable = modifier.IsDispellable

	-- options mounting...
	ModifiersHandler.modifiersOptions = ModifiersHandler.modifiersOptions or {}
	if ModifiersHandler.currentModifierOptions == nil and ModifiersHandler.modifiersOptions[modifierId] ~= nil then
		modifier.options = ModifiersHandler.modifiersOptions[modifierId]
		ModifiersHandler.modifiersOptions[modifierId] = nil
	else
		modifier.options = ModifiersHandler.currentModifierOptions
		ModifiersHandler.currentModifierOptions = nil
	end

	-- OnAttached happens after this event, so we just use the notifications for em...
	modifier.notificationHandler = ModifierNotificationBus.Connect(modifier, modifier.entityId)
	--table.insert(ModifiersHandler.activeModifiers, modifier);
	ModifiersHandler.activeModifiers[tostring(modifier.entityId)] = modifier
	Debug.Log("modifier with id="..modifierTypeId.." created! (in lua space)");	
	-- modifier:OnCreated(modifier.options)
end

function LinkLuaModifier(id,modifier)
	Debug.Log("called LinkLuaModifier with id="..id);	
	ModifiersHandler.modifierLibrary=ModifiersHandler.modifierLibrary or {};
	if( not ModifiersHandler.modifierLibrary[id] ) then
		ModifiersHandler.modifierLibrary[id]=modifier;
		Debug.Log("		added LinkLuaModifier with id="..id);	
		for k,v in pairs(ModifiersHandler.modifierLibrary) do
			Debug.Log(k);
		end
	end
end

function ModifiersHandler:AddNewModifier(caster,ability,id,options)
	local s=ModifiersHandler;--parent.modifiers;
	Debug.Log("called AddNewModifier with id="..id);	

	if(s == nil ) then Debug.Log("ModifiersHandler is nil"); return; end

	ModifiersHandler.modifierLibrary=ModifiersHandler.modifierLibrary or {};
	ModifiersHandler.activeModifiers=ModifiersHandler.activeModifiers or {};

	if( not ModifiersHandler.modifierLibrary[id] ) then
		ModifiersHandler.modifierLibrary[id] = false
		Require("modifiers."..id);
	end
	
	if( ModifiersHandler.modifierLibrary[id] ) then
		local casterId = caster
		
		ModifiersHandler.currentModifierOptions = options

		local modifierId = GameManagerRequestBus.Broadcast.CreateModifier(GetId(caster),GetId(ability),id);

		if  not ModifiersHandler.activeModifiers[tostring(modifierId)] then
			Debug.Error("modifier is not inizialized yet")
		end
		
		local modifier = ModifiersHandler.activeModifiers[tostring(modifierId)] or modifierId

		-- if it's not nil then it was loaded asyncronously, which is actually a little easier anyway...
		if ModifiersHandler.currentModifierOptions ~= nil then
			ModifiersHandler.currentModifierOptions = nil
			ModifiersHandler.modifiersOptions = ModifiersHandler.modifiersOptions or {}
			ModifiersHandler.modifiersOptions[modifierId] = options
		end

		if(modifierId and modifierId:IsValid()) then
			--set cooldown
			if(type(options)=='table' and options.duration ~= nil and type(options.duration)=='number') then
				Debug.Log("setting modifier cooldown")
				modifier:SetDuration(options.duration)
			end
			local count = 0
			for k,v in pairs(ModifiersHandler.activeModifiers) do
				count = count + 1
			end

			Debug.Log("modifier with id="..id.." created!  (total : "..tostring(count).." )");	
			return modifier--modifierId;
		end
	end

	Debug.Log("modifier with id="..id.." is not defined. Available modifiers are:");	
	for k,v in pairs(ModifiersHandler.modifierLibrary) do
		Debug.Log(k);
	end
	return nil;
end

function ModifiersHandler:RemoveModifier (modifier)
	ModifiersHandler.activeModifiers =  ModifiersHandler.activeModifiers or {}
	ModifiersHandler.statusModifiers =  ModifiersHandler.statusModifiers or {}
	ModifiersHandler.particleModifiers = ModifiersHandler.particleModifiers or {};
	ModifiersHandler.unitStatusChecks =  ModifiersHandler.unitStatusChecks or {}

	
	ModifiersHandler.activeModifiers[tostring(GetId(modifier))]=nil

	local count = 0
	for k,v in pairs(ModifiersHandler.activeModifiers) do
		count = count + 1
	end

	Debug.Log("deleting object from ModifiersHandler.activeModifiers (new total : "..tostring(count).." )")

	--[[for k,v in ipairs(ModifiersHandler.activeModifiers) do
		if v.entityId == modifier.entityId then
			Debug.Log("deleting object from ModifiersHandler.activeModifiers")
			table.remove(ModifiersHandler.activeModifiers, k);
			break;
		end
	end

	for k,v in ipairs(ModifiersHandler.statusModifiers) do
		if v.entityId == modifier.entityId then
			Debug.Log("deleting object from ModifiersHandler.statusModifiers")
			table.remove(ModifiersHandler.statusModifiers, k);
			table.insert(ModifiersHandler.unitStatusChecks, GetId(modifier:GetParent()))
			break;
		end
	end
	]]

	for k,v in ipairs(ModifiersHandler.particleModifiers) do
		if v.entityId == modifier.entityId then
			Debug.Log("deleting object from ModifiersHandler.particleModifiers")
			table.remove(ModifiersHandler.particleModifiers, k);
			break;
		end
	end

	-- has to be done syncronously
	-- if its not, when next tick runs and it tries to reference the entityid then it will already
	-- be cleaned up from memory and then it'll get a nil status and break
	--self:EnforceModifiers(modifier:GetParent())

end


function ModifiersHandler:CreateModifierThinker(caster,ability,id,options,origin)
	local modifier = ModifiersHandler:AddNewModifier(caster,ability,id,options)
	
	--UnitRequestBus.Event.AddModifier(GetId(caster), modifier)

	local modifierId = GetId(modifier)

	Debug.Log("modifier thinker id is :"..tostring(modifierId))
	origin = origin or TransformBus.Event.GetWorldTranslation(GetId(caster))

	TransformBus.Event.SetParent(modifierId,EntityId())

	ModifierRequestBus.Event.SetDoesStack(modifierId,false)

	NavigationEntityRequestBus.Event.SetPosition(modifierId,origin)
	TransformBus.Event.SetWorldTranslation(modifierId,origin)

	modifier:SetAuraRadius(options and options.radius or 20)

	return modifier
end


function ModifiersHandler:CreateModifierThinkerProjectile(caster,ability,id,options,origin)
	
	local projectile = ModifiersHandler:CreateModifierThinker(caster,ability,id,options,origin)

	local entityId = GetId(projectile)

	NavigationEntityRequestBus.Event.RemoveFromNavigationManager(entityId)

	function projectile:Fire()
		NavigationEntityRequestBus.Event.AddToNavigationManager(entityId)
	end

	return projectile
end



return ModifiersHandler;
