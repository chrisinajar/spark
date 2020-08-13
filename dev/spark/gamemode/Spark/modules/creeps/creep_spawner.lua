local json = require "scripts.json"
require "gamemode.Spark.modules.creeps.creep_types"
require "gamemode.Spark.modules.creeps.creep_power"

CreepSpawner = CreateModule("CreepSpawner", GAME_PHASE_GAME)

local CreepPowerLevel = 0

local NAME_ENUM = 1
local HEALTH_ENUM = 2
local MANA_ENUM = 3
local DAMAGE_ENUM = 4
local ARMOR_ENUM = 5
local GOLD_BOUNTY_ENUM = 6
local EXP_BOUNTY_ENUM = 7

function CreepSpawner:Init ()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("Creep_Spawn_Interval", 60)
	
	self:InitCreepSpawn()	
end

function CreepSpawner:SetPowerLevel(powerLevel)
	CreepPowerLevel = powerLevel
end

function CreepSpawner:CreepSpawnTimer()

end

function CreepSpawner:UpgradeCreeps()
	self:SetPowerLevel(CreepPowerLevel + 1)
end

function CreepSpawner:InitCreepSpawn()
	self.CreepSpawners = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("Creep_Spawner"))
	Debug.Log("CreepCamp:InitSpawnCamp")

	for i=1,#self.CreepSpawners do
		local spawner = self.CreepSpawners[i]
		
		local unitType = GetComponentProperty(spawner, "UnitType", "float")
		local maxUnits = GetComponentProperty(spawner, "MaxUnits", "float")
		local creepCategory = CreepTypes[unitType]
		local creepGroup = creepCategory[math.random(1, #creepCategory)] 
		
		for i=1, #creepGroup do 
			--Debug.Log("CREEP GROUP IS : " .. tostring(creepGroup[i]))
			local pos = TransformBus.Event.GetWorldTranslation(spawner);
			pos = MapRequestBus.Broadcast.GetNearestFreePosition(pos);
			--Debug.Log("SPAWN POS IS : " .. tostring(pos) .. "FOR UNIT " .. tostring(i))
			self:SpawnCreepInCamp(spawner, creepGroup[i], maxUnits, pos)			
		end
	end
	self:UpgradeCreeps()
	self.timer = CreateInterval(partial(self.SpawnCreeps, self), self.gameManager:GetValue("Creep_Spawn_Interval"))
end

function CreepSpawner:SpawnCreeps()
	--Debug.Log("CreepCamp:SpawnCamp")
	
	for i=1,#self.CreepSpawners do
		local spawner = self.CreepSpawners[i]
		
		local unitType = GetComponentProperty(spawner, "UnitType", "float")
		local maxUnits = GetComponentProperty(spawner, "MaxUnits", "float")
		local creepCategory = CreepTypes[unitType]
		local creepGroup = creepCategory[math.random(1, #creepCategory)] 
		
		--Debug.Log("NUMBER OF MAX UNITS OF AREA IS : " .. tostring(maxUnits))
		
		for i=1, #creepGroup do 
			--Debug.Log("CREEP GROUP IS : " .. tostring(creepGroup[i]))
			local pos = TransformBus.Event.GetWorldTranslation(spawner);
			pos = MapRequestBus.Broadcast.GetNearestFreePosition(pos);
			--Debug.Log("SPAWN POS IS : " .. tostring(pos) .. "FOR UNIT " .. tostring(i))
			
			self:SpawnCreepInCamp(spawner, creepGroup[i], maxUnits, pos)
		end
	end
	self:UpgradeCreeps()
end

function CreepSpawner:DoSpawn()

end

function CreepSpawner:SpawnCreepInCamp(Area, creepProperties, maximumUnits)--, pos)
	--Debug.Log("SPAWN CREEP IN CAMP STARTED")
	local units = TriggerAreaRequestsBus.Event.GetEntitiesInside(Area)
	--Debug.Log("NUMBER OF UNITS IN AREA IS : " .. tostring(#units))
	creepProperties = self:AdjustCreepPropertiesByPowerLevel( creepProperties, CreepPowerLevel )
	if maximumUnits and maximumUnits <= #units then
		for i = 1, #units do 
			local unitProperties = self:GetCreepProperties(units[i])
			local distributedScale = 1.0 / #units
	
			unitProperties = self:AddCreepPropertiesWithScale(unitProperties, 1.0, creepProperties, distributedScale)
			self:SetCreepPropertiesOnHandle(units[i], unitProperties)
		end
		--Debug.Log("CREEP STUFF FINISHED")
		return false
	end
	local pos = TransformBus.Event.GetWorldTranslation(Area);
	pos = MapRequestBus.Broadcast.GetNearestFreePosition(pos);
	--Debug.Log("SPAWNING CREEP")
	--Debug.Log("SPAWNING POS IS : " .. tostring(pos))
	local o = {}
	o.pos = {pos.x,pos.y,pos.z};
	o.tags = {}
	o.team = "neutrals"
	table.insert(o.tags,"creep")

	json_str = json.stringify(o);

	--Debug.Log("creep json: "..json_str)
	
	local creepHandle = GameManagerRequestBus.Broadcast.CreateUnitWithJson("creep",json_str);
	--Debug.Log("CREEP HANDLE IS : " .. tostring(creepHandle))
	
	if creepHandle ~= nil then			
		self:SetCreepPropertiesOnHandle(creepHandle, creepProperties)
		--creepHandle:AddNewModifier(nil, nil, "modifier_creep_loot", {locationString = locationString})
	end
	return true
end

function CreepSpawner:GetCreepProperties(creepHandle)
	local creepProperties = {}
	
	creepProperties[HEALTH_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "hp_max"))
	--Debug.Log("CREEP GET HEALTH IS : " .. creepProperties[HEALTH_ENUM])
	creepProperties[MANA_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "mana"))
	--Debug.Log("CREEP GET MANA IS : " .. creepProperties[MANA_ENUM])
	creepProperties[DAMAGE_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "damage")) / 2
	--Debug.Log("CREEP GET DAMAGE IS : " .. creepProperties[DAMAGE_ENUM])
	creepProperties[ARMOR_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "armor"))
	--Debug.Log("CREEP GET ARMOR IS : " .. creepProperties[ARMOR_ENUM])
	creepProperties[GOLD_BOUNTY_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "bounty"))
	--Debug.Log("CREEP GET GOLD IS : " .. creepProperties[GOLD_BOUNTY_ENUM])
	creepProperties[EXP_BOUNTY_ENUM] = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "deathXP"))
	--Debug.Log("CREEP GET EXP IS : " .. creepProperties[EXP_BOUNTY_ENUM])
	
	return creepProperties
end

function CreepSpawner:AdjustCreepPropertiesByPowerLevel(creepProperties, powerLevel)
	local adjustedCreepProperties = {}
	local creepPowerTable = CreepPower:GetPowerForMinute(powerLevel)
	
	adjustedCreepProperties[NAME_ENUM] = creepProperties[NAME_ENUM]
	--Debug.Log("CREEP ADJUST NAME IS : " .. adjustedCreepProperties[NAME_ENUM])
	adjustedCreepProperties[HEALTH_ENUM] = creepProperties[HEALTH_ENUM] * creepPowerTable[HEALTH_ENUM]
	--Debug.Log("CREEP ADJUST HEALTH IS : " .. adjustedCreepProperties[HEALTH_ENUM])
	adjustedCreepProperties[MANA_ENUM] = creepProperties[MANA_ENUM] * creepPowerTable[MANA_ENUM]
	--Debug.Log("CREEP ADJUST MANA IS : " .. adjustedCreepProperties[MANA_ENUM])
	adjustedCreepProperties[DAMAGE_ENUM] = creepProperties[DAMAGE_ENUM] * creepPowerTable[DAMAGE_ENUM]
	--Debug.Log("CREEP ADJUST DAMAGE IS : " .. adjustedCreepProperties[DAMAGE_ENUM])
	adjustedCreepProperties[ARMOR_ENUM] = creepProperties[ARMOR_ENUM] * creepPowerTable[ARMOR_ENUM]
	--Debug.Log("CREEP ADJUST ARMOR IS : " .. adjustedCreepProperties[ARMOR_ENUM])
	adjustedCreepProperties[GOLD_BOUNTY_ENUM] = creepProperties[GOLD_BOUNTY_ENUM] * creepPowerTable[GOLD_BOUNTY_ENUM]
	--Debug.Log("CREEP ADJUST GOLD IS : " .. adjustedCreepProperties[GOLD_BOUNTY_ENUM])
	adjustedCreepProperties[EXP_BOUNTY_ENUM] = creepProperties[EXP_BOUNTY_ENUM] * creepPowerTable[EXP_BOUNTY_ENUM]
	--Debug.Log("CREEP ADJUST EXP IS : " .. adjustedCreepProperties[EXP_BOUNTY_ENUM])		
	return adjustedCreepProperties
end

function CreepSpawner:AddCreepPropertiesWithScale(propertiesOne, scaleOne, propertiesTwo, scaleTwo)
	local CREEP_POWER_MAX = 1.5
	local addedCreepProperties = {}

	addedCreepProperties[HEALTH_ENUM] = propertiesOne[HEALTH_ENUM] * scaleOne + propertiesTwo[HEALTH_ENUM] * scaleTwo
	if addedCreepProperties[HEALTH_ENUM] > propertiesTwo[HEALTH_ENUM] * CREEP_POWER_MAX then
		addedCreepProperties[HEALTH_ENUM] = propertiesTwo[HEALTH_ENUM] * CREEP_POWER_MAX
	end
	--Debug.Log("CREEP ADJUST EXP IS : " .. propertiesOne[HEALTH_ENUM])
	--Debug.Log("CREEP ADJUST EXP IS : " .. tostring(scaleOne))
	--Debug.Log("CREEP ADJUST EXP IS : " .. propertiesTwo[HEALTH_ENUM])
	--Debug.Log("CREEP ADJUST EXP IS : " .. tostring(scaleTwo))
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[HEALTH_ENUM])
	addedCreepProperties[MANA_ENUM] = propertiesOne[MANA_ENUM] * scaleOne + propertiesTwo[MANA_ENUM] * scaleTwo
	if addedCreepProperties[MANA_ENUM] > propertiesTwo[MANA_ENUM] * CREEP_POWER_MAX then
		addedCreepProperties[MANA_ENUM] = propertiesTwo[MANA_ENUM] * CREEP_POWER_MAX
	end
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[MANA_ENUM])
	addedCreepProperties[DAMAGE_ENUM] = propertiesOne[DAMAGE_ENUM] * scaleOne + propertiesTwo[DAMAGE_ENUM] * scaleTwo
	if addedCreepProperties[DAMAGE_ENUM] > propertiesTwo[DAMAGE_ENUM] * CREEP_POWER_MAX then
		addedCreepProperties[DAMAGE_ENUM] = propertiesTwo[DAMAGE_ENUM] * CREEP_POWER_MAX
	end
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[DAMAGE_ENUM])
	addedCreepProperties[ARMOR_ENUM] = propertiesOne[ARMOR_ENUM] * scaleOne + propertiesTwo[ARMOR_ENUM] * scaleTwo
	if addedCreepProperties[ARMOR_ENUM] > propertiesTwo[ARMOR_ENUM] * CREEP_POWER_MAX then
		addedCreepProperties[ARMOR_ENUM] = propertiesTwo[ARMOR_ENUM] * CREEP_POWER_MAX
	end
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[ARMOR_ENUM])
	addedCreepProperties[GOLD_BOUNTY_ENUM] = propertiesOne[GOLD_BOUNTY_ENUM] * scaleOne + propertiesTwo[GOLD_BOUNTY_ENUM] * scaleTwo
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[GOLD_BOUNTY_ENUM])
	addedCreepProperties[EXP_BOUNTY_ENUM] = propertiesOne[EXP_BOUNTY_ENUM] * scaleOne + propertiesTwo[EXP_BOUNTY_ENUM] * scaleTwo
	--Debug.Log("CREEP ADJUST EXP IS : " .. addedCreepProperties[EXP_BOUNTY_ENUM])
	
	return addedCreepProperties
end

function CreepSpawner:SetCreepPropertiesOnHandle(creepHandle, creepProperties)
	--HEALTH
	local currentHealthMissing = VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "hp_max")) - VariableManagerRequestBus.Broadcast.GetValue(VariableId(creepHandle, "hp"))
	local targetHealth = creepProperties[HEALTH_ENUM]
	
	if currentHealthMissing > 0 then
		targetHealth = math.max(1, creepProperties[HEALTH_ENUM] - currentHealthMissing)
	end
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "hp_max"),math.ceil(creepProperties[HEALTH_ENUM]))
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "hp"),math.ceil(targetHealth))
	--Debug.Log("CREEP SET HEALTH IS : " .. creepProperties[HEALTH_ENUM])
	
	--MANA
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "mana"),math.ceil(creepProperties[MANA_ENUM]))
	--Debug.Log("CREEP SET MANA IS : " .. creepProperties[MANA_ENUM])
	
	--DAMAGE
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "damage"),math.ceil(creepProperties[DAMAGE_ENUM]))
	--Debug.Log("CREEP SET DAMAGE IS : " .. creepProperties[DAMAGE_ENUM])
	
	--ARMOR
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "base_armor"),math.ceil(creepProperties[ARMOR_ENUM]))
	--Debug.Log("CREEP SET ARMOR IS : " .. creepProperties[ARMOR_ENUM])
	
	--GOLD BOUNTY
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "bounty"),math.ceil(creepProperties[GOLD_BOUNTY_ENUM]))
	--Debug.Log("CREEP SET GOLD IS : " .. creepProperties[GOLD_BOUNTY_ENUM])
	
	--EXP BOUNTY
	VariableManagerRequestBus.Broadcast.SetValue(VariableId(creepHandle, "deathXP"),math.ceil(creepProperties[EXP_BOUNTY_ENUM]))
	--Debug.Log("CREEP SET EXP IS : " .. creepProperties[EXP_BOUNTY_ENUM])
end