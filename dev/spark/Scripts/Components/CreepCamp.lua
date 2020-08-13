
require "scripts.library.timers"
require "scripts.library.ebus"

--local DebugRenderer = require "scripts.DebugRenderer";
local json = require "scripts.json"

local CreepCamp={
	Properties =
	{
		UnitType = { default=1.0 },
		MaxUnits = { default=4.0 },
	},
}

function CreepCamp:OnActivate()
	self.unitTypeGetter = CreatePropertyGetter(self, "UnitType", "float")
	self.maxUnitGetter = CreatePropertyGetter(self, "MaxUnits", "float")
end

function CreepCamp:OnDeactivate()
	self.unitTypeGetter()
	self.maxUnitGetter()
end

return CreepCamp;
