require("scripts.core.item")
require("scripts.core.game")
require("scripts.factory")

-- item factory

ItemFactory = class(Factory)

function ItemFactory:GetFactoryName()
	return "item"
end

function ItemFactory:GetType()
	local item = Item()
	item.entityId = self.entityId
	Debug.Log("Running factory method for: " .. item:GetType())
	return Require("items." .. item:GetType())
end

return ItemFactory()
