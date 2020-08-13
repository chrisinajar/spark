local json= require "scripts.json"

function GetInfo(id)
    if not id then return nil end
    local json_str=StaticDataRequestBus.Event.GetJsonString( id )
    return json_str and json.parse(json_str)
end

function GetItemInfo(id)
    if type(id) == 'string' then -- is a ItemTypeId
        local json_str=InfoRequestBus.Broadcast.GetItemJsonString( id )
        return json_str and json.parse(json_str)
    else --is a ItemId
        return GetInfo(id);
    end
end

function GetAbilityInfo(id)
    if type(id) == 'string' then -- is a AbilityTypeId
        local json_str=InfoRequestBus.Broadcast.GetAbilityJsonString( id )
        return json_str and json.parse(json_str)
    else --is a AbilityId
        return GetInfo(id);
    end
end

local itemInfo = {}

function GetUnitInfo(id)
    if type(id) == 'string' then -- is a UnitTypeId
        if itemInfo[id]==nil then
            local json_str=InfoRequestBus.Broadcast.GetUnitJsonString( id )
            itemInfo[id] = json_str and json.parse(json_str)
        end
        return itemInfo[id]
    else --is a UnitId
        return GetInfo(id);
    end
end

function GetGameModeInfo()
    local json_str=InfoRequestBus.Broadcast.GetGameModeJsonString()
    return json_str and json.parse(json_str)
end

function GetJsonValueByPath(path,root)--path is an array of strings
    --Debug.Log("GetJsonValueByPath with path:"..dump(path).."  root:"..dump(root))
    if root == nil then
        return nil
    elseif path==nil or #path==0 then 
        return root; 
    end
    local memberId = table.remove(path,1)--get first element of the array and remove it
    local member = root[memberId]
    if member~=nil then
        return GetJsonValueByPath(path,member)
    end
end


function GetKvValue(str, level)
    level = level or 1;
    level = math.max(math.floor(level),1);
    local result = "";
    for i in string.gmatch(str, "%S+") do
        result = i
        level = level - 1;
        if level<= 0 then return result; end
    end
    return result;
end

function HasMultipleValues(str)
    if type(str) == 'number' then return false end
    if type(str) == 'string' then 
        return (GetNumberKvValues(str) or 0)>1;
    end
    return nil; 
end

function GetNumberKvValues(str)
    if type(str) == 'number' then return 1 end
    if type(str) == 'string' then 
        _,n = str:gsub("%S+","")
        return n
    end
end

function run_test()
    Debug.Log("running info.lua test");

    Debug.Error(HasMultipleValues(123)==false);
    Debug.Error(HasMultipleValues("123")==false);
    Debug.Error(HasMultipleValues("0 1 2")==true);
    Debug.Error(HasMultipleValues({})==nil);

    Debug.Error(GetNumberKvValues(123)==1);
    Debug.Error(GetNumberKvValues("123")==1);
    Debug.Error(GetNumberKvValues("0 1 2")==3);  
    Debug.Error(GetNumberKvValues({})==nil);
end

--run_test();