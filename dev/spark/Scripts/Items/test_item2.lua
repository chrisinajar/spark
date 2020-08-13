require "scripts.items.item";
require "scripts.modifier";
local Test = require "scripts.ui.maingame.itemscript";

local modifier


local testItem2 = Item();

function testItem2:OnCreated()
	--Ability:OnActivate(self);
	Debug.Log( self:GetName().." testItem2:OnCreated!");
	ParticleComponentRequestBus.Event.Enable(self.entityId,false)
	local c=CastingBehavior(CastingBehavior.IMMEDIATE);
	self:SetCastingBehavior(c);
	
	self:RegisterVariable("max_range",10)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
	
	self:SetCooldown(10);
	self:SetCosts({mana=25});
	
end

function testItem2:OnItemPhaseStart()
	Debug.Log(self:GetName().." OnItemPhaseStart!");
	return true;
end


function testItem2:OnSpellStart()
	Debug.Log(self:GetName().." onSpellStart!");

	local pos=self:GetCursorPosition();
	Debug.Log(tostring(pos));
	Debug.Log(dump(pos));
	
	
	caster = self:GetCaster();
	if(caster) then
		local id=caster:GetId();
		Debug.Log("caster is :"..tostring(id));	
		--Test:StartCooldown();
		AddNewModifier(caster,self,"test_item2_modifier",{duration=4});		
	end
	
	Debug.Log("end of OnSpellStart");
	
end


--MODIFIER

local test_item2_modifier = class(Modifier);

function test_item2_modifier:OnCreated()
	Debug.Log("test_item2_modifier:OnCreated()");
	self:AttachVariableModifier("intelligence");
end
function test_item2_modifier:OnDestroy()
	Debug.Log("test_item2_modifier:OnDestroy()");
end


function test_item2_modifier:GetModifierBonus_intelligence()
	--Add strength bonus
	--Debug.Log("ADDING FORCE")
	return 25;
end

LinkLuaModifier("test_item2_modifier",test_item2_modifier);



return testItem2;


