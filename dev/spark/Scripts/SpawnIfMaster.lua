require "scripts.common"

local o={}


function o:OnActivate()
	if(self:IsAuthoritative()) then
		
		SpawnerComponentRequestBus.Event.Spawn(self.entityId);
	
	end
	Debug.Log("spawn if master:");
	--Debug.Log(dump(self));
end

return o;