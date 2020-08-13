require "scripts.core.variable"

function CreateAttribute (name)
	local Attribute = class(Variable)

	local base_stat = "base_" .. name
	local gain_stat = "gain_" .. name

	function Attribute:GetValue ()
		return self.owner:GetValue(base_stat) + (self.owner:GetValue(gain_stat) * self.owner:GetValue("level"))
	end

	return Attribute
end
