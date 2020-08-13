
function CreateGlobalVariable (name, defaultValue)
	if rawget(_G, name)  == nil then
		if defaultValue == nil then
			defaultValue = {}
		end
		rawset(_G, name, defaultValue)
	end
	return _G[name]
end
