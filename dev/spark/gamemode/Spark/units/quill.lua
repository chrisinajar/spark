require "scripts.core.hero"
Quill = class(Hero)

function Quill:OnCreated ()
	Hero.OnCreated(self)

	Debug.Log('Creating the Quill!')
end

return Quill
