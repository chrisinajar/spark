require "scripts.core.hero"
Minerva = class(Hero)

function Minerva:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Minerva!')
end

return Minerva
