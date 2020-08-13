
local DebugRenderer = {}

function DebugRenderer.Begin(name,clear)
    DebugRenderRequestBus.Broadcast.Begin(name,true);
end 
function DebugRenderer.DrawSphere(pos,radius,color,timer)
    color = color or Color(1,0,0,0.7)
    timer = timer or 0.1
    DebugRenderRequestBus.Broadcast.DrawSphere(pos,radius,color,timer)
end

function DebugRenderer.DrawLine(p1,p2,color,timer)
    color = color or Color(1,0,0,1)
    timer = timer or 5
    DebugRenderRequestBus.Broadcast.DrawLine(p1,p2,color,timer)
end

-- x,y,fontSize are in pixelSpace eg. [0,0] -> [800,600]
function DebugRenderer.Draw2DText(text,x,y,fontSize,color,timer)
    color = color or Color(1,0,0,1)
    timer = timer or 0.1
    fontSize = fontSize or 20
    x = x or 0
    y = y or 20
    DebugRenderRequestBus.Broadcast.Draw2DText(text,x,y,fontSize,color,0.1);
end

function DebugRenderer.DrawAura(modifier,color,timer)

    local modifierId = GetId(modifier)
    local pos= TransformBus.Event.GetLocalTranslation(modifierId)
	local radius = ModifierRequestBus.Event.GetAuraRadius(modifierId);
	color = color or Color(1,1,0,0.2);
    timer = timer or 0.1
    
	DebugRenderer.Begin("aura"..tostring(modifierId),true);
	DebugRenderer.DrawSphere(pos,radius,color,timer);
    --DebugRenderer.Draw2DText(name,0,20,20,Color(1,0,0),0.1);--todo need a way to convert from world to screen coordinate

    --draw a circle on the ground
    color.a = 1.0
    local p1=Vector3(pos.x+radius,pos.y,pos.z);
    p1.z = MapRequestBus.Broadcast.GetTerrainHeightAt(p1)+0.1

    local step=math.rad(10) --step each 10 degree
    for i=step,6.28,step do
        local p2 = Vector3(pos.x + math.cos(i)*radius,pos.y + math.sin(i)*radius, pos.z );
        p2.z = MapRequestBus.Broadcast.GetTerrainHeightAt(p2) + 0.1

        DebugRenderRequestBus.Broadcast.DrawLine(p1,p2,color,timer);
        p1 = p2
    end

end


--todo other functions

return DebugRenderer
