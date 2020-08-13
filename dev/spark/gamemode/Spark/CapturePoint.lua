require "scripts.GameUtils"
local DebugRenderer = require "scripts.DebugRenderer";
require "gamemode.Spark.modules.capture_point";

local CapturePoint={
	percentage = 0;
	currentTeamId = nil;
	PrizeTable = {"core_1", "core_2", "core_3", "core_4", "core_5"};
	Properties =
	{
		completion_rate_for_unit = {default = 0.1 , description = "0.1=10% per second per unit	-> one unit take 10 seconds"}, 
		item_prize = {default = "blink", description = ""},
	},
}

function CapturePoint:OnActivate()
	Debug.Log("CapturePoint:OnActivate")

	self.tickBusHandler = TickBus.Connect(self)	
end

function CapturePoint:OnTick(deltaTime, timePoint)

	self:Draw()
	
	local units = TriggerAreaRequestsBus.Event.GetEntitiesInside(self.entityId)
	
	if self.currentTeamId==nil then --the capture point is neutral, find which team has more units inside
		if #units==0 then
			return;
		end
		
		--count the units for each team
		local numUnits = {} -- map of counters (number of units for each team)
		for i = 1, #units do
			local teamId = UnitRequestBus.Event.GetTeamId(units[i])
			numUnits[teamId] = (numUnits[teamId] or 0) + 1;
		end

		--find team with the max number of units
		for k,v in pairs(numUnits) do
			if self.currentTeamId==nil or v>numUnits[self.currentTeamId] then
				self.currentTeamId=k
			end
		end
	end

	local multiplier = 0
	local malus = -1
	local contested = false

	for i = 1, #units do
		local teamId = UnitRequestBus.Event.GetTeamId(units[i])

		if self.currentTeamId==teamId then
			malus = 0
			multiplier = multiplier + 1
		else
			multiplier = multiplier - 1
			contested = true
		end
	end

	multiplier = multiplier + malus

	if contested then
		multiplier = math.min(0,multiplier)
	end

	self.percentage = self.percentage + multiplier*self.Properties.completion_rate_for_unit*deltaTime

	if self.percentage > 1 then
		self:OnCaptured(self.currentTeamId)
	else 
		if self.percentage<=0 then
		self.percentage = 0
		self.currentTeamId = nil
		end
	end
end

function CapturePoint:Draw(name)
	local pos=TransformBus.Event.GetWorldTranslation(self.entityId)
	pos.z = MapRequestBus.Broadcast.GetTerrainHeightAt(pos)+0.1
	local radius = 5
	local color = Color(1,1,1);
	local angle = 1

	if self.currentTeamId then
		angle = self.percentage
		color = GetTeamColor(self.currentTeamId)
	end

	DebugRenderer.Begin("cp"..tostring(self.entityId),true);

    local p1=Vector3(pos.x,pos.y+radius,pos.z);
    p1.z = MapRequestBus.Broadcast.GetTerrainHeightAt(p1)+0.1
	
	DebugRenderer.DrawLine(pos,p1,color,1);

    local step=math.rad(10) --step every 10 degree
    for i=step,6.30 * angle,step do
        local p2 = Vector3(pos.x + math.sin(i)*radius,pos.y + math.cos(i)*radius, pos.z );
        p2.z = MapRequestBus.Broadcast.GetTerrainHeightAt(p2) + 0.1

        DebugRenderer.DrawLine(p1,p2,color,1);
        p1 = p2
	end
	
	DebugRenderer.DrawLine(pos,p1,color,1);
end

function CapturePoint:OnCaptured(teamId)
	local PrizeIndex = CapturePointModule:GetNumberOfCaptures()
	--get all the heroes of the team
	local teamHeroes = FilterArray( GetAllUnits(), function (unit)
		return unit:GetTeamId() == teamId and HasTag(unit,"hero")
	end);

	--reward them with the item
	for i=1,#teamHeroes do
		if PrizeIndex <= 4 then
			teamHeroes[i]:GiveItem( CreateItem(tostring(self.PrizeTable[PrizeIndex +1])) );
		else
			teamHeroes[i]:GiveItem( CreateItem(tostring(self.PrizeTable[5])) );
		end
	end
	CapturePointModule:OnCaptured(teamId);
	--suicide
	GameEntityContextRequestBus.Broadcast.DestroyGameEntity(self.entityId);
end

function CapturePoint:OnDeactivate()
	self.tickBusHandler:Disconnect()
end

return CapturePoint;
