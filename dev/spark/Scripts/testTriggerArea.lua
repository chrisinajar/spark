

local o={}

function o:OnActivate()
	self.time=0;
	self.origin = TransformBus.Event.GetWorldTranslation(self.entityId);
	
	self.origin.x=502
	self.origin.y=553
	self.origin.z=41
	
	self.tickBusHandler = TickBus.Connect(self);

	self.tah= TriggerAreaNotificationBus.Connect(self,self.entityId);
	
	self.radius = SphereShapeComponentRequestsBus.Event.GetRadius(self.entityId);
	
	
	
	
end

function o:OnTriggerAreaEntered(entityId)
	Debug.Log("OnTriggerAreaEntered");
end
function o:OnTriggerAreaExited(entityId)
	Debug.Log("OnTriggerAreaExited");
end

function o:OnTick(deltaTime, timePoint)
	
	if not self.initDone then
		Debug.Log("Units present in the game are:")
		local tags=vector_Crc32();
		tags:push_back(Crc32("unit"));
		local units=GameManagerRequestBus.Broadcast.GetEntitiesHavingTags(tags);
		--units=GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("unit"))  in this case would do the same
		if(#units>0) then	
			for i=1, #units do
				Debug.Log("unit "..i..":"..tostring(units[i]))
			end
			
			self.initDone=true
		end
		
	end
	
	
	local pos= TransformBus.Event.GetWorldTranslation(self.entityId);
	
	local radius=self.radius*(1+math.cos(self.time * 0.3));
	SphereShapeComponentRequestsBus.Event.SetRadius(self.entityId,radius);
	
	
	pos.y = self.origin.y + math.cos( self.time )*15;
	pos.x = self.origin.x + math.sin( self.time )*15;
	
	TransformBus.Event.SetWorldTranslation(self.entityId,pos);
	
	self.time = self.time+deltaTime
	
	
	local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(pos , radius);
	if(#units>0) then
		Debug.Log("GetUnitsInsideSphere returned:")
		for i=1, #units do
			Debug.Log("unit "..i..":"..tostring(units[i]))
		end
	end
	
	units=TriggerAreaRequestsBus.Event.GetEntitiesInside(self.entityId);
	if(#units>0) then
		Debug.Log("GetEntitiesInside returned:")
		for i=1, #units do
			Debug.Log("unit "..i..":"..tostring(units[i]))
		end		
	end
	
	
	local on=0;
	if(#units>0) then on=1 end
	
	LightComponentRequestBus.Event.SetState(self.entityId,on);
			
end

return o;