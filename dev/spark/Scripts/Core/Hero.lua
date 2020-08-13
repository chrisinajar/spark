require("scripts.core.unit")
require("scripts.core.gamemanager")
require("scripts.GameUtils")

Hero = class(Unit);

function Hero:OnActivate ()
	Unit.OnActivate(self);

	if not IsServer() then
		return
	end

	--self:SetValue("movement_speed", 100);
	--AnimGraphComponentRequestBus.Event.SetNamedParameterFloat(self.entityId,"SpeedMultiplier", 0.13)--12);

	--self:SetValue("projectile_speed", 30);
	--self:SetValue("base_damage", 50);
	
	self:RegisterVariable("cooldown_reduction", 0);

	self:RegisterVariable("attribute_primary", 0);

    self:RegisterDependentVariable("strength");
    self:RegisterVariable("base_strength", 10);
    self:RegisterVariable("gain_strength", 1.5);
	self:RegisterDependentVariable("agility");
	self:RegisterVariable("base_agility", 10);
	self:RegisterVariable("gain_agility", 1.5);
	self:RegisterDependentVariable("intelligence");
	self:RegisterVariable("base_intelligence", 10);
	self:RegisterVariable("gain_intelligence", 1.5);
	self:RegisterDependentVariable("will");
	self:RegisterVariable("base_will", 10);
	self:RegisterVariable("gain_will", 1.5);

	self:RegisterVariable("gold", 1000);
	self:RegisterVariable("gpm", 60);

	self:RegisterVariable("experience", 0);
	self:RegisterVariable("ability_points", 0);
	self:RegisterDependentVariable("level");
	self:RegisterDependentVariable("experience_max");
	self:RegisterDependentVariable("experience_progress");

	self:RegisterVariable("vision_range", 30);
	self:RegisterVariable("vision_type", VISIBILITY_VISIBLE);

	self.OnNewOrder = self.OnNewOrder

	local info = self:GetJson()
	Debug.Log("Got some info " .. tostring(info.attribute_primary))

	-- set up stats and functionality from static data in info
	if info and IsServer() then
		local attribute = string.lower(tostring(info.attribute_primary))
		if attribute == "strength" then
			self:SetValue("attribute_primary", 1);
		elseif attribute == "agility" then
			self:SetValue("attribute_primary", 2);
		elseif attribute == "intelligence" then
			self:SetValue("attribute_primary", 3);
		elseif attribute == "will" then
			self:SetValue("attribute_primary", 4);
		end

		if info.attribute_starting_strength then
			self:SetValue("base_strength", info.attribute_starting_strength)
		end
		if info.attribute_starting_agility then
			self:SetValue("base_agility", info.attribute_starting_agility)
		end
		if info.attribute_starting_intelligence then
			self:SetValue("base_intelligence", info.attribute_starting_intelligence)
		end
		if info.attribute_starting_will then
			self:SetValue("base_will", info.attribute_starting_will)
		end

		if info.attribute_strength_base then
			self:SetValue("gain_strength", info.attribute_strength_base)
		end
		if info.attribute_agility_base then
			self:SetValue("gain_agility", info.attribute_agility_base)
		end
		if info.attribute_intelligence_base then
			self:SetValue("gain_intelligence", info.attribute_intelligence_base)
		end
		if info.attribute_will_base then
			self:SetValue("gain_will", info.attribute_starting_will)
		end

		if info.base_health then
			self:RegisterVariable("base_hp", info.base_health);
		end
		if info.base_health_regen then
			self:RegisterVariable("base_hp_regen", info.base_health_regen);
		end
		-- if info.base_mana then
		-- 	self:RegisterVariable("base_mana", info.base_mana);
		-- end
		-- if info.base_mana_regen then
		-- 	self:RegisterVariable("base_mana_regen", info.base_mana_regen);
		-- end

		-- if info.base_damage then
		-- 	self:SetValue("base_damage", info.base_damage)
		-- end

		if info.base_physical_armor then
			self:SetValue("base_armor", info.base_physical_armor)
		end
		-- base_magic_armor
		-- attack_type is handled in C++
		-- attack_damage_type
		-- if info.attack_animation_point then
		-- 	self:SetValue("attack_point", info.attack_animation_point)
		-- end
		-- if info.attack_backswing then
		-- 	self:SetValue("backswing_point", info.attack_backswing)
		-- end
		-- if info.attack_range then
		-- 	self:SetValue("attack_range", info.attack_range / 50)
		-- end
		-- if info.projectile_speed then
		-- 	self:SetValue("projectile_speed", info.projectile_speed)
		-- end
		-- if info.vision_daytime_radius then
		-- 	self:SetValue("vision_range", info.vision_daytime_radius / 50)
		-- end
	end

	self:RegisterVariable("mana_percentage", 1);--mana percentage: 1 is 100%
	self:RegisterDependentVariable("mana_regen");
	self:RegisterDependentVariable("mana_max");
	self:RegisterDependentVariable("mana");
end

function Hero:OnCreated ()
	--Unit:OnActivate(self);
	Debug.Log(tostring(self));
	Debug.Log("Hero:OnCreated    unit name:" .. self:GetName());

	--QuickBuyItemList test
	local itemList=vector_basic_string_char_char_traits_char();
	itemList:push_back("test_item");
	UnitRequestBus.Event.SetQuickBuyItemList(self.entityId,itemList);

	local itemList=UnitRequestBus.Event.GetQuickBuyItemList(self.entityId);
	Debug.Log(dump(itemList));
	if itemList then
		for i=1, #itemList do		
			Debug.Log("item "..i..":"..itemList[i]);
		end
	end
	--clear it after the test 
	UnitRequestBus.Event.SetQuickBuyItemList(self.entityId,vector_basic_string_char_char_traits_char());
	Debug.Log("end of Hero:OnActivate    unit name:"..self:GetName());

	self:SetValue("mana", self:GetValue("mana_max"));
end

function Hero:Respawn ()
	self:SetAlive(true)
	local player = self:GetPlayerOwner()
	if player and player:GetHero() == self then
		player:LookAtHero()
	end
end

function Hero:OnNewOrder(order, queue)
	if not Game:IsAuthoritative() then
		return
	end
	-- lua implemented unit orders!
	if order.type == UNIT_ORDER_UPGRADE and order.target:IsValid() then
		-- upgrade abilities and items
		-- bounds checking and stuff is run in the updrade method
		local ability = Ability({ entityId = order.target })
		ability:Upgrade()
		return
	end
end

function Hero:OnDamageTaken(damage)
	Debug.Log("Hero:OnDamageTaken    "..damage:ToString());
	Debug.Log(tostring(UnitAbilityRequestBus.Event.GetAbilityInSlot(self.entityId,Slot(Slot.Ability,3))));
end


function Hero:OnDestroyed ()
	--Unit:OnDeactivate(self);
	Debug.Log("Hero:OnDestroyed    unit name:"..self:GetName());
	if(self.tickBusHandler ~= nil )then
		self.tickBusHandler:Disconnect();
	end
end

function Hero:OnKilled(damage)
	--self:Destroy()
	self:SetValue("deaths", self:GetValue("deaths") + 1)
	local killer = Unit({ entityId = damage.source})
	if damage.target == self:GetId() and killer:GetTeamId() ~= self:GetTeamId() then
		killer:Give("kills", self:GetValue("kills") + 1)
	end
end

function Hero:IsHero ()
	return true
end

return require("scripts.components.hero");