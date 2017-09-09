
int g_iStation[MAXPLAYERS + 1];

bool CTFBotGoToUpgradeStation_OnStart(int actor)
{
	g_iStation[actor] = FindClosestUpgradeStation(actor);

	if(g_iStation[actor] == -1)
	{
		g_bPath[actor] = false;
		return false;
	}
	
	EquipWeaponSlot(actor, TFWeaponSlot_Melee);
	
	char map[PLATFORM_MAX_PATH];
	GetCurrentMap(map, PLATFORM_MAX_PATH);
	
	if(StrContains(map, "mvm_mannworks") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-643.9, -2635.2, 384.0}));
	
		g_bPath[actor] = true;
		return true;
	}
	
	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(g_iStation[actor]), true, 8000.0, false, false, TEAM_ANY);
	if(area == NavArea_Null)
		return false;
	
	float center[3];
	area.GetCenter(center);
	
	center[2] += 50.0;
	
	TR_TraceRay(center, WorldSpaceCenter(g_iStation[actor]), MASK_ALL, RayType_EndPoint);
	TR_GetEndPosition(center);
	
	PF_SetGoalVector(actor, center);
	
	g_bPath[actor] = true;
	return true;
}

bool CTFBotGoToUpgradeStation_Update(int actor)
{
	int station = g_iStation[actor];
	if (!IsValidEntity(station))
	{
		PrintToServer("Upgrade station is invalid.");
		g_bPath[actor] = false;
		return false;
	}
	
	if (IsStandingAtUpgradeStation(actor)) 
	{
		BotAim(actor).AimHeadTowards(WorldSpaceCenter(station), IMPORTANT, 3.0, "Look at our upgrade station");
		PrintToServer("Reached upgrade station; buying upgrades.");
		ChangeAction(actor, ACTION_UPGRADE);
		return false;
	}
	
	g_bPath[actor] = true;
	
	return true;
}

void CTFBotGoToUpgradeStation_OnEnd(int actor)
{
	g_iStation[actor] = -1;
}

int FindClosestUpgradeStation(int actor)
{
	int stations[MAXPLAYERS+1];
	int stationcount;
	
	int i = -1;
	while ((i = FindEntityByClassname(i, "func_upgradestation")) != -1)
	{
		if(GetEntProp(i, Prop_Data, "m_bDisabled"))
			continue;
		
		NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(i), true, 8000.0, false, false, TEAM_ANY);
		if(area == NavArea_Null)
			return false;
		
		float center[3];
		area.GetCenter(center);
		
		center[2] += 50.0;
		
		TR_TraceRay(center, WorldSpaceCenter(i), MASK_ALL, RayType_EndPoint);
		TR_GetEndPosition(center);
		
		if(!PF_IsPathToVectorPossible(actor, center, NULL_FLOAT))
			continue;
		
		stations[stationcount] = i;
		stationcount++;
	}
	
	return stations[GetRandomInt(0, stationcount - 1)];
}