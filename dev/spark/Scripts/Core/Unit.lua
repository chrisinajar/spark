require "scripts.common";
require "scripts.variableholder";
require "scripts.modifier";
require "scripts.core.player";

Unit = class(VariableHolder)

function Unit:GetId ()
	return self.entityId;
end

function Unit:OnActivate ()
	Debug.Log("Unit:OnActivate name:" .. self:GetName().." id:"..tostring(self.entityId));

	self.OnNewOrder = self.OnNewOrder
	self.OnModifierAttached = self.OnModifierAttached
	self.OnModifierDetached = self.OnModifierDetached
	self.OnSpawned = self.OnSpawned
	self.OnDeath = self.OnDeath
	self.OnKilled = self.OnKilled
	self.OnDamageTaken = self.OnDamageTaken
	
	self.unitNotificationHanlder = UnitNotificationBus.Connect(self, self.entityId);
	self.damageNotificationHandler = OnDamageTakenNotificationBus.Connect(self, self.entityId)
	

	self:RegisterVariable("hp_percentage", 1);--health percentage: 1 is 100%
	self:RegisterDependentVariable("hp_regen");
	self:RegisterDependentVariable("hp_max");
	self:RegisterDependentVariable("hp");

	self:RegisterVariable("base_armor", 1);
	self:RegisterDependentVariable("armor");
	self:RegisterDependentVariable("armor_reduction");
	self:RegisterVariable("base_damage", 50);
	self:RegisterDependentVariable("damage");
	
	self:RegisterVariable("max_move_speed", 550);
	
	self:RegisterVariable("kills", 0);
	self:RegisterVariable("deaths", 0);
	self:RegisterVariable("assists", 0);

	local info = self:GetJson()
	Debug.Log("Got some info " .. tostring(info.attribute_primary))

	-- set up stats and functionality from static data in info
	if info and IsServer() then
		
	end
end

function Unit:GetJson()
	if self.__json then
		return self.__json
	end
	self.__json = GetUnitInfo(self:GetName())
	return self.__json
end

function Unit:OnDeactivated ()
	--todo disconnect everything
	Debug.Log("Unit:OnDeactivated");
	self.unitNotificationHanlder:Disconnect()
	self.damageNotificationHandler:Disconnect()
	self:OnDestroyed();
end

function Unit:Destroy() 
	UnitRequestBus.Event.Destroy(self.entityId)
end

function Unit:GetPosition ()
	return TransformBus.Event.GetWorldTranslation(self.entityId)
end

function Unit:SetPosition (position , updateZ)
	if updateZ==nil or updateZ==true then
		position.z = MapRequestBus.Broadcast.GetTerrainHeightAt(position)
	elseif updateZ==2 then
		position.z = math.max(position.z , MapRequestBus.Broadcast.GetTerrainHeightAt(position))
	end
	NavigationEntityRequestBus.Event.SetPosition(self.entityId, position);
	-- TransformBus.Event.SetWorldTranslation(self.entityId, position);
end

function Unit:GetRotationZ ()
	return NavigationEntityRequestBus.Event.GetRotationZ(self.entityId)
end

function Unit:SetRotationZ (rotation)
	NavigationEntityRequestBus.Event.SetRotationZ(self.entityId, rotation);
end

function Unit:GetDirection ()
	return GameManagerRequestBus.Broadcast.GetForwardDirection(self.entityId);
--[[
	local eulers = TransformBus.Event.GetLocalRotation(self.entityId);
	local radians = eulers.z - (1.5 * math.pi)
	if radians > math.pi then
		radians = radians - math.pi
	end
	if radians < 0 - math.pi then
		radians = radians + math.pi
	end
	return Vector3(math.cos(radians), math.sin(radians), 0)
--]]
end

function Unit:GetName()
	return UnitRequestBus.Event.GetName(self.entityId) or self:GetJson().type or "";

end

function Unit:GetTeamId()
	return UnitRequestBus.Event.GetTeamId(self.entityId) or nil;
end

function Unit:SetTeamId(teamId)
	UnitRequestBus.Event.SetTeamId(self.entityId,teamId)
end

function Unit:IsAlive()
	return UnitRequestBus.Event.IsAlive(self.entityId);
end
function Unit:SetAlive(alive)
	if(alive==nil) then alive=true; end
	UnitRequestBus.Event.SetAlive(self.entityId,alive);
end

function Unit:GetStatus ()
	return UnitRequestBus.Event.GetStatus(self.entityId);
end

function Unit:SetStatus (status)
	return UnitRequestBus.Event.SetStatus(self.entityId, status);
end

--Gameplay
function Unit:Die ()
	UnitRequestBus.Event.SetAlive(self.entityId,false);
end
function Unit:Kill ()
	self:Die()
end

function Unit:ApplyDamage(damage)
	damage.target = self.entityId
	OnDamageTakenRequestBus.Event.ApplyDamage(self.entityId,damage);
end


function Unit:HasAbilityInSlot(abilitySlot)
	return UnitAbilityRequestBus.Event.HasAbilityInSlot(self.entityId,abilitySlot);
end

function Unit:GetAbilityInSlot(abilitySlot)
	return UnitAbilityRequestBus.Event.GetAbilityInSlot(self.entityId,abilitySlot);
end

function Unit:SetAbilityInSlot(ability,abilitySlot)
	UnitAbilityRequestBus.Event.SetAbilityInSlot(self.entityId,abilitySlot,GetId(ability));
end

function Unit:ClearAbilitySlot(abilitySlot)
	UnitAbilityRequestBus.Event.ClearAbilitySlot(self.entityId,abilitySlot);
end

function Unit:HasAbility(ability)
	return UnitAbilityRequestBus.Event.HasAbility(self.entityId,GetId(ability))
end

function Unit:GetAbilitySlot(ability)
	return UnitAbilityRequestBus.Event.GetAbilitySlot(self.entityId,GetId(ability))
end

function Unit:DetachAbility(ability)
	return UnitAbilityRequestBus.Event.DetachAbility(self.entityId,GetId(ability))
end

function Unit:GetAbilities()
	return UnitAbilityRequestBus.Event.GetAbilities(self.entityId)
end

function Unit:HasAbilityType(abilityType)
	local abilityId = UnitAbilityRequestBus.Event.HasAbilityType(self.entityId,abilityType)
	if abilityId and abilityId:IsValid() then return abilityId end 
end


function Unit:GetFirstFreeItemSlot()
	for i=0,9 do
		local slot = Slot(Slot.Inventory,i)
		if not self:HasAbilityInSlot(slot) then
			return slot;
		end
	end
	return nil;
end

function Unit:GiveItem(item)
	local itemId = GetId(item)
	local firstFreeSlot = self:GetFirstFreeItemSlot()

	if itemId and itemId:IsValid() and firstFreeSlot then
		self:SetAbilityInSlot(itemId,firstFreeSlot)
	end	
end

function Unit:FindModifierByTypeId(id)
	local id = UnitRequestBus.Event.FindModifierByTypeId(self.entityId,id)
	if( id and id:IsValid()) then
		return Modifier({entityId=id});
	end
	return nil;
end

function Unit:AddNewModifier(caster,ability,modifierTypeId,options)
	local modifier = ModifiersHandler:AddNewModifier(caster,ability,modifierTypeId,options)
	UnitRequestBus.Event.AddModifier(self.entityId, GetId(modifier))

	return modifier
end

function Unit:RemoveModifier(modifier)
	UnitRequestBus.Event.RemoveModifier(self.entityId,GetId(modifier))
end

function Unit:ApplyDispel(dispel)
	UnitRequestBus.Event.ApplyDispel(self.entityId,dispel)
end

function Unit:GetPlayerOwner ()
	local playerEntityId = UnitRequestBus.Event.GetPlayerOwner(self.entityId)
	if playerEntityId then
		return Player({ entityId = playerEntityId })
	end
	return nil
end

function Unit:IsHero ()
	return HasTag(self.entityId,"hero")
end


return Unit;
