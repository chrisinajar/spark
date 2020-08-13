
ShopModule = CreateModule("ShopModule", GAME_PHASE_PRE_GAME)

function ShopModule:Init ()
	self.gameManager = GetGameManager()
	
	self:ListenToUIEvent("BuyItem")
end

function ShopModule:BuyItem (data)
	UnitRequestBus.Event.NewOrder(SelectionRequestBus.Broadcast.GetMainSelectedUnit(),BuyItemOrder(Item),false)
end

