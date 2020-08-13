require "scripts.core.hero"
HookHero = class(Hero)

function HookHero:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the hook hero!')
end

return HookHero
