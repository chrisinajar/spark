require "scripts.core.hero"
Harbinger = class(Hero)

function Harbinger:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Harbinger!')
end

return Harbinger
