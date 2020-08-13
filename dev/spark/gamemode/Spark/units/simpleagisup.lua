require "scripts.core.hero"
SimpleAgiSup= class(Hero)

function SimpleAgiSup:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the SimpleAgiSup!')
end

return SimpleAgiSup
