
CapturePointModule = CreateModule("CapturePoint", GAME_PHASE_GAME)

require "gamemode.Spark.modules.scorelimit";
--local firstZones = {left = Vector(116, 361, 12), right = Vector(312, 361, 12),}

local NumberOfCaptures = 0

function CapturePointModule:Init ()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("PointsCaptured", 0)
	self.gameManager:RegisterVariable("HUDNotifyTimer", 60)
	self.gameManager:RegisterVariable("InitialCaptureTime", 420)
	self.gameManager:RegisterVariable("TimeBetweenPoints", 600)

	--self.timer = CreateInterval(partial(self.OnTimer, self), 1)
	self.CaptureSpawner = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("CapturePointSpawner"))
	local firstPointLeft = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("FirstPointLeft"))
	local firstPointRight = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("FirstPointRight"))
	self.FirstSpawnLeft = TransformBus.Event.GetWorldTM(firstPointLeft[1])
	self.FirstSpawnRight = TransformBus.Event.GetWorldTM(firstPointRight[1])
	
	CreateTimer(function ()
	 	Debug.Log('Starting Capture Point!!')
	 	self:SpawnFirstPoint()
	 end, self.gameManager:GetValue("InitialCaptureTime"))
	
end

function CapturePointModule:SpawnFirstPoint()
	local playerList = self.gameManager.GetPlayerList()

	self.spawnTicket = SpawnerComponentRequestBus.Event.SpawnAbsolute(self.CaptureSpawner[1], self.FirstSpawnLeft)
	self.spawnTicket = SpawnerComponentRequestBus.Event.SpawnAbsolute(self.CaptureSpawner[1], self.FirstSpawnRight)
	for _,player in ipairs(playerList) do
		player:SendClientEvent("notificationText", {
			text = "CAPTURE POINTS STARTED", duration = 5
		})
	end
	self.gameManager:SetValue("PointsCaptured", 2)
	CreateInterval(partial(self.SpawnPoint, self), self.gameManager:GetValue("TimeBetweenPoints"))
end

function CapturePointModule:SpawnPoint()
	local playerList = self.gameManager.GetPlayerList()
	
	self.spawnTicket = SpawnerComponentRequestBus.Event.SpawnAbsolute(self.CaptureSpawner[1], self.FirstSpawnLeft)
	self.spawnTicket = SpawnerComponentRequestBus.Event.SpawnAbsolute(self.CaptureSpawner[1], self.FirstSpawnRight)
	for _,player in ipairs(playerList) do
		player:SendClientEvent("notificationText", {
			text = "CAPTURE POINTS STARTED", duration = 5
		})
	end
	self.gameManager:SetValue("PointsCaptured", 2)
end

function CapturePointModule:OnCaptured(team)
	local CurrentCaptured = self.gameManager:GetValue("PointsCaptured")
	self.gameManager:SetValue("PointsCaptured", CurrentCaptured - 1)
	ScoreLimit:AddPoints(team, NumberOfCaptures + 1)
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_capturepoint");
	if self.gameManager:GetValue("PointsCaptured") == 0 then
		self:EndOfCapturePoints()
	end
end

function CapturePointModule:GetPointsCaptured()
	return self.gameManager:GetValue("PointsCaptured")
end

function CapturePointModule:GetNumberOfCaptures()
	return NumberOfCaptures
end

function CapturePointModule:HUDTimerTillNextPoint()

end

function CapturePointModule:EndOfCapturePoints()
	NumberOfCaptures = NumberOfCaptures + 1
	local playerList = self.gameManager.GetPlayerList()
	
	for _,player in ipairs(playerList) do
		player:SendClientEvent("notificationText", {
			text = "CAPTURE POINTS ENDED", duration = 5
		})
	end
end

function CapturePointModule:HUDCloseToSpawningTime()
	-- DISPLAY TEXT FOR CLOSE TO START OF CAPUTRE POINTS
end

