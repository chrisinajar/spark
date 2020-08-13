require "scripts.core.hero"
Astromage = class(Hero)

function Astromage:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Astromage!')
end

return Astromage