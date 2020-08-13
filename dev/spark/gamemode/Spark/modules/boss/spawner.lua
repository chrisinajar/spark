require "scripts.library.zonecontrol"
local BossList = require "gamemode.spark.modules.boss.boss_list"

BossSpawner = CreateModule("BossSpawner", GAME_PHASE_GAME)

function BossSpawner:Init ()
	Debug.Log("Starting the boss spawn timer...")
	self.bossPits = self:FindPits()

	self:SpawnAllBosses()
end

function BossSpawner:FindPits ()
	local spawnerList = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("boss_pit"))
	local spawners = {}

	for i = 1,#spawnerList do
		local spawner = spawnerList[i]
		local startTier = GetComponentProperty(spawner, "StartingTier", "float")
		local pitType = GetComponentProperty(spawner, "PitType", "float")
		local position = TransformBus.Event.GetWorldTranslation(spawner)
		table.insert(spawners, {
			entityId = spawner,
			startTier = startTier,
			pitType = pitType,
			position = position
		})
	end

	return spawners
end

function BossSpawner:SpawnAllBosses ()
	for _,pit in ipairs(self.bossPits) do
		Debug.Log('Found a pit with ' .. pit.pitType .. '/' .. pit.startTier .. ' at ' .. tostring(pit.position))
	end
end

function BossSpawner:SpawnBossInPit (pit)
	local spawnCount = pit.spawnCount or 0
	pit.spawnCount = spawnCount + 1
	local bossList = BossList[pit.pitType]
	local tier = math.min(#bossList, spawnCount + pit.startTier)
end
