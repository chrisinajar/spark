require "scripts.items.item";
require "scripts.modifier";

local modifier
local testItem = Item();

function testItem:OnCreated()
	--Ability:OnActivate(self);
	Debug.Log( self:GetName().." testItem:OnCreated!");
	ParticleComponentRequestBus.Event.Enable(self.entityId,false)
	local c=CastingBehavior(CastingBehavior.IMMEDIATE);
	self:SetCastingBehavior(c);
	
	self:RegisterVariable("max_range",10)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
	
	self:SetCooldown(10);
	self:SetCosts({mana=25});
	
end

function testItem:OnItemPhaseStart()
	Debug.Log(self:GetName().." OnItemPhaseStart!");
	return true;
end


function testItem:OnSpellStart()
	Debug.Log(self:GetName().." onSpellStart!");

	local pos=self:GetCursorPosition();
	Debug.Log(tostring(pos));
	Debug.Log(dump(pos));
	
	
	caster = self:GetCaster();
	if(caster) then
		local id=caster:GetId();
		Debug.Log("caster is :"..tostring(id));	
		AddNewModifier(caster,self,"test_item_modifier",{duration=4});		
	end
	
	Debug.Log("end of OnSpellStart");
	
end


--MODIFIER

local test_item_modifier = class(Modifier);

function test_item_modifier:OnCreated()
	Debug.Log("test_item_modifier:OnCreated()");
	self:AttachVariableModifier("strength");
end
function test_item_modifier:OnDestroy()
	Debug.Log("test_item_modifier:OnDestroy()");
end


function test_item_modifier:GetModifierBonus_strength()
	--Add strength bonus
	--Debug.Log("ADDING FORCE")
	return 25;
end

LinkLuaModifier("test_item_modifier",test_item_modifier);



return testItem;


