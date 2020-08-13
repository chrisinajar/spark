require "scripts.core.hero"
Muse = class(Hero)

function Muse:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Muse!')
end

return Muse
