modifier_creep_behavior = class(Modifier)

LinkLuaModifier("modifier_creep_behavior", modifier_creep_behavior)

function modifier_creep_behavior:OnAttached ()
	-- self:SetVisible(false)
    self:ListenToDamageEvent("OnDamageTaken")
    Debug.Log("modifier_creep_behavior:OnCreated")

    self.origin = self:GetParent():GetPosition()

    self.thinkTimer = CreateInterval(partial(self.Think, self), 1)
end

function modifier_creep_behavior:OnDestroyed ()
	self:UnlistenToDamageEvent()
	if self.thinkTimer then
		return self.thinkTimer();
	end
end

function modifier_creep_behavior:IsDispellable(dispel)
    return false
end

function modifier_creep_behavior:Think()
	-- do stuff? leash?
end

function modifier_creep_behavior:OnDamageTaken(damage)
    Debug.Log("modifier_creep_behavior:OnDamageTaken")
	if damage.damage < 0 or damage.source == nil then
		return
	end
    local parent = self:GetParent()
    local aggroUnitRadius = 20
    local aggroHeroRadius = 15
    local attacker = GetEntityInstance(damage.source)
    local parentTeam = parent:GetTeamId()

    Debug.Log("modifier_creep_behavior:OnDamageTaken checking for units")

	local units = GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(parent:GetPosition(), aggroUnitRadius);
	local moreUnits = GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(attacker:GetPosition(), aggroHeroRadius);

	local function checkAggroUnit (unitId)
		local unit = GetEntityInstance(unitId)
		if unit:GetTeamId() == parentTeam then
    		Debug.Log("Adding aggro to unit!")
    		local modifier = unit:FindModifierByTypeId("modifier_creep_behavior_aggro")
    		if not modifier then
				modifier = unit:AddNewModifier(self, nil, "modifier_creep_behavior_aggro", {
					source = damage.source,
					origin = self.origin
				})
    		end
			modifier:SetDuration(5)	
		end
	end

	for i = 1, #units do
		checkAggroUnit(units[i])
	end
	for i = 1, #moreUnits do
		checkAggroUnit(moreUnits[i])
	end
end

modifier_creep_behavior_aggro = class(Modifier)
LinkLuaModifier("modifier_creep_behavior_aggro", modifier_creep_behavior_aggro)


function modifier_creep_behavior_aggro:OnAttached (unit)
	UnitRequestBus.Event.NewOrder(GetId(unit), AttackOrder(self.options.source), false);
end

function modifier_creep_behavior_aggro:OnDetached (oldParent)
	UnitRequestBus.Event.NewOrder(oldParent, MoveOrder(self.options.origin), false);
	Modifier.OnDetached(self, oldParent)
end


modifier_creep_behavior_should_leash = class(Modifier)
LinkLuaModifier("modifier_creep_behavior_should_leash", modifier_creep_behavior_should_leash)

function modifier_creep_behavior_should_leash:OnAttached (unit)
end
