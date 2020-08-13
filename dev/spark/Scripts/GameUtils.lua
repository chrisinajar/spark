require "scripts.common"


function CreateAbility(id)
	return Ability{entityId=GameManagerRequestBus.Broadcast.CreateAbility(id)}
end

function CreateItem(ItemTypeId)
	return Item{entityId=GameManagerRequestBus.Broadcast.CreateItem(ItemTypeId)}
end

function CreateUnit(UnitTypeId, json)
	local id
	if json then
		id = GameManagerRequestBus.Broadcast.CreateUnitWithJson(UnitTypeId,json)
	else
		id = GameManagerRequestBus.Broadcast.CreateUnit(UnitTypeId)
	end
	if id and id:IsValid() then
		return Unit{entityId=id}
	end
end

function GetAllUnits()
	local array=GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("unit"));
	local new_array = {}
	for i=1,#array do
		table.insert(new_array,Unit{entityId=array[i]})
	end
	return new_array
end

function GetNearestUnitInArray(target,units)
	
	if type(target)=='table' then
		if target.GetPosition then
			target = target:GetPosition()
		end
	end

	--target at this point should be a Vector3, todo: check it (  type() doesn't work   )

	if units and #units>0 then
		local unit = units[1]
		local distance = Distance2D(target,unit:GetPosition())

		for i=2,#units do
			local it_distance = Distance2D(target,units[i]:GetPosition())
			if it_distance < distance then
				unit = units[i]
				distance = it_distance
			end
		end
		
		return unit
	end
end

function GetNearestFriendlyUnit(unit)
	if unit==nil then return end

	local teamId = unit:GetTeamId()
	local friendly_units_not_me = FilterArray(GetAllUnits(),function (elem)
		return elem:GetTeamId()==teamId and GetId(elem)~=GetId(unit)
	end);
	return GetNearestUnitInArray(unit,friendly_units_not_me)
end

function FilterArray(array,filter)
	local temp = {}
	for k,v in pairs(array) do
		if filter(v)==true then
			table.insert(temp,v)
		end
	end
	array = temp
	return array
end


function FilterByTeam(units,teamId)
	return FilterArray(units,function (elem)
		return UnitRequestBus.Event.GetTeamId(GetId(elem))==teamId;
	end);
end

function GetTeamIdByName(teamName)
	return GameManagerRequestBus.Broadcast.GetTeamIdByName(teamName)
end

function GetTeamName(teamId)
	return GameManagerRequestBus.Broadcast.GetTeamName(teamId)
end

function SetTeamName(teamId,name)
	return GameManagerRequestBus.Broadcast.SetTeamName(teamId,name)
end

function GetTeamColor(teamId)
	return GameManagerRequestBus.Broadcast.GetTeamColor(teamId)
end

function SetTeamColor(teamId,color)
	return GameManagerRequestBus.Broadcast.SetTeamColor(teamId,color)
end

function AttachParticleSystem(entity, particleSystemTypeId)
	local ps = GameManagerRequestBus.Broadcast.CreateParticleSystem(particleSystemTypeId)
	TransformBus.Event.SetParent(ps,GetId(entity))
	TransformBus.Event.SetLocalTranslation(ps,Vector3(0,0,0))
	return ps
end
