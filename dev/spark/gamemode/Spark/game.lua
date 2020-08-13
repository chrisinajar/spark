require "scripts.info";
require "scripts.core.unit"
require "scripts.library.globals"
require "scripts.library.timers"
require "scripts.library.ebus"
require "scripts.library.module"
Require("CommonCheatCodes")

Game = {}

LoadModule("team_select");
LoadModule("scorelimit");
LoadModule("hudtimer");
LoadModule("duels");  -- with multiple navigation areas + unit HUD cause it to crash at the end of the duel, when units teleport back
LoadModule("capture_point");
LoadModule("rune_system");
LoadModule("gold");
LoadModule("hero_selection");
LoadModule("creeps");
LoadModule("boss");

function Game:OnActivate (gameMode)

	local screenSize = SettingsRequestBus.Broadcast.GetSettingValue("ScreenSize","Video")
	local text = SettingsRequestBus.Broadcast.GetSettingValue("DisplayMode","Video")
	local values=splitstring(screenSize, "x")
	if #values >= 2 then
		x = tonumber(values[1])
		y = tonumber(values[2])
		if text == "Borderless Window" or text == "Exclusive Fullscreen" then
			SettingsRequestBus.Broadcast.ChangeVideoMode(Vector2(x,y),true);
		else
			SettingsRequestBus.Broadcast.ChangeVideoMode(Vector2(x,y),false);
		end
	end
	--SettingsRequestBus.Broadcast.ChangeVideoMode(Vector2(1920,1080),true);

	self.gameMode = gameMode
	Debug.Log("Game:OnActivate()")
	Debug.Log("gamemode is " .. self.gameMode);

	GameManagerRequestBus.Broadcast.GetTeamIdByName("neutrals"); -- teamId = 0 -> neutrals   (so creating units with "createunit" and no option will create a neutral)
	GameManagerRequestBus.Broadcast.GetTeamIdByName("right");	 -- teamId = 1
	GameManagerRequestBus.Broadcast.GetTeamIdByName("left");	 -- teamId = 2
	
	CreateTimer(function ()
		-- local steamid = SteamWorksRequestBus.Broadcast.GetSteamID()
		local name = SteamWorksRequestBus.Broadcast.GetSteamName()

		Debug.Log('Found steam username ' .. name)
		Debug.Log('Local player id: ' .. tostring(GetGameManager():GetLocalPlayerId()))
		local unitList = InfoRequestBus.Broadcast.GetHeroList()
		for i=1, #unitList do
	        if unitList[i] then
	        	Debug.Log('Enabled hero ' .. unitList[i])
	        end
	    end
	end, 3)

	self.unitsHandler = UnitsNotificationBus.Connect(self);
	self.commandHandler = ConsoleNotificationBus.Connect(self);
	self.variableFilter = VariableManagerNotificationBus.Connect(self);
	self.gameManagerHandler = GameManagerNotificationBus.Connect(self);

	self.deadUnits = self.deadUnits or {}
	self.respawningUnits = self.respawningUnits or {}

	Debug.Log("connected to filter")

	self.playerAccountHandlers = {
		OnGetCurrentUserComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnGetCurrentUserComplete")
		end,
		OnSignUpComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnSignUpComplete")
		end,
		OnConfirmSignUpComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnConfirmSignUpComplete")
		end,
		OnResendConfirmationCodeComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnResendConfirmationCodeComplete")
		end,
		OnForgotPasswordComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnForgotPasswordComplete")
		end,
		OnConfirmForgotPasswordComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnConfirmForgotPasswordComplete")
		end,
		OnInitiateAuthComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnInitiateAuthComplete")
		end,
		OnRespondToForceChangePasswordChallengeComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnRespondToForceChangePasswordChallengeComplete")
		end,
		OnSignOutComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnSignOutComplete")
		end,
		OnChangePasswordComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnChangePasswordComplete")
		end,
		OnGlobalSignOutComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnGlobalSignOutComplete")
		end,
		OnDeleteOwnAccountComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnDeleteOwnAccountComplete")
		end,
		OnGetUserComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnGetUserComplete")
		end,
		OnVerifyUserAttributeComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnVerifyUserAttributeComplete")
		end,
		OnDeleteUserAttributesComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnDeleteUserAttributesComplete")
		end,
		OnUpdateUserAttributesComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnUpdateUserAttributesComplete")
		end,
		OnGetPlayerAccountComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnGetPlayerAccountComplete")
		end,
		OnUpdatePlayerAccountComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnUpdatePlayerAccountComplete")
		end,
		OnGetServiceStatusComplete = function ()
			Debug.Log("---------------")
			Debug.Log("---------------")
			Debug.Log("Got cloud gem player event!!!")
			Debug.Log("OnGetServiceStatusComplete")
		end,
	}
	
	CloudGemPlayerAccountNotificationBus.Connect(self.playerAccountHandlers)
	CloudGemPlayerAccountRequestBus.Broadcast.GetServiceStatus()
	CloudGemPlayerAccountRequestBus.Broadcast.GetCurrentUser()

	-- LocalizationRequestBus.Broadcast.SetLanguage("chineseT")
	-- s=LocalizationRequestBus.Broadcast.LocalizeString("@ui_Hello")
	-- s = "localizated string("..tostring(LocalizationRequestBus.Broadcast.GetLanguage()).."):"..s
	-- Debug.Log(s) 

	-- LocalizationRequestBus.Broadcast.SetLanguage("russian")
	-- s=LocalizationRequestBus.Broadcast.LocalizeString("@ui_Hello")
	-- s = "localizated string("..tostring(LocalizationRequestBus.Broadcast.GetLanguage()).."):"..s
	-- Debug.Log(s)

	-- LocalizationRequestBus.Broadcast.SetLanguage("English")
	-- local s=LocalizationRequestBus.Broadcast.LocalizeString("@ui_Hello")
	-- s = "localizated string("..tostring(LocalizationRequestBus.Broadcast.GetLanguage()).."):"..s
	-- Debug.Log(s)
end

function Game:OnGameManagerReady ()
	Debug.Log('Game manager is ready! now we can do basic things... we\'re in phase: ' .. GameManagerRequestBus.Broadcast.GetGamePhase())
	self.ready = true
	Game:OnGamePhaseChange(GameManagerRequestBus.Broadcast.GetGamePhase())
end

function Game:OnGamePhaseChange (phase)
	if not self.ready then
		return
	end
	Debug.Log('Hey look we actually got a new phase! ' .. tostring(phase))

	if phase ~= GameManagerRequestBus.Broadcast.GetGamePhase() then
		Debug.Log('But its the wrong phase!! ' .. phase .. ' vs ' .. GameManagerRequestBus.Broadcast.GetGamePhase())
		return
	end

	if phase == GAME_PHASE_CONFIGURE_GAME then
		self.TeamSelectCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/TeamSelectScreen.uicanvas")
	end

	if phase == GAME_PHASE_HERO_SELECT then
		Debug.Log('Showing hero selection!')
		if self.TeamSelectCanvas then
			UiCanvasManagerBus.Broadcast.UnloadCanvas(self.TeamSelectCanvas)
			self.TeamSelectCanvas = nil
		end
		self.HeroSelectCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/HeroPickingScreen.uicanvas")
	end

	if phase == GAME_PHASE_PRE_GAME then
		if self.HeroSelectCanvas then
			UiCanvasManagerBus.Broadcast.UnloadCanvas(self.HeroSelectCanvas)
			self.HeroSelectCanvas = nil
		end

		local gameManager = GetGameManager()
		if gameManager:GetLocalPlayerId() then
			MapRequestBus.Broadcast.ShowMapForTeam(gameManager:GetLocalPlayer():GetTeamId())  --wip
		end
	end
	
	if phase == GAME_PHASE_POST_GAME then
		self.EndScoreBoardCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/EndScoreBoard.uicanvas")
	end

	-- SERVER STUFF BELOW THIS LINE
	if not self:IsAuthoritative() then
		Debug.Log('But were not Authoritative so we do nothing')
		return
	end

	if phase == GAME_PHASE_HERO_SELECT then
		Debug.Log('Creating a fake player!')
		-- local bot = GetGameManager():CreateFakePlayer()
		-- CreateTimer(function ()
		-- 	HeroSelection:OnHeroSelected("astromage", bot)
		-- end, 1)
	end

	if phase == GAME_PHASE_PRE_GAME then
		self:SpawnUnits()

		local playerList = GetGameManager():GetPlayerList()

		local leftPlayers = { }
		local rightPlayers = { }

		Debug.Log('Selecting unit for the player')
		for _,player in ipairs(playerList) do
			local hero = player:GetHero()
			Debug.Log('hero is ' .. tostring(hero:GetName()))
			if hero then
				player:SelectUnit(hero:GetId())
				player:LookAtHero()
			end
		end
	end
end

function Game:OnDeactivate()
	Debug.Log("Game:OnDeactivate()")
	if self.tickbusHandler then 
		self.tickbusHandler:Disconnect()
	end
	self.unitsHandler:Disconnect()
	self.commandHandler:Disconnect()
	self.variableFilter:Disconnect()
	self.gameManagerHandler:Disconnect()
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

function Game:SpawnUnits()
	if( self:IsAuthoritative() ) then
			GameManagerRequestBus.Broadcast.CreateUnitWithJson("muse",'{ "team":"left","tags":["hero"]}');
			GameManagerRequestBus.Broadcast.CreateUnitWithJson("muse",'{ "team":"right","tags":["hero"]}');
			GameManagerRequestBus.Broadcast.CreateUnitWithJson("muse",'{ "team":"right","tags":["hero"]}');
	end
end

function Game:OnUnitSpawned(unitId)

	if not HasTag(unitId,"hero") then return end
	
	Debug.Log("Game:OnUnitSpawned start")
	local unit = GetEntityInstance(unitId)

	Debug.Log("Game:OnUnitSpawned " .. tostring(unitId) .. " named " .. unit:GetName());
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_respawn");
	local info = GetUnitInfo(unitId);

	local tags=vector_Crc32();
	tags:push_back(Crc32("spawn"));

	if(info and info.team) then	
		Debug.Log("unit's team is : "..info.team)
		tags:push_back(Crc32(info.team));

	-- allow untagged spawn on the left side
	else
	 	return;
	end

	if( self:IsAuthoritative() ) then
		local spawn = GameManagerRequestBus.Broadcast.GetEntitiesHavingTags(tags);
		
		if spawn and #spawn >= 1 then
			spawn = spawn[1];
			
			local new_pos = TransformBus.Event.GetWorldTranslation(spawn)
			new_pos = MapRequestBus.Broadcast.GetNearestFreePosition(new_pos);
			Debug.Log("Game:OnUnitSpawned relocating the unit to the spawn ("..tostring(new_pos)..")")
			unit:SetPosition(new_pos)
		else
			Debug.Log("Game:OnUnitSpawned spawn not found")
		end
	end
end

function Game:OnTick(deltaTime)
	for i=1,#self.deadUnits do
		local u = self.deadUnits[i]
		local position = TransformBus.Event.GetWorldTranslation(u)
		position.z = position.z - 1 *deltaTime
		TransformBus.Event.SetWorldTranslation(u, position);
	end
	if #self.deadUnits== 0 then
		self.tickbusHandler:Disconnect()
		self.tickbusHandler = nil
	end
end

function Game:OnUnitKilled(unitId, damage)
	Debug.Log("Game:OnUnitKilled " .. damage:ToString());
end

function Game:OnUnitDeath(unitId)
	if not self:IsAuthoritative() then
		return
	end

	Debug.Log("Game:OnUnitDeath");

	NavigationEntityRequestBus.Event.RemoveFromNavigationManager(unitId) -- so it doesn't block pathing anymore

	CreateTimer(function()
		table.insert(self.deadUnits,unitId)
		if self.tickbusHandler==nil then
			self.tickbusHandler = TickBus.Connect(self)
		end
	end, 2 )

	if HasTag(unitId,"hero") then
		--heroes do respawn

		--using the timer we only can pass a string.
		--for now there is no way to create a entityId from a string, so we need to do it like that
		self.respawningUnits = self.respawningUnits or {}
		table.insert(self.respawningUnits,unitId)

		self:AddTimer(4,tostring(unitId))
	else
		--otherwise destroy them after a delay, so the death animation can be played

		CreateTimer(function()
			for k,v in pairs(self.deadUnits) do
				if v == unitId then
					table.remove(self.deadUnits, k)
					break
				end
			end
			UnitRequestBus.Event.Destroy(unitId)
		end, 5 )
	end


end

function Game:OnTimerFinished(description)
	Debug.Log("OnTimerFinished "..description)
	
	
	for i=1,#self.respawningUnits do
		if( tostring(self.respawningUnits[i]) == description) then

			local unitId = self.respawningUnits[i]

			for k,v in pairs(self.deadUnits) do
				if v == unitId then
					table.remove(self.deadUnits, k)
					break
				end
			end

			local unit = Unit({entityId=unitId})
			--UnitRequestBus.Event.SetAlive(self.deadUnits[i],true)

			unit:SetAlive(true)
			NavigationEntityRequestBus.Event.AddToNavigationManager(unitId)
			unit:SetValue("hp_percentage", 1);
			unit:SetValue("mana_percentage", 1);

			break;
		end
	end

end


function Game:OnCommandFilter(cmd)
	if self:IsAuthoritative() then
		return CommonCheatCodes(self,cmd)
	end
end


return Game;