require("scripts.core.ability")
require("scripts.core.game")
require("scripts.factory")

-- ability factory

AbilityFactory = class(Factory)

function AbilityFactory:GetFactoryName()
	return "ability"
end

function AbilityFactory:GetType()
	local ability = Ability({ entityId = self.entityId})
	Debug.Log("Running factory method for: " .. ability:GetType())
	return require("gamemode." .. SystemGame:GetGameMode() .. ".abilities." .. ability:GetType())
end

return AbilityFactory()
