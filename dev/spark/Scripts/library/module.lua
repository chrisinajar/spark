require "scripts.library.ebus"
require "scripts.library.globals"
require "scripts.library.functional"
require "scripts.core.module"

CreateGlobalVariable("Modules")

package.path = ";./?.lua" .. package.path

local function GetCallingGameMode (offset)
  offset = offset or 4

  local functionInfo = debug.getinfo(offset - 1, "Sl")
  local gameMode = string.match(functionInfo.source, "[/\\]([^/\\]+)[/\\].*.lua")

  return gameMode
end

local function safeRequire (name)
  return pcall(function ()
    return require(name)
  end)
end

function LoadModule (gameMode, name)
  if name == nil then
    name = gameMode
    gameMode = GetCallingGameMode()
  end
  Debug.Log('Loading module: ' .. gameMode .. '/' .. name)
  if not safeRequire("gamemode." .. gameMode .. ".modules." .. name .. ".index") and not safeRequire("gamemode." .. gameMode .. ".modules." .. name) then
    Debug.Error('Module "' .. tostring(name) .. '" does not exist in gamemode "' .. tostring(gameMode) .. '"')
  end
end

-- called from the module implementations to get their global constant
function CreateModule (name, initPhase)
  if Modules.modules[name] then
    return Modules.modules[name]
  end
  local myModule = Module()
  myModule.name = name
  myModule.initPhase = initPhase or GAME_PHASE_CONFIGURE_GAME
  Modules.modules[name] = myModule

  myModule.unlistenAll = Modules.unlistenAll.listen

  if Modules.gamePhases[myModule.initPhase].hasLoaded then
    InitModule(myModule)
  else
    table.insert(Modules.gamePhases[myModule.initPhase].moduleQueue, myModule)
  end

  return myModule
end

function DestroyModules()
  if Modules.handler then
    Modules.handler()
    Modules.handler = nil
  end
  Modules.unlistenAll.broadcast({})
  Modules.unlistenAll.unlistenAll()
end

local function InitModule (myModule)
    if not IsServer() then
      return {}
    end
    Debug.Log('Initializing module: ' .. myModule.name)
    myModule:Init()
end

local function LoadModuleQueue (newPhase)
  for _,myModule in ipairs(Modules.gamePhases[newPhase].moduleQueue) do
    InitModule(myModule)
  end
  Modules.gamePhases[newPhase].hasLoaded = true
  Modules.gamePhases[newPhase] = {}
end

function InitModules()
  Modules.unlistenAll = Event()
  Modules.modules = {}
  Modules.loadedModules = {}
  Modules.gamePhases = {
    [GAME_PHASE_WAITING_FOR_PLAYERS] = {
      hasLoaded = false,
      moduleQueue = {}
    },
    [GAME_PHASE_CONFIGURE_GAME] = {
      hasLoaded = false,
      moduleQueue = {}
    },
    [GAME_PHASE_HERO_SELECT] = {
      hasLoaded = false,
      moduleQueue = {}
    },
    [GAME_PHASE_PRE_GAME] = {
      hasLoaded = false,
      moduleQueue = {}
    },
    [GAME_PHASE_GAME] = {
      hasLoaded = false,
      moduleQueue = {}
    },
    [GAME_PHASE_POST_GAME] = {
      hasLoaded = false,
      moduleQueue = {}
    },
  }

  Modules.handler = ConnectToEvents(GameManagerNotificationBus, nil, { "OnGameManagerReady", "OnGamePhaseChange" })
  Modules.handler.OnGamePhaseChange(LoadModuleQueue)
end