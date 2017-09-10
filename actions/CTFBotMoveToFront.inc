float m_vecGoalArea[MAXPLAYERS + 1][3];

float m_ctMoveTimeout[MAXPLAYERS + 1];

bool CTFBotMoveToFront_OnStart(int actor)
{
	int spawn = -1;
	while((spawn = FindEntityByClassname(spawn, "func_respawnroomvisualizer")) != -1)
	{
		if(GetEntProp(spawn, Prop_Data, "m_iDisabled"))
			continue;
		
		if(GetTeamNumber(spawn) == GetTeamNumber(actor))
			continue;
		
		PrintToServer("spawn %i", spawn);
		
		break;
	}
	
	if(spawn == -1)
		return false;
	
	float flSmallestDistance = 99999.0;
	int iBestEnt = -1;
	
	int holo = -1;
	while((holo = FindEntityByClassname(holo, "prop_dynamic")) != -1)
	{
		char strModel[PLATFORM_MAX_PATH];
		GetEntPropString(holo, Prop_Data, "m_ModelName", strModel, PLATFORM_MAX_PATH);
		
		if(!StrEqual(strModel, "models/props_mvm/robot_hologram.mdl"))
			continue;
	
		if (GetEntProp(holo, Prop_Send, "m_fEffects") & 32)
			continue;
		
		if(GetTeamNumber(holo) == GetTeamNumber(actor))
			continue;
		
		float len;
		
		float flDistance = GetVectorDistance(WorldSpaceCenter(spawn), WorldSpaceCenter(holo));
		if (flDistance <= flSmallestDistance && PF_IsPathToVectorPossible(actor, WorldSpaceCenter(holo), len))
		{
			iBestEnt = holo;
			flSmallestDistance = flDistance;
		}
	}
	
	if(iBestEnt == -1)
	{
		ChangeAction(actor, ACTION_IDLE);
		FakeClientCommand(actor, "tournament_player_readystate 1");
		return false;
	}
	
	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(iBestEnt), true, 1000.0, true, true, GetClientTeam(actor));
	if(area == NavArea_Null)
		return false;
	
	area.GetRandomPoint(m_vecGoalArea[actor]);
	
	EquipWeaponSlot(actor, TFWeaponSlot_Melee);
	
	m_ctMoveTimeout[actor] = GetGameTime() + 30.0;
	
	return true;
}

bool CTFBotMoveToFront_Update(int actor)
{
//	PrintToServer("%f %f %f", m_vecGoalArea[actor][0], m_vecGoalArea[actor][1], m_vecGoalArea[actor][2]);
	
	if (m_ctMoveTimeout[actor] < GetGameTime())
	{
		PrintToServer("[%8.3f] CTFBotMoveToFront(#%d): Timeout elapsed!", GetGameTime(), actor);
		ChangeAction(actor, ACTION_IDLE);
		return false;
	}
	
	//Reached goal
	if (GetVectorDistance(m_vecGoalArea[actor], WorldSpaceCenter(actor)) < 80.0)
	{
		FakeClientCommand(actor, "tournament_player_readystate 1");
		
		ChangeAction(actor, ACTION_IDLE);
		
		return false;
	}
	
	PF_SetGoalVector(actor, m_vecGoalArea[actor]);
	
	g_bPath[actor] = true;
	
	return true;
}

void CTFBotMoveToFront_OnEnd(int actor)
{
	m_vecGoalArea[actor] = NULL_VECTOR;
}

bool CTFBotMoveToFront_IsPossible(int actor)
{
	if (!TF2_IsMvM()) 
		return false;
	
/*	if (TF2_GetPlayerClass(actor) == TFClass_Engineer)
		return false;

	if(TF2_GetPlayerClass(actor) == TFClass_Medic)
		return false;*/

	return true;
}