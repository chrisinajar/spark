
require "scripts.library.ebus"

--local DebugRenderer = require "scripts.DebugRenderer";
local json = require "scripts.json"

local BossPit={
	Properties =
	{
		StartingTier = { default = 1.0 },
		PitType = { default = 1.0 },
	},
}

function BossPit:OnActivate()
	self.startingTierGetter = CreatePropertyGetter(self, "StartingTier", "float")
	self.pitTypeGetter = CreatePropertyGetter(self, "PitType", "float")
end

function BossPit:OnDeactivate()
	self.startingTierGetter()
	self.pitTypeGetter()
end

return BossPit;
