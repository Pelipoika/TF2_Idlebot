float m_vecGoalArea[MAXPLAYERS + 1][3];

float m_ctMoveTimeout[MAXPLAYERS + 1];

bool CTFBotMoveToFront_OnStart(int actor)
{
	//Find robot spawn.
	int spawn = -1;
	while((spawn = FindEntityByClassname(spawn, "func_respawnroomvisualizer")) != -1)
	{
		if(GetEntProp(spawn, Prop_Data, "m_iDisabled"))
			continue;
		
		if(GetTeamNumber(spawn) == GetTeamNumber(actor))
			continue;
		
		//PrintToServer("[%8.3f] CTFBotMoveToFront_OnStart(#%d): spawn == %i", GetGameTime(), actor, spawn);
		
		break;
	}
	
	if(spawn == -1)
		return false;
	
	float flSmallestDistance = 99999.0;
	int iBestEnt = -1;
	
	//Get nearest hologram near robot spawn.
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
		
		float flDistance = GetVectorDistance(WorldSpaceCenter(spawn), WorldSpaceCenter(holo));
		if (flDistance <= flSmallestDistance && PF_IsPathToVectorPossible(actor, WorldSpaceCenter(holo), NULL_FLOAT))
		{
			iBestEnt = holo;
			flSmallestDistance = flDistance;
		}
	}
	
	//Cannot path to target hologram from whereever we are.
	if(iBestEnt == -1)
	{
		ChangeAction(actor, ACTION_IDLE, "CTFBotMoveToFront_OnStart: Cannot path to target hologram from whereever we are. Pressing F4");
		FakeClientCommand(actor, "tournament_player_readystate 1");
		
		//PrintToServer("[%8.3f] CTFBotMoveToFront_OnStart(#%d): iBestEnt == -1", GetGameTime(), actor);
		return false;
	}
	
	//Get random area near hologram to path to.
	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(iBestEnt), true, 1000.0, true, true, GetClientTeam(actor));
	if(area == NavArea_Null)
	{
		//PrintToServer("[%8.3f] CTFBotMoveToFront_OnStart(#%d): Area == NavArea_Null!", GetGameTime(), actor);
		return false;
	}
	
	g_bPath[actor] = true;
	area.GetRandomPoint(m_vecGoalArea[actor]);
	//EquipWeaponSlot(actor, TFWeaponSlot_Melee);
	m_ctMoveTimeout[actor] = GetGameTime() + 50.0;
	
	return true;
}

bool CTFBotMoveToFront_Update(int actor)
{
	if (m_ctMoveTimeout[actor] < GetGameTime())
	{
		FakeClientCommand(actor, "tournament_player_readystate 1");
		PrintToServer("[%8.3f] CTFBotMoveToFront(#%d): Timeout elapsed!", GetGameTime(), actor);
		ChangeAction(actor, ACTION_IDLE, "CTFBotMoveToFront: Timeout elapsed!");
		return false;
	}
	
	//Reached goal
	if (GetVectorDistance(m_vecGoalArea[actor], WorldSpaceCenter(actor)) < 80.0)
	{
		FakeClientCommand(actor, "tournament_player_readystate 1");
		ChangeAction(actor, ACTION_IDLE, "CTFBotMoveToFront: Goal reached!");
		return false;
	}
	
	PF_SetGoalVector(actor, m_vecGoalArea[actor]);
	return true;
}

void CTFBotMoveToFront_OnEnd(int actor)
{
	if(IsValidClientIndex(actor))
		EquipWeaponSlot(actor, TFWeaponSlot_Primary);
		
	m_vecGoalArea[actor] = NULL_VECTOR;
	
	g_bPath[actor] = false;
}

bool CTFBotMoveToFront_IsPossible(int actor)
{
	if (!TF2_IsMvM()) 
		return false;
	
	//Fuck you warning.
	actor--;
	actor++;
	
/*	if (TF2_GetPlayerClass(actor) == TFClass_Engineer)
		return false;

	if(TF2_GetPlayerClass(actor) == TFClass_Medic)
		return false;*/

	return true;
}