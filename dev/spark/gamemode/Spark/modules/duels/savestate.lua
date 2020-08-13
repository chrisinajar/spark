
local SaveState = {}

function SaveState:SavePlayerState (player)
	state = {}

	local hero = player:GetHero()
	if hero then
		state.hero = {}
		state.hero.position = hero:GetPosition()
		state.hero.is_alive = hero:IsAlive()
		if state.hero.is_alive then
			state.hero.hp_percentage = hero:GetValue("hp_percentage")
			state.hero.mana_percentage = hero:GetValue("mana_percentage")
		end
	end

	return state
end

function SaveState:RestorePlayerState (player, state)
	local hero = player:GetHero()
	if not hero:IsAlive() then
		hero:Respawn()
	end
	if state.hero then
		-- restore hero
		if state.hero.position then
			hero:SetPosition(state.hero.position)
		end
		if state.hero.is_alive then
			hero:SetValue("hp_percentage", state.hero.hp_percentage)
			hero:SetValue("mana_percentage", state.hero.mana_percentage)
		else
			hero:SetValue("hp_percentage", 100)
			hero:SetValue("mana_percentage", 100)
		end
	end
end

return SaveState
