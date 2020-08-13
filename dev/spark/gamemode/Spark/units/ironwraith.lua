require "scripts.core.hero"
IronWraith = class(Hero)

function IronWraith:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Iron Wraith!')
end

return IronWraith
