
//MVM Upgrade stuff
static int MAX_INT = 99999999;
static int MIN_INT = -99999999;

JSONArray CTFPlayerUpgrades[MAXPLAYERS + 1];
float g_flNextUpgrade[MAXPLAYERS + 1];
int m_nPurchasedUpgrades[MAXPLAYERS + 1];

int GetUpgradePriority(JSONObject info)
{
	CMannVsMachineUpgrades upgrade = CMannVsMachineUpgradeManager().GetUpgradeByIndex(info.GetInt("index"));
	
/*	if (info.GetInt("pclass") == view_as<int>(TFClass_Sniper)) {
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_PRIMARY && StrEqual(upgrade.m_szAttribute(), "explosive sniper shot")) {
			return 100;
		}
	}
	
	if (info.GetInt("pclass") == view_as<int>(TFClass_Medic)) {
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_SECONDARY && StrEqual(upgrade.m_szAttribute(), "generate rage on heal")) {
			return 100;
		}
	}
	
	if (info.GetInt("pclass") == view_as<int>(TFClass_Soldier)) {
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_PRIMARY)
		{
			if(StrEqual(upgrade.m_szAttribute(), "heal on kill")) 
				return 90;
			
			if(StrEqual(upgrade.m_szAttribute(), "rocket specialist")) 
				return 80;
		}
	}
	*/
	if (info.GetInt("pclass") == view_as<int>(TFClass_Spy)) 
	{
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_MELEE) 
		{
			if(StrEqual(upgrade.m_szAttribute(), "armor piercing"))
				return 100;
				
			if(StrEqual(upgrade.m_szAttribute(), "melee attack rate bonus"))
				return 90;
				
			if(StrEqual(upgrade.m_szAttribute(), "robo sapper"))
				return 80;
		}
	}
	/*
	if (info.GetInt("pclass") == view_as<int>(TFClass_Heavy)) {
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_PRIMARY && StrEqual(upgrade.m_szAttribute(), "attack projectiles")) {
			return 100;
		}
	}
	
	if (info.GetInt("pclass") == view_as<int>(TFClass_Scout)) {
		if (info.GetInt("slot") == TF_LOADOUT_SLOT_SECONDARY && StrEqual(upgrade.m_szAttribute(), "applies snare effect")) {
			return 100;
		}
	}*/
	
	// low priority for canteen upgrades
	if (info.GetInt("slot") == TF_LOADOUT_SLOT_ACTION) 
	{
		return -10;
	}
	
	// default priority
	return GetRandomInt(50, 100);
}

bool CTFBotPurchaseUpgrades_OnStart(int actor)
{
	if (!IsStandingAtUpgradeStation(actor)) 
	{
	//	PrintToServer("Not standing at an upgrade station!");
		ChangeAction(actor, ACTION_GOTO_UPGRADE, "CTFBotPurchaseUpgrades_OnStart: Not standing at an upgrade station!");
		return false;
	}
	
	CollectUpgrades(actor);
	
	KV_MvM_UpgradesBegin(actor);
	
	g_flNextUpgrade[actor] = GetGameTime() + GetUpgradeInterval();
	
	g_bUpdateLookingAroundForEnemies[actor] = false;
	
	return true;
}

bool CTFBotPurchaseUpgrades_Update(int actor)
{
	if (!IsStandingAtUpgradeStation(actor)) 
	{
		ChangeAction(actor, ACTION_GOTO_UPGRADE, "CTFBotPurchaseUpgrades_Update: Not standing at an upgrade station!");
		return false;
	}

	float flNextTime = g_flNextUpgrade[actor] - GetGameTime();
	if (flNextTime <= 0.0)
	{
		g_flNextUpgrade[actor] = GetGameTime() + GetUpgradeInterval();
		
		JSONObject info = CTFBotPurchaseUpgrades_ChooseUpgrade(actor);
		if (info != null) 
		{
			CTFBotPurchaseUpgrades_PurchaseUpgrade(actor, info);
		}
		else 
		{
			g_flNextUpdate[actor] = 0.0;
			
			if(TF2_GetPlayerClass(actor) == TFClass_Engineer)
			{
				ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Engineer: Start building.");
			}
			else if(TF2_GetPlayerClass(actor) == TFClass_Medic)
				ChangeAction(actor, ACTION_MEDIC_HEAL, "Medic: Start heal mission.");
			else if(IsSniperRifle(actor))
				ChangeAction(actor, ACTION_SNIPER_LURK, "Sniper: Start lurking."); 
			else
				ChangeAction(actor, ACTION_MOVE_TO_FRONT, "Finished upgrading; Move to front and press F4"); 
			
			//Engineer presses F4 once done setting up buildings.
			if(TF2_GetPlayerClass(actor) != TFClass_Engineer)
				FakeClientCommand(actor, "tournament_player_readystate 1");
			
			delete info;
			return false;
		}
		
		delete info;
	}
	
	return true;
}

void CTFBotPurchaseUpgrades_OnEnd(int actor)
{
	KV_MvM_UpgradesDone(actor);
	
	g_bUpdateLookingAroundForEnemies[actor] = true;
}

JSONObject CTFBotPurchaseUpgrades_ChooseUpgrade(int actor)
{
	int currency = GetEntProp(actor, Prop_Send, "m_nCurrency");
	
	CollectUpgrades(actor);
	
	for (int i = 0; i < CTFPlayerUpgrades[actor].Length; i++) 
	{
		JSONObject info = view_as<JSONObject>(CTFPlayerUpgrades[actor].Get(i));
		
		CMannVsMachineUpgrades upgrades = CMannVsMachineUpgradeManager().GetUpgradeByIndex(info.GetInt("index"));
		if(upgrades.Address == Address_Null)
		{
			PrintToServer("CMannVsMachineUpgrades is NULL");
			
			delete info;
			return null;
		}
		
		char attrib[128]; attrib = upgrades.m_szAttribute();
		CEconItemAttributeDefinition attr = GetAttributeDefinitionByName(attrib);
		if(attr.Address == Address_Null)
			continue;
		
		int iAttribIndex = attr.GetIndex(); 
		if (!SDKCall(g_hCanUpgradeWithAttrib, actor, info.GetInt("slot"), iAttribIndex, upgrades))
		{
			//PrintToServer("upgrade %d/%d: cannot be upgraded with", info.GetInt("slot"), info.GetInt("index"));
			delete info;
			continue;
		}
		
		int iCost = SDKCall(g_hGetCostForUpgrade, upgrades, info.GetInt("slot"), info.GetInt("pclass"), actor);
		if (iCost > currency)
		{
			//PrintToServer("upgrade %d/%d: cost $%d > $%d", info.GetInt("slot"), info.GetInt("index"), iCost, currency);
			
			delete info;
			continue;
		}
	
		int tier = SDKCall(g_hGetUpgradeTier, info.GetInt("index"));
		if (tier != 0) 
		{
			if (!SDKCall(g_hIsUpgradeTierEnabled, actor, info.GetInt("slot"), tier))
			{
				//PrintToServer("upgrade %d/%d: tier %d isn't enabled", info.GetInt("slot"), info.GetInt("index"), tier);
				
				delete info;
				continue;
			}
		}
		
		return info;
	}
	
	return null;
}

void CollectUpgrades(int client)
{
	if(CTFPlayerUpgrades[client] != null)
		delete CTFPlayerUpgrades[client];
		
	CTFPlayerUpgrades[client] = new JSONArray();
	
	
/*	int iArraySlots[] = 
	{
		-1,
		TF_LOADOUT_SLOT_PRIMARY,
		TF_LOADOUT_SLOT_SECONDARY,
	//	TF_LOADOUT_SLOT_MELEE,
	//	TF_LOADOUT_SLOT_BUILDING,
	//	TF_LOADOUT_SLOT_PDA
	};*/
	
	ArrayList iArraySlots = new ArrayList();
	iArraySlots.Push(-1);	//Always buy player upgrades
	
	bool bDemoKnight = (HasDemoShieldEquipped(client) && (!IsValidEntity(GetPlayerWeaponSlot(client, TFWeaponSlot_Primary)) || HasParachuteEquipped(client)));
	bool bEngineer = (TF2_GetPlayerClass(client) == TFClass_Engineer);
	
	if(bEngineer)
	{
		iArraySlots.Push(TF_LOADOUT_SLOT_MELEE);
		iArraySlots.Push(TF_LOADOUT_SLOT_BUILDING);
		iArraySlots.Push(TF_LOADOUT_SLOT_PDA);
	}
	else
	{	
		//Demoknight doesn't buy primary weapon upgrades.
		if(bDemoKnight)
			iArraySlots.Push(TF_LOADOUT_SLOT_MELEE);
		else
			iArraySlots.Push(TF_LOADOUT_SLOT_PRIMARY);
	
		if(IsValidEntity(GetPlayerWeaponSlot(client, TFWeaponSlot_Secondary)))
		{
			iArraySlots.Push(TF_LOADOUT_SLOT_SECONDARY);
		}
	}

	for (int i = 0; i < iArraySlots.Length; i++)
	{
		int slot = iArraySlots.Get(i);
	
		for (int index = 0; index < MAX_UPGRADES; index++)
		{
			CMannVsMachineUpgrades upgrades = CMannVsMachineUpgradeManager().GetUpgradeByIndex(index);
			
			if (upgrades.m_iUIGroup() == 1 && slot != -1) 
				continue;
			
			/* Blacklist */
	//		if(StrEqual(upgrades.m_szAttribute(), "increased jump height"))
	//			continue;
			
			CEconItemAttributeDefinition attr = GetAttributeDefinitionByName(upgrades.m_szAttribute());
			if(attr.Address == Address_Null)
				continue;
			
			if (!SDKCall(g_hCanUpgradeWithAttrib, client, slot, attr.GetIndex(), upgrades))
				continue;
			
			JSONObject UpgradeInfo = new JSONObject();
			UpgradeInfo.SetInt("pclass", view_as<int>(TF2_GetPlayerClass(client)));
			UpgradeInfo.SetInt("slot", slot);
			UpgradeInfo.SetInt("index", index);
			UpgradeInfo.SetInt("random", GetRandomInt(MIN_INT, MAX_INT));
			UpgradeInfo.SetInt("priority", GetUpgradePriority(UpgradeInfo));
			
			CTFPlayerUpgrades[client].Push(UpgradeInfo);
			
			delete UpgradeInfo;
		}
	}
	
	delete iArraySlots;
	
/*	PrintToServer("Unsorted upgrades for #%d \"%N\": %i total\n", client, client, CTFPlayerUpgrades[client].Length);
	PrintToServer("%3s %4s %-5s %-8s\n", "#", "SLOT", "INDEX", "PRIORITY");
	
	for (int i = 0; i < CTFPlayerUpgrades[client].Length; i++) 
	{
		JSONObject UpgradeInfo = view_as<JSONObject>(CTFPlayerUpgrades[client].Get(i));
		
		PrintToServer("%3d %4d %-5d %-8d", i, UpgradeInfo.GetInt("slot"), UpgradeInfo.GetInt("index"), UpgradeInfo.GetInt("priority"));
		
		delete UpgradeInfo;
	}
	*/
	
	//NEW!
	JSONArray new_json = new JSONArray();
	/////
	
	while(CTFPlayerUpgrades[client].Length > 0)
	{	
		JSONObject mObj = view_as<JSONObject>(CTFPlayerUpgrades[client].Get(0));
		int minimum = mObj.GetInt("priority"); // arbitrary number in list
		
		//NEW!
		JSONObject tempObj = new JSONObject();
		tempObj.SetInt("pclass",   mObj.GetInt("pclass"));
		tempObj.SetInt("slot",     mObj.GetInt("slot"));
		tempObj.SetInt("index",    mObj.GetInt("index"));
		tempObj.SetInt("random",   mObj.GetInt("random"));
		tempObj.SetInt("priority", mObj.GetInt("priority"));
		/////
		
		delete mObj;
		
		for (int x = 0; x < CTFPlayerUpgrades[client].Length; x++)
		{
			JSONObject xObj = view_as<JSONObject>(CTFPlayerUpgrades[client].Get(x)); // arbitrary number in list
			
			if (xObj.GetInt("priority") > minimum)
			{
				minimum = xObj.GetInt("priority");
				
				//NEW!
				tempObj.SetInt("pclass",   xObj.GetInt("pclass"));
				tempObj.SetInt("slot",     xObj.GetInt("slot"));
				tempObj.SetInt("index",    xObj.GetInt("index"));
				tempObj.SetInt("random",   xObj.GetInt("random"));
				tempObj.SetInt("priority", xObj.GetInt("priority"));
				/////
			}

			delete xObj;
		}
		
		//NEW!
		new_json.Push(tempObj);
		delete tempObj;
		/////
		
		int index = FindPriorityIndex(CTFPlayerUpgrades[client], "priority", minimum);
		CTFPlayerUpgrades[client].Remove(index);
	}
    
	//PrintToServer("Preferred upgrades for #%d \"%N\"\n", client, client);
	//PrintToServer("%3s %4s %4s %5s %-64s\n", "#", "SLOT", "COST", "INDEX", "ATTRIBUTE");
	
	for (int i = 0; i < new_json.Length; i++) 
	{
		JSONObject info = view_as<JSONObject>(new_json.Get(i));
		CTFPlayerUpgrades[client].Push(info);
		
		//CMannVsMachineUpgradeManager manager = CMannVsMachineUpgradeManager();
		//int cost = SDKCall(g_hGetCostForUpgrade, manager.GetUpgradeByIndex(info.GetInt("index")), info.GetInt("slot"), info.GetInt("pclass"), client);
		//PrintToServer("%3d %4d %4d %5d %-64s", i, info.GetInt("slot"), cost, info.GetInt("index"), manager.GetUpgradeByIndex(info.GetInt("index")).m_szAttribute());
		
		delete info;
	}
	
	delete new_json;
}

void CTFBotPurchaseUpgrades_PurchaseUpgrade(int actor, JSONObject info)
{
	KV_MVM_Upgrade(actor, 1, info.GetInt("slot"), info.GetInt("index"));
	++m_nPurchasedUpgrades[actor];
}

void KV_MvM_UpgradesBegin(int client)
{
	m_nPurchasedUpgrades[client] = 0;

	KeyValues kv = new KeyValues("MvM_UpgradesBegin");
	FakeClientCommandKeyValues(client, kv);
	delete kv;
}

void KV_MVM_Upgrade(int client, int count, int slot, int index)
{
	KeyValues kv = new KeyValues("MVM_Upgrade");
	kv.JumpToKey("upgrade", true);
	kv.SetNum("itemslot", slot);
	kv.SetNum("upgrade",  index);
	kv.SetNum("count",    count);
	FakeClientCommandKeyValues(client, kv);
	delete kv; 
}

void KV_MvM_UpgradesDone(int client)
{
	KeyValues kv = new KeyValues("MvM_UpgradesDone");
	kv.SetNum("num_upgrades", m_nPurchasedUpgrades[client]);
	FakeClientCommandKeyValues(client, kv);
	delete kv;
}

float GetUpgradeInterval()
{
	const float interval = 1.25;
	const float variance = 0.3;
	
	return GetRandomFloat(interval - variance, interval + variance);
}

int FindPriorityIndex(JSONArray array, const char[] key, int value)
{
	int index = -1;
	
	for (int i = 0; i < array.Length; i++)
	{
		JSONObject iObj = view_as<JSONObject>(array.Get(i));
		if (value == iObj.GetInt(key))
		{
			index = i;
			
			delete iObj;
			break;
		}
		
		delete iObj;
	}
	
	return index;
}