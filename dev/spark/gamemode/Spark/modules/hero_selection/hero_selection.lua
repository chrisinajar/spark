require "scripts.library.zonecontrol"
require "scripts.library.ui_events"

HeroSelection = CreateModule("HeroSelection", GAME_PHASE_HERO_SELECT)

function HeroSelection:Init ()
	self.gameManager = GetGameManager()
	self.state = self.gameManager:GetGamePhase()
	self.heroSelectId = GameplayNotificationId(self.gameManager:GetId(), "SelectHero", "float")
	self.heroSelectHandler = ListenToUIEvent("SelectHero", partial(self.OnHeroSelected, self))
	self.players = {}
	self.hasStarted = false

	local playerList = self.gameManager.GetPlayerList()

	for _,player in ipairs(playerList) do
		self.players[player:GetPlayerId()] = false
	end
end

function HeroSelection:OnHeroSelected (hero, player)
	if not hero or hero == "" then
		return
	end
	local playerId = player:GetPlayerId()
	if self.players[playerId] then
		Debug.Log('Player has already chosen a hero!! ' .. hero)
		return
	end
	Debug.Log('Got a hero selection! ' .. hero)

	player:SetSelectedHero(hero)
	self.players[playerId] = true
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_ui_lockin_hero");
	self:CheckFinished()
end

function HeroSelection:CheckFinished ()
	if self.hasStarted then
		return
	end
	local hasAllPlayers = true
	for _,hasChosen in pairs(self.players) do
		if not hasChosen then
			hasAllPlayers = false
		end
	end

	if hasAllPlayers then
		self:StartGame()
	end
end

function HeroSelection:StartGame ()
	if self.hasStarted then
		return
	end
	self.hasStarted = true
	CreateTimer(function ()
		self.gameManager:SetGamePhase(GAME_PHASE_PRE_GAME)
	end, 2)
end
