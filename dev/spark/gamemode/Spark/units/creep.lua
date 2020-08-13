require "scripts.core.creep"

BaseCreep = class(Creep)

function BaseCreep:OnCreated ()
	Creep.OnCreated(self)

	Debug.Log('Creating a creep!')
end

return BaseCreep
