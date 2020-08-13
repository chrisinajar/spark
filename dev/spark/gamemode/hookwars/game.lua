require "scripts.info";
require "scripts.core.unit"
Require("CommonCheatCodes")

Game={}

function Game:OnActivate (gameMode)
	self.gameMode = gameMode
	Debug.Log("Game:OnActivate()")
	Debug.Log("gamemode is " .. self.gameMode);

	if( self:IsAuthoritative() ) then
		GameManagerRequestBus.Broadcast.CreateUnitWithJson("bard",'{ "team":"left" }');
		GameManagerRequestBus.Broadcast.CreateUnitWithJson("bard",'{ "team":"right"}');
	end
	self.unitsHandler = UnitsNotificationBus.Connect(self);

	self.commandHandler = ConsoleNotificationBus.Connect(self);

	self.variableFilter = VariableManagerNotificationBus.Connect(self);

	Debug.Log("connected to filter")
end

function Game:OnDeactivate()
	Debug.Log("Game:OnDeactivate()")
	self.unitsHandler:Disconnect()
	self.commandHandler:Disconnect()
	self.variableFilter:Disconnect()
end

function Game:OnSetValueFilter(id,value)
	
	if self.wtf == true then
		if(id.variableId == "mana") then
			FilterResult(FilterResult.FILTER_PREVENT);
		end

		if(id.variableId == "cooldown_timer") then
			FilterResult(FilterResult.FILTER_PREVENT);
		end
	end
end



function Game:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function Game:OnUnitSpawned(unitId)
	local unit = Unit({entityId=unitId})

	Debug.Log("Game:OnUnitSpawned"..tostring(unitId));
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_respawn");
	local info = GetUnitInfo(unitId);

	local tags=vector_Crc32();
	tags:push_back(Crc32("spawn"));

	if(info and info.team) then	
		Debug.Log("unit's team is : "..info.team)
		tags:push_back(Crc32(info.team));
	else
		return;
	end

	local spawn = GameManagerRequestBus.Broadcast.GetEntitiesHavingTags(tags);
	
	if spawn and #spawn>=1 then
		Debug.Log("Game:OnUnitSpawned relocating the unit to the spawn")
		spawn = spawn[1];

		local new_pos = TransformBus.Event.GetWorldTranslation(spawn)
		unit:SetPosition(new_pos)
	else
		Debug.Log("Game:OnUnitSpawned spawn not found")
	end
end

function Game:OnUnitDeath(unitId)
	Debug.Log("Game:OnUnitDeath");

	--using the timer we only can pass a string.
	--for now there is no way to create a entityId from a string, so we need to do it like that
	self.deadUnits = self.deadUnits or {}
	table.insert(self.deadUnits,unitId)

	self:AddTimer(4,tostring(unitId))
end

function Game:OnTimerFinished(description)
	Debug.Log("OnTimerFinished "..description)
	
	
	for i=1,#self.deadUnits do
		if( tostring(self.deadUnits[i]) == description) then

			local unit = Unit({entityId=self.deadUnits[i]})
			--UnitRequestBus.Event.SetAlive(self.deadUnits[i],true)

			unit:SetAlive(true)
			unit:SetValue("hp_percentage", 1);
			unit:SetValue("mana_percentage", 1);

			break;
		end
	end

end



function Game:OnCommandFilter(cmd)
	return CommonCheatCodes(self,cmd)
end



return Game;