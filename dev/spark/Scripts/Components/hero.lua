require("scripts.factory")

-- hero factory

HeroFactory = class(Factory)

function HeroFactory:GetFactoryName()
	return "hero"
end

function HeroFactory:GetType()
	require("scripts.core.hero")
	local hero = Hero({
		entityId = self.entityId
	})
	local name = hero:GetName()
	Debug.Log("Running factory method for: " .. name)
	if name and #name > 0 then
		return require("gamemode." .. SystemGame:GetGameMode() .. ".units." .. name)
	else
		Debug.Log("ERROR - Spawning a unit with no name! this should never happen! (" .. tostring(name) .. ")")
		return Hero
	end
end

return HeroFactory()
