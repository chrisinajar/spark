
require "scripts.info"

local shop={
	Properties =
	{
		ItemsList = "items" --by default sell all the items defined
	},
}

function Price(costs)
	local vector = vector_Amount();
	if( type(costs) == 'table' ) then
		for k,v in pairs(costs) do
			if(type(v)=='number') then
				local cost=Amount(k,v);
				vector:push_back(cost);
			end
		end
	end
	return vector;
end

function shop:OnActivate()
	Debug.Log("shop:OnActivate")
	
	self.tah= TriggerAreaNotificationBus.Connect(self,self.entityId);

	local type = self.Properties.ItemsList
	local gameModeInfo = GetGameModeInfo();

	local itemsList = GetJsonValueByPath(splitstring(type,"/"),gameModeInfo)

	if( itemsList == nil ) then return; end

	if itemsList[1]~=nil then --is probably a json array
		for i=1,#itemsList do
			ShopRequestBus.Event.AddItem(self.entityId,itemsList[i])
		end
	else --is probably a json object
		for k,v in pairs(itemsList) do
			ShopRequestBus.Event.AddItem(self.entityId,k)
		end
	end	
end

function shop:OnTriggerAreaEntered(unitId)
	Debug.Log("shop OnTriggerAreaEntered");
	
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_entershop");
	
	local shopId=GameManagerRequestBus.Broadcast.GetNearestShop(unitId);
	if( shopId and  shopId:IsValid() and self.entityId==shopId) then
		Debug.Log("unitId : in shop range");
	else
		Debug.Log("unitId : cannot get the shop");
	end
end
function shop:OnTriggerAreaExited(entityId)
	Debug.Log("shop OnTriggerAreaExited");
end

return shop;
