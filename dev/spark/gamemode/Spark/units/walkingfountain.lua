require "scripts.core.hero"
WalkingFountain = class(Hero)

function WalkingFountain:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the WalkingFountain!')
end

return WalkingFountain
