
Require("GameUtils")

--more like ward stack
observer_ward = class(Ability)

function observer_ward:OnCreated ()
	self:RegisterVariable("max_range", 500)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0) 

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET,CastingBehavior.UNIT_TARGET));

	self:SetLevel(1)
end


function observer_ward:OnSpellStart ()
    
    local behavior = self:GetBehaviorUsed()

    if behavior:Contains(CastingBehavior.POINT_TARGET) then
        Debug.Log("placing ward")
        local position = self:GetCursorPosition()
        local caster   = self:GetCaster()

        local ward = CreateUnit("monkey_ward")

        --TransformBus.Event.SetLocalRotation(GetId(ward),TransformBus.Event.GetWorldRotation(GetId(caster)))
        ward:SetRotationZ(caster:GetRotationZ())
        ward:SetPosition(position)
        ward:SetTeamId(caster:GetTeamId())
        ward:RegisterVariable("vision_range", 30);
        ward:RegisterVariable("vision_type", VISIBILITY_VISIBLE);


        
    else
        Debug.Log("toogling ward type (wip)")
        --toogle ward type
    end
end

return observer_ward