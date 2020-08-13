require "scripts.core.hero"
OldMan = class(Hero)

function OldMan:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the OldMan!')
end

return OldMan
