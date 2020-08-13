require "scripts.library.zonecontrol"
require "scripts.library.ebus"
local DuelInstance = require "gamemode.spark.modules.duels.duel_instance"

Duels = CreateModule("Duels", GAME_PHASE_GAME)

function Duels:Init ()
	self.gameManager = GetGameManager()
	self.duelZone = ZoneControl("duel")
	self.duelLeftSpawn = ZoneControl("duelLeftSpawn")
	self.duelRightSpawn = ZoneControl("duelRightSpawn")

	Debug.Log('Starting a duel!!')
	Duels:StartDuel({
		full = true,
		first = true,
		timeout = 60
	})
end

function Duels:StartDuel (options)
	local playerList = self.gameManager.GetPlayerList()
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_duel");
	local leftPlayers = { }
	local rightPlayers = { }

	for _,player in ipairs(playerList) do
		local team = player:GetTeamId()
		player:SendClientEvent("notificationText", {
			text = "START DUEL", duration = 5, color = "red"
		})
		if GetTeamName(team) == "left" then
			table.insert(leftPlayers, player)
		elseif GetTeamName(team) == "right" then
			table.insert(rightPlayers, player)
		end
	end

	local playerCount = #playerList
	local participants = math.min(#rightPlayers, #leftPlayers)
	local split = math.floor(math.random() * participants)
	if options.full then
		split = participants
	end

	if participants < 1 then
		Debug.Log(' !! There are not enough players to have a duel!')
		return
	end

	local duel1Zones = self:GetZonesForPlayerCount(split)
	local duel2Zones = self:GetZonesForPlayerCount(participants - split)
	local duel1LeftPlayers = {}
	local duel1RightPlayers = {}
	local duel2LeftPlayers = {}
	local duel2RightPlayers = {}

	for _,player in ipairs(leftPlayers) do
		if #duel1LeftPlayers >= split and #duel2LeftPlayers < (participants - split) then
			table.insert(duel2LeftPlayers, player:GetPlayerId())
		end
		if #duel2LeftPlayers >= (participants - split) and #duel1LeftPlayers < split then
			table.insert(duel1LeftPlayers, player:GetPlayerId())
		end

		if #duel2LeftPlayers < (participants - split) and #duel1LeftPlayers < split then
			if math.random() > 0.5 then
				table.insert(duel1LeftPlayers, player:GetPlayerId())
			else
				table.insert(duel2LeftPlayers, player:GetPlayerId())
			end
		end
	end

	for _,player in ipairs(rightPlayers) do
		if #duel1RightPlayers >= split and #duel2RightPlayers < (participants - split) then
			table.insert(duel2RightPlayers, player:GetPlayerId())
		end
		if #duel2RightPlayers >= (participants - split) and #duel1RightPlayers < split then
			table.insert(duel1RightPlayers, player:GetPlayerId())
		end

		if #duel2RightPlayers < (participants - split) and #duel1RightPlayers < split then
			if math.random() > 0.5 then
				table.insert(duel1RightPlayers, player:GetPlayerId())
			else
				table.insert(duel2RightPlayers, player:GetPlayerId())
			end
		end
	end

	local endDuelsEvent = Event()
	local startDuelsEvent = Event()
	local duelCount = 1
	local function checkWon (winner)
		-- reward teams
		ScoreLimit:AddPoints(winner, 1)

		-- check end duels
		duelCount = duelCount - 1
		if duelCount == 0 then
			-- all duels are done
			endDuelsEvent.broadcast({})
		end
	end


	local duel = DuelInstance({
		zones = duel1Zones,
		leftPlayers = duel1LeftPlayers,
		rightPlayers = duel1RightPlayers,
		timeout = options.timeout or 90
	})
	endDuelsEvent.listen(partial(duel.End, duel))
	startDuelsEvent.listen(partial(duel.Start, duel))
	duel:OnDuelWon(checkWon)

	if participants - split > 0 then
		duelCount = 2
		duel = DuelInstance({
			zones = duel2Zones,
			leftPlayers = duel2LeftPlayers,
			rightPlayers = duel2RightPlayers,
			timeout = options.timeout or 90
		})
		endDuelsEvent.listen(partial(duel.End, duel))
		startDuelsEvent.listen(partial(duel.Start, duel))
		duel:OnDuelWon(checkWon)
	end




	duel:Start()
end

function Duels:GetZonesForPlayerCount (count)
	-- add other zones here
	return {
		duel = self.duelZone,
		leftSpawn = self.duelLeftSpawn,
		rightSpawn = self.duelRightSpawn,
	}
end
