RuneSystem = CreateModule("RuneSystem", GAME_PHASE_GAME)

local spawnTicket = {};

function RuneSystem:Init()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("Rune_Spawn_Interval", 120)
	self.gameManager:RegisterVariable("First_Rune_Spawn_Time", 25)
	
	self.RuneSpawners = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("Rune_Spawner"))
	
	self:SpawnFirstRune()
end

function RuneSystem:SpawnFirstRune()
	for i = 1, #self.RuneSpawners do
		spawnTicket[i] = SpawnerComponentRequestBus.Event.Spawn(self.RuneSpawners[i])
	end
	self.timer = CreateInterval(partial(self.SpawnRunes, self), self.gameManager:GetValue("Rune_Spawn_Interval"))
end

function RuneSystem:SpawnRunes()
	self.Runes = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("rune"))
	self:RemoveAllRunes(self.Runes)
	for i = 1, #self.RuneSpawners do
		spawnTicket[i] = SpawnerComponentRequestBus.Event.Spawn(self.RuneSpawners[i])
	end
end


function RuneSystem:RemoveAllRunes(runes)	
	for i = 1, #runes do 
		GameEntityContextRequestBus.Broadcast.DestroyGameEntity(runes[i]);
	end
	
end