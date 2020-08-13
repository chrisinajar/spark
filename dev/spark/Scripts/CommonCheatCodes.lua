local json = require "scripts.json"

function CommonCheatCodes(game,cmd)
    Debug.Log("Game:OnCommandFilter cmd="..cmd);
	
	cmd = trim(cmd)
	local first = string.sub(cmd, 1, 1)
	if first == "-" or first == "/" or first == "\\" then
		cmd = string.sub(cmd, 2)
	else 
		return false;
	end

	local args = splitstring(cmd)

	--global commands
	if(args[1] == "createunit") then
		
		local unitTypeId = tostring(args[2]);
		local json_str = args[3] or "{}";

		if unitTypeId == nil then
			Debug.Log("incorrect parameters! usage is : createunit <unitTypeId> [<json>]")
			return true;
		end

		if( game:IsAuthoritative() ) then
			local o = json.parse(json_str)
			if o.pos == nil then
				mouse = MouseCursorPositionRequestBus.Broadcast.GetSystemCursorPositionPixelSpace()
				local pos = SparkCameraRequestBus.Broadcast.TerrainRayCast(mouse)
				o.pos = {pos.x,pos.y,pos.z};
				json_str = json.stringify(o);
			end	

			GameManagerRequestBus.Broadcast.CreateUnitWithJson(unitTypeId,json_str);
		end

		return true;
	elseif(args[1] == "createunits") then
		
		local unitTypeId = tostring(args[2]);
		local num = tonumber(args[3]) or 1
		local json_str = args[4] or "{}";

		if unitTypeId == nil then
			Debug.Log("incorrect parameters! usage is : createunit <unitTypeId> [<json>]")
			return true;
		end

		if( game:IsAuthoritative() ) then
			local o = json.parse(json_str)
			if o.pos == nil then
				mouse = MouseCursorPositionRequestBus.Broadcast.GetSystemCursorPositionPixelSpace()
				local pos = SparkCameraRequestBus.Broadcast.TerrainRayCast(mouse)
				o.pos = {pos.x,pos.y,pos.z};
				json_str = json.stringify(o);
			end	

			for i=1,num do
				GameManagerRequestBus.Broadcast.CreateUnitWithJson(unitTypeId,json_str)
			end
		end

		return true;
	elseif args[1] == "wtf" then

		local on = not (args[2]=="false" or args[2]=='0')
		Debug.Log("wtf : "..tostring(on))
		
		game.wtf = on;

		return true;
	elseif args[1] == "killall" then
		Debug.Log("killing all units...  :'(")
		
		local units = GetAllUnits()
		for i, u in ipairs(units) do
			u:SetAlive(false)
		end
		return true
	elseif args[1] == "test" then
		local num = tonumber(args[2]) or 1
		Debug.Log("running test num "..tostring(num))
		
		--todo add test

		return true;
	end

	--unit related command
	local unit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
			
	if(unit and unit:IsValid()) then
		unit = Unit({entityId=unit})
	else
		Debug.Log("no unit selected");
		return true;
	end


	if(args[1] == "give") then

		local amount = tonumber(args[3]);
		if(args[2] and amount) then
			unit:RegisterVariable(args[2],  unit:GetValue(args[2])+amount);		
		else
			Debug.Log("incorrect parameters! usage is : give <variable_name> <amount>")
		end

		return true;
	elseif(args[1] == "refresh") then
		
		unit:SetAlive(true)
		unit:SetValue("hp_percentage", 1);
		unit:SetValue("mana_percentage", 1);

		return true;
	elseif(args[1] == "giveitem") then
		
		local itemTypeId = args[2]

		local itemId = GameManagerRequestBus.Broadcast.CreateItem(itemTypeId)
		local firstFreeSlot = unit:GetFirstFreeItemSlot()

		if itemId and itemId:IsValid() and firstFreeSlot then
			unit:SetAbilityInSlot(itemId,firstFreeSlot)
		end

		return true;
	end
	
	--units related command
	local unitsId = SelectionRequestBus.Broadcast.GetSelectionGroup()
	local units = {}

	if unitsId then
		for i=1,#unitsId do
			Debug.Log(tostring(unitsId[i]))
			if(unitsId[i] and unitsId[i]:IsValid()) then
				units[i] = Unit({entityId=unitsId[i]})
			end
		end
	end
	
	if(args[1] == "kill") then
		Debug.Log("killing the selected units...  :'(")

		for i=1,#units do
			units[i]:SetAlive(false)
		end
		
		return true;
	elseif(args[1] == "godmode") then
		for i=1,#units do
			units[i]:Give("movement_speed", 400)
			units[i]:Give("gold", 99999999)
			if not units[i]:HasAbilityType("dev_blink") then
				units[i]:GiveItem(CreateItem("dev_blink"));
			end
			if not units[i]:HasAbilityType("tempest_satchel") then
				units[i]:GiveItem(CreateItem("tempest_satchel"));
			end

			CommonCheatCodes(game,"/wtf");
		end
		
		return true;
	end

	Debug.Log("Game:OnCommandFilter unknown command!");
	return false;
end