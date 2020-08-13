
local UserInput = {
    centering = false,
    Properties =
	{
	},
}

local CenterCameraSlot = Slot(8,0);

function UserInput:OnActivate()
    self.SlotNotificationBusHandler = SlotNotificationBus.Connect(self)

    InputMapperRequestBus.Broadcast.BindHotKeyToSlot(Hotkey("space"),CenterCameraSlot);
    InputMapperRequestBus.Broadcast.BindHotKeyToSlot(Hotkey("F4"),Slot(Slot.Shop,0));

    Debug.Log("UserInput:OnActivate() done");
end

function UserInput:OnDeactivate()
    if self.SlotNotificationBusHandler~=nil then 
        self.SlotNotificationBusHandler:Disconnect()
    end
end

function UserInput:OnSlotPressed(slot)
    if slot==CenterCameraSlot then
        --center camera to the main selected unit
        Debug.Log("centering camera")
        self.centering = true
        if self.tickBusHandler==nil then 
            self.tickBusHandler = TickBus.Connect(self)	
        end    
    end
end

function UserInput:OnSlotReleased(slot)
    if slot == CenterCameraSlot then
        self.centering = false
        if self.tickBusHandler~=nil then 
            self.tickBusHandler:Disconnect()
            self.tickBusHandler = nil
        end    
    end
end


function UserInput:OnTick(deltaTime, timePoint)

    if self.centering then
        local unitId = SelectionRequestBus.Broadcast.GetMainSelectedUnit()

        if unitId and unitId:IsValid() then
            local pos = TransformBus.Event.GetWorldTranslation(unitId)

            SparkCameraRequestBus.Broadcast.LookAtWorldPosition(pos)
        end
    end
end

return UserInput;