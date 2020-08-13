require "scripts.modifier";

modifier_status_silence = class(Modifier)

LinkLuaModifier("modifier_status_silence", modifier_status_silence)

function modifier_status_silence:GetStatus ()
	return {
		UNIT_STATUS_SILENCE
	}
end

return modifier_status_silence
