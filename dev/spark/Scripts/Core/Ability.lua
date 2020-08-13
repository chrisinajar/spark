require "scripts.variableholder";
require "scripts.info"

Ability = class(VariableHolder, function (obj, ...)
	VariableHolder.init(obj, ...)

	obj.OnSpellStart = obj.OnSpellStart
	obj.OnAbilityPhaseStart = obj.OnAbilityPhaseStart
	obj.OnAbilityPhaseInterrupted = obj.OnAbilityPhaseInterrupted
	obj.OnChannelFinish = obj.OnChannelFinish
	obj.OnUpgrade = obj.OnUpgrade
	obj.OnAttached = obj.OnAttached
	obj.OnDetached = obj.OnDetached
end);

function Ability:SetCooldown(cooldown)
	self:RegisterVariable("cooldown_max",cooldown);
	self:RegisterDependentVariable("cooldown");
	self:RegisterVariable("cooldown_current",cooldown);
	if( not self:HasValue("cooldown_timer") ) then
		self:RegisterVariable("cooldown_timer");
	end
end

function Ability:GetLevel()
	return AbilityRequestBus.Event.GetLevel(self.entityId) or 0;
end

function Ability:SetLevel(level)
	if( level >= 0 and level <= Ability.GetMaxLevel(self)) then
		AbilityRequestBus.Event.SetLevel(self.entityId,level);
	end
end

function Ability:GetMaxLevel()
	if GetAbilityInfo(self.entityId) then
		return GetAbilityInfo(self.entityId).MaxLevel or 4;
	end
	return 4
end

function Ability:Upgrade ()
	if not Game:IsAuthoritative() then
		GameNetSyncRequestBus.Broadcast.SendNewOrder(self:GetCaster().entityId, UpgradeOrder(self.entityId), false)
		return
	end
	local owner = self:GetOwner()
	if owner and owner:GetValue("ability_points") > 0 then
		owner:Take("ability_points", 1)
	else
		Debug.Log('Not enough ability points! level up more, noob...')
		return
	end

	local level = Ability.GetLevel(self);
	if level < Ability.GetMaxLevel(self) then
		Ability.SetLevel(self, level + 1);
	else
		Debug.Log("Ability:Upgrade() : already at max level!");
	end
end


function Ability:GetCosts()
	return AbilityRequestBus.Event.GetCosts(self.entityId);
end
 
function Ability:SetCosts(costs)
	if( type(costs) == 'table' ) then
		local vector = vector_Amount();
		for k,v in pairs(costs) do
			if(type(v)=='number') then
				local cost=Amount(k,v);
				vector:push_back(cost);
			end
		end
		return AbilityRequestBus.Event.SetCosts(self.entityId,vector);
	else
		return AbilityRequestBus.Event.SetCosts(self.entityId,costs);
	end
end

function Ability:OnCreated () end
function Ability:OnDestroyed () end
function Ability:GetModifiers ()
	return {}
end

function Ability:OnActivate()
	Debug.Log("Ability:OnActivate() id is ["..tostring(self:GetId()).."]");
	
	if self:HasValue("cooldown_max") then
		self:SetCooldown(self:GetValue("cooldown_max"))
	end
	self.AbilityNotificationBusHandler = AbilityNotificationBus.Connect(self,self.entityId);

	if self:GetCaster() then
		self:AttachModifiers(self:GetCaster())
	end
end
function Ability:OnDeactivate()
end

function Ability:GetType()
	return AbilityRequestBus.Event.GetAbilityTypeId(self.entityId) or "";
end

function Ability:GetName()
	return AbilityRequestBus.Event.GetAbilityTypeId(self.entityId) or "";
end

function Ability:GetCastingBehavior()
	return AbilityRequestBus.Event.GetCastingBehavior(self.entityId);
end

function Ability:SetCastingBehavior(behavior)
	AbilityRequestBus.Event.SetCastingBehavior(self.entityId,behavior);
end

function Ability:GetCaster()
	local unitId = CastContextRequestBus.Event.GetCaster(self.entityId)
	if unitId and unitId:IsValid() then
		return Unit({ entityId = unitId });
	end
	return self:GetOwner()
end

function Ability:GetOwner()
	local unitId = AbilityRequestBus.Event.GetCaster(self.entityId)
	if unitId and unitId:IsValid() then
		return Unit({ entityId = unitId });
	end
	return nil
end

function Ability:GetCursorTarget()
	local unit_id = CastContextRequestBus.Event.GetCursorTarget(self.entityId); 
	if(unit_id:IsValid()) then return Unit({entityId=unit_id}); else return nil; end
end

function Ability:GetCursorPosition()
	return CastContextRequestBus.Event.GetCursorPosition(self.entityId); 
end

function Ability:GetBehaviorUsed()
	return CastContextRequestBus.Event.GetBehaviorUsed(self.entityId); 
end

function Ability:IsItem()
	local result = StaticDataRequestBus.Event.GetValue(self.entityId, "IsItem")
	Debug.Log('is item! ' .. tostring(result))
	return StaticDataRequestBus.Event.GetValue(self.entityId, "IsItem") == "true"
end

function Ability:OnAbilityPhaseStart()
	return true;
end

function Ability:OnSpellStart()

end

function Ability:OnAttached(unitId)
	self:AttachModifiers(unitId)
end

function Ability:DetachModifiers ()
	if not self.modifiers then
		return
	end
	for i,m in ipairs(self.modifiers) do
		m:Destroy()
	end
	self.modifiers = {}
end

function Ability:OnUpgrade ()
	Debug.Log('This ability has been upgraded!!')
	self:AttachModifiers(self:GetCaster())
end

function Ability:AttachModifiers (caster)
	self:DetachModifiers()

	if not caster or self:GetLevel() < 1 then
		return
	end

	local modifiers = self:GetModifiers()

	if not modifiers or #modifiers == 0 then
		return
	end

	local unit = Unit({entityId = GetId(caster) })
	self.modifiers = {}

	for i,modifierName in ipairs(modifiers) do
		Debug.Log('Attaching modifier ' .. modifierName)
		local modifier = unit:AddNewModifier(caster, self, modifierName, {})
		table.insert(self.modifiers, modifier)
	end
end

function Ability:GetSpecialValue (name)
	local value = StaticDataRequestBus.Event.GetSpecialValueLevel(self.entityId, name, self:GetLevel())
	
	
	if value==nil or value=="" then 
		return nil
	end

	return tonumber(value) or value
end

function Ability:DetachAndDestroy()
	local unitId = AbilityRequestBus.Event.GetCaster(self.entityId)
	if( unitId and unitId:IsValid()) then
		UnitAbilityRequestBus.Event.DetachAbility(unitId, self.entityId)
	end
	GameManagerRequestBus.Broadcast.DestroyEntity(self.entityId)
end






