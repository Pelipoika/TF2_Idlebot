
int g_iStation[MAXPLAYERS + 1];

float m_ctUpgradeTimeout[MAXPLAYERS + 1];

bool CTFBotGoToUpgradeStation_OnStart(int actor)
{
	g_iStation[actor] = FindClosestUpgradeStation(actor);

	if(g_iStation[actor] <= MaxClients)
	{
		return false;
	}
	
	g_bUpdateLookingAroundForEnemies[actor] = false;
	
	m_ctUpgradeTimeout[actor] = GetGameTime() + 50.0;
	
	BotAim(actor).ReleaseFireButton();
	EquipWeaponSlot(actor, TFWeaponSlot_Melee);

	char map[PLATFORM_MAX_PATH];
	GetCurrentMap(map, PLATFORM_MAX_PATH);
	
	if(StrContains(map, "mvm_mannworks") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-643.9, -2635.2, 384.0}));
		return true;
	}
	else if(StrContains(map, "mvm_teien") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({4613.1, -6561.9, 260.0}));
		return true;
	}
	else if(StrContains(map, "mvm_sequoia") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-5117.0, -377.3, 4.5}));
		return true;
	}
	else if(StrContains(map, "mvm_highground") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-2013.0, 4561.0, 448.0}));
		return true;
	}
	else if(StrContains(map, "mvm_newnormandy") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-345.0, 4178.0, 205.0}));
		return true;
	}
	else if(StrContains(map, "mvm_snowfall") != -1)
	{
		PF_SetGoalVector(actor, view_as<float>({-26.0, 792.0, -159.0}));
		return true;
	}
	
	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(g_iStation[actor]), true, 1000.0, false, false, TEAM_ANY);
	if(area == NavArea_Null)
		return false;
	
	float center[3];
	area.GetRandomPoint(center);
	
	center[2] += 50.0;
	
	TR_TraceRayFilter(center, WorldSpaceCenter(g_iStation[actor]), MASK_PLAYERSOLID, RayType_EndPoint, NextBotTraceFilterIgnoreActors);
	TR_GetEndPosition(center);
	
	PF_SetGoalVector(actor, center);
	
	g_bPath[actor] = true;
	
	return true;
}

bool CTFBotGoToUpgradeStation_Update(int actor)
{
	if (m_ctUpgradeTimeout[actor] < GetGameTime())
	{
		PrintToServer("[%8.3f] CTFBotGoToUpgradeStation(#%d): Timeout elapsed!", GetGameTime(), actor);
		
		//Try jump
		g_iAdditionalButtons[actor] |= IN_JUMP;
		
		float there[3];
		bool bYes = false;
		
		if (PF_GetFutureSegment(actor, 1, there)) 
		{ 
			bYes = true; 
		}
		else if(PF_GetFutureSegment(actor, 0, there)) 
		{ 
			bYes = true; 
		}
		
		if(bYes)
		{
			there[2] += 18.0;
			TeleportEntity(actor, there, NULL_VECTOR, NULL_VECTOR);
		}
		else
		{		
			NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(actor), true, 4000.0, false, false, GetClientTeam(actor));
			if(area == NavArea_Null)
			{
				ChangeAction(actor, ACTION_IDLE, "Cannot reach upgrade station.");
			}
			else
			{
				float center[3];
				area.GetCenter(center);
				
				center[2] += GetEntPropFloat(actor, Prop_Send, "m_flStepSize");
				
				TeleportEntity(actor, center, NULL_VECTOR, NULL_VECTOR);
			}
		}
		
		m_ctUpgradeTimeout[actor] = GetGameTime() + 30.0;
		
		return false;
	}

	int station = g_iStation[actor];
	if (!IsValidEntity(station))
	{
		PrintToServer("[%8.3f] CTFBotGoToUpgradeStation(#%d): Upgrade station is invalid.", GetGameTime(), actor);
		ChangeAction(actor, ACTION_IDLE, "Upgrade station is invalid.");
		return false;
	}
	
	if (IsStandingAtUpgradeStation(actor)) 
	{
		BotAim(actor).AimHeadTowards(WorldSpaceCenter(station), IMPORTANT, 3.0, "Look at our upgrade station");
		ChangeAction(actor, ACTION_UPGRADE, "Reached upgrade station; buying upgrades.");
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
		
		TR_TraceRay(center, WorldSpaceCenter(i), MASK_PLAYERSOLID, RayType_EndPoint);
		TR_GetEndPosition(center);
		
		if(!PF_IsPathToVectorPossible(actor, center))
			continue;
		
		stations[stationcount] = i;
		stationcount++;
	}
	
	return stations[GetRandomInt(0, stationcount - 1)];
}