require "scripts.library.client_events"

Camera = CreateGlobalVariable("Camera")

function Camera:OnActivate()
	self.unlistenAll = Event()
	self.unlistenAll.listen(ListenToClientEvent("LookAtEntity", partial(self.LookAtEntity, self)))
	self.unlistenAll.listen(ListenToClientEvent("SelectUnit", partial(self.SelectUnit, self)))
end

function Camera:OnDeactivate()
	self.unlistenAll.broadcast()
	self.unlistenAll.unlistenAll()
	self.unlistenAll = nil
end

function Camera:LookAtWorldPosition (position)
	SparkCameraRequestBus.Broadcast.LookAtWorldPosition(position)
end

function Camera:LookAtEntity (data, entity)
	local position = TransformBus.Event.GetWorldTranslation(entity)
	SparkCameraRequestBus.Broadcast.LookAtWorldPosition(position)
end

function Camera:SelectUnit (data, entity)
	Debug.Log("Hey we're trying to select a unit! " .. tostring(entity))
	SelectionRequestBus.Broadcast.SelectUnit(entity);
end
