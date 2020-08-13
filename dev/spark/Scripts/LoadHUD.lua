loadcanvas = {}

function loadcanvas:OnActivate()
    local canvas=UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/InGameHUD.uicanvas")

end

return loadcanvas;