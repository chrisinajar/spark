loadcanvas =
{
   Properties =
   {
	  canvas = ""
   },
}

function loadcanvas:OnTick()
	if GameManagerRequestBus.Broadcast.GetGamePhase() == GAME_PHASE_PRE_GAME then
		self:Load()
		if self.tickBusHandler then
			self.tickBusHandler:Disconnect()
		end
	end
end

function loadcanvas:OnActivate()
	if UiCanvasLuaProxy then
		self.uiCanvasLuaProxy = UiCanvasLuaProxy()
		self.tickBusHandler = TickBus.Connect(self);
	end
end

function loadcanvas:Load()
	local canvas=self.uiCanvasLuaProxy:LoadCanvas(self.Properties.canvas);--"UI/Canvases/InGameHUD.uicanvas")
end

function loadcanvas:OnDeactivate()
	if self.tickBusHandler then
		self.tickBusHandler:Disconnect()
	end
end

return loadcanvas;
