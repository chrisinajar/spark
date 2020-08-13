require("scripts.core.unit")
require("scripts.GameUtils")
require("scripts.core.variable")

Creep = class(Unit);


function Creep:OnCreated ()
	--Unit:OnActivate(self);

	VariableManagerRequestBus.Broadcast.RegisterVariableAndInizialize(VariableId(self.entityId,"hp_max"),200)

	Debug.Log("armor is : "..tostring(self:GetValue("armor")))
	--self:SetValue("movement_speed", 100);
	--AnimGraphComponentRequestBus.Event.SetNamedParameterFloat(self.entityId,"SpeedMultiplier", 0.13)--12);

	self:SetValue("projectile_speed", 30);
	self:SetValue("base_damage", 30);
	
	--self:RegisterVariable("cooldown_reduction", 0);

	--self:RegisterVariable("mana_percentage", 1);--mana percentage: 1 is 100%
	--self:RegisterDependentVariable("mana_regen");
	--self:RegisterDependentVariable("mana_max");
	--self:RegisterDependentVariable("mana");

	self:RegisterVariable("bounty", 1000);
	self:RegisterVariable("deathXP", 45);

	--self:RegisterVariable("experience", 0);
	--self:RegisterVariable("ability_points", 0);
	self:RegisterDependentVariable("level");
	--self:RegisterDependentVariable("experience_max");
	--self:RegisterDependentVariable("experience_progress");
	
	Debug.Log("end of Creep:OnActivate    unit name:"..self:GetName());
end

function Creep:OnDamageTaken(damage)
	Debug.Log("Creep:OnDamageTaken    "..damage:ToString());
end

function Creep:OnDestroyed ()
	--Unit:OnDeactivate(self);
	Debug.Log("Creep:OnDestroyed    unit name:"..self:GetName());
	if self.tickBusHandler ~= nil then
		self.tickBusHandler:Disconnect();
		self.tickBusHandler = nil
	end
end

function Creep:OnKilled(damage)
	--self:Destroy()
	local killer = Unit({ entityId = damage.source})
	if HasTag(killer, "hero") and damage.target == self:GetId() then
		killer:Give("gold", self:GetValue("bounty"))
		killer:Give("experience", self:GetValue("deathXP"))
	end
end

return require("scripts.components.creep");
