require("scripts.factory")

-- creep factory

CreepFactory = class(Factory)

function CreepFactory:GetFactoryName()
	return "creep"
end

function CreepFactory:GetType()
	require("scripts.core.creep")
	local creep = Creep({
		entityId = self.entityId
	})
	local name = creep:GetName()
	Debug.Log("Running factory method for: " .. name)
	if name and #name > 0 then
		return require("gamemode." .. SystemGame:GetGameMode() .. ".units." .. name)
	else
		Debug.Log("ERROR - Spawning a unit with no name! this should never happen! (" .. tostring(name) .. ")")
		return Creep
	end
end

return CreepFactory()
