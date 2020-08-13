local ShowCursor = {
	Properties = {
		--
	}
}

function ShowCursor:OnActivate ()
	if LyShineLua then
		LyShineLua.ShowMouseCursor(true)
	end
end

function ShowCursor:OnDeactivate ()
	if LyShineLua then
		LyShineLua.ShowMouseCursor(false)
	end
end

return ShowCursor
