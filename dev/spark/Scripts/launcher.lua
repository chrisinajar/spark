
local o={}

function o:OnActivate()
	GameManagerSystemRequestBus.Broadcast.LoadGameFile("gamemode/spark.txt");
	GameManagerSystemRequestBus.Broadcast.PlayGame();
	
	Debug.Log("launching game...");
end

return o;