
function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function isModuleAvailable(name)
  if package.loaded[name] then
    return true
  else
    for _, searcher in ipairs(package.searchers or package.loaders) do
      local loader = searcher(name)
      if type(loader) == 'function' then
        package.preload[name] = loader
        return true
      end
    end
    return false
  end
end

function dump(o,max_level)
  max_level = max_level or 4;
	
  if max_level>0 and type(o) == 'table' then
    local s = '{ '
    for k,v in pairs(o) do
      if type(k) ~= 'number' then k = '"'..k..'"' end
      s = s .. '['..k..'] = ' .. dump(v,max_level-1) .. ','
    end
    return s .. '} '
  else
    return tostring(o).." "
  end
end

-- look up for `k' in list of tables `plist'
local function search (k, plist)
  for i=1, table.getn(plist) do
	local v = plist[i][k]     -- try `i'-th superclass
	if v then return v end
  end
end

function GetId(o)
	if (type(o) == 'table') then 
		return o.entityId
	elseif (type(o) == 'userdata') then
		return o
	end
	return EntityId()
end

function splitstring(inputstr, sep)
  if sep == nil then
          sep = "%s"
  end
  local t={}
  for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
          table.insert(t, str)
  end
  return t
end

function trim(s)
  return string.match(s,'^()%s*$') and '' or string.match(s,'^%s*(.*%S)')
end

function Require(hint)

  --check inside the gamemodes folder used
  local path
  path = string.gsub(hint,"(.lua)","")
  path = string.gsub(path,"[.]","/")

  path = GameManagerRequestBus.Broadcast.GetActualFilename(path..".lua");
  Debug.Log(path)

  if(path and path~="") then
    --if file exist, convert back to lua syntax
    path = string.gsub(path, ".*(spark/)(gamemode)","%2")
  else

    path = hint

    --hopefully is a global script, not inside any gamemode
    if not (path:find("^scripts.") ~= nil) then --if don't start with "script.
      path = "scripts."..path --add it
    end  
  end

  path = string.gsub(path,"(.lua)","")
  path = string.gsub(path,"/",".")

  --Debug.Log("trying to use require script with : "..path);
  if isModuleAvailable(path) then
    return require(path)
  else
    Debug.Log("Warning : \'require\' used with unavailable file : "..path)
  end
end

function HasTag(entity,tag)
  return TagComponentRequestBus.Event.HasTag(GetId(entity),Crc32(tag))
end