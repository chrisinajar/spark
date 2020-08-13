require "scripts.core.variable"

local cooldown = class(Variable);

function cooldown:GetValue(deltaTime)
    local cooldown_reduction = 0;

    local unitId = AbilityRequestBus.Event.GetCaster(self.owner.entityId)
    if unitId and unitId:IsValid() then
        local unit = VariableHolder({entityId=unitId});
        cooldown_reduction = unit:GetValue("cooldown_reduction")
    end

    return self.owner:GetValue("cooldown_max") * ( 1 - cooldown_reduction);
end

return cooldown;
