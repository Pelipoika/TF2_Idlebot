/*
ConVar tf_bot_medic_stop_follow_range("tf_bot_medic_stop_follow_range", "75", FCVAR_CHEAT);
ConVar tf_bot_medic_start_follow_range("tf_bot_medic_start_follow_range", "250", FCVAR_CHEAT);
ConVar tf_bot_medic_max_heal_range("tf_bot_medic_max_heal_range", "600", FCVAR_CHEAT);
ConVar tf_bot_medic_debug("tf_bot_medic_debug", "0", FCVAR_CHEAT);
ConVar tf_bot_medic_max_call_response_range("tf_bot_medic_max_call_response_range", "1000", FCVAR_CHEAT);
ConVar tf_bot_medic_cover_test_resolution("tf_bot_medic_cover_test_resolution", "8", FCVAR_CHEAT);
*/

//tf_medigun_autoheal 0 / 1
bool g_bMedigunAutoHeal[MAXPLAYERS + 1];

int m_hPatient[MAXPLAYERS + 1];

float g_flNextHideSpotComputation[MAXPLAYERS + 1];

float g_flReloadRapeCooldown[MAXPLAYERS + 1];

stock bool CTFBotMedicHeal_OnStart(int actor)
{
	m_hPatient[actor] = -1;
	
	g_flNextHideSpotComputation[actor] = -1.0;
	g_flReloadRapeCooldown[actor] = -1.0;
	
	QueryClientConVar(actor, "tf_medigun_autoheal", OnMedigunModeGet, GetClientUserId(actor));
	
	g_bPath[actor] = true;
	
	return true;
}

public void OnMedigunModeGet(QueryCookie cookie, int client, ConVarQueryResult result, const char[] cvarName, const char[] cvarValue, any value)
{
	int queriedClient = GetClientOfUserId(value);
	if(queriedClient <= 0)
		return;
	
	g_bMedigunAutoHeal[queriedClient] = !!StringToInt(cvarValue);
}


bool CTFBotMedicHeal_Update(int actor)
{
	m_hPatient[actor] = CTFBotMedicHeal_SelectPatient(actor, m_hPatient[actor]);
	
	if(m_hPatient[actor] <= 0)
		return false;
	
	if(GetClientHealth(actor) <= CTFBotMedicHeal_GetUberHealthThreshold(actor) 
	|| GetClientHealth(m_hPatient[actor]) <= CTFBotMedicHeal_GetUberHealthThreshold(actor))
	{
		BotAim(actor).PressAltFireButton();
	}
	
	int hPatient = 0;
	
	bool bVisibleToEnemy = (CTFBotMedicHeal_IsVisibleToEnemy(actor, WorldSpaceCenter(actor)) || CTFBotMedicHeal_IsVisibleToEnemy(actor, GetEyePosition(actor)));
	
	float flDistanceToPatient = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(m_hPatient[actor]));
	
	if(IsWeapon(actor, TF_WEAPON_MEDIGUN)) 
	{
		hPatient = GetEntPropEnt(GetActiveWeapon(actor), Prop_Send, "m_hHealingTarget");
	}
	
	//Cycle resistances
	if(g_flReloadRapeCooldown[actor] <= GetGameTime() && IsVaccinator(actor) && hPatient != INVALID_ENT_REFERENCE)
	{
		//Vaccinator current resistance type.
		int iResistType = GetResistType(actor);
		
		//Patients last taken damage type.
		int iLastDmgType = GetLastDamageType(hPatient);
		
		if(iLastDmgType & DMG_BULLET     && iResistType != 0) 
		{
			PrintToServer("DMG_BULLET PLEASE %i %i", iLastDmgType, iResistType);
			BotAim(actor).PressReloadButton();
		}
		else if(iLastDmgType & DMG_BLAST && iResistType != 1) 
		{
			PrintToServer("DMG_BLAST PLEASE %i %i", iLastDmgType, iResistType);
			BotAim(actor).PressReloadButton();
		}
		else if(iLastDmgType & DMG_BURN  && iResistType != 2)
		{
			PrintToServer("DMG_BURN PLEASE %i %i", iLastDmgType, iResistType);
			BotAim(actor).PressReloadButton();
		}
		
		g_flReloadRapeCooldown[actor] = GetGameTime() + GetRandomFloat(0.5, 0.75);
	}
	
	EquipWeaponSlot(actor, flDistanceToPatient > 500.0 ? TFWeaponSlot_Primary : TFWeaponSlot_Secondary);
	
	//PrintToServer("bVisibleToEnemy %i distance %f", bVisibleToEnemy, flDistanceToPatient);
	
	if (hPatient == m_hPatient[actor]) //Healing patiend, find a spot to cover in while healing our patient.
	{
		if (bVisibleToEnemy && g_flNextHideSpotComputation[actor] <= GetGameTime())
		{
			float vecBestHideSpot[3]; 
			bool bFoundSpot = CTFBotMedicHeal_ComputeFollowPosition(hPatient, vecBestHideSpot);
			
			if(bFoundSpot)
			{			
				PF_SetGoalVector(actor, vecBestHideSpot);
				
				if(GetVectorDistance(GetAbsOrigin(actor), vecBestHideSpot) < 60.0)
					g_bPath[actor] = false;
				else
					g_bPath[actor] = true;
			}
			
			g_flNextHideSpotComputation[actor] = GetGameTime() + GetRandomFloat(0.5, 3.0);
		}
		else
		{
			g_bPath[actor] = flDistanceToPatient > 300.0;
		}
		
		//Autoheal not on, keep holding M1
		if(!g_bMedigunAutoHeal[actor])
		{
			BotAim(actor).PressFireButton();
		}
		
		if(IsLineOfFireClear(GetEyePosition(actor), WorldSpaceCenter(m_hPatient[actor])))
		{
			BotAim(actor).AimHeadTowardsEntity(m_hPatient[actor], OVERRIDE_ALL, 1.0, "Look at our patient");
		}
		
		g_bUpdateLookingAroundForEnemies[actor] = false;
	}
	else
	{
		PF_SetGoalEntity(actor, m_hPatient[actor]);
		g_bPath[actor] = true;
		
		if(IsLineOfFireClear(GetEyePosition(actor), WorldSpaceCenter(m_hPatient[actor])))
		{
			BotAim(actor).AimHeadTowardsEntity(m_hPatient[actor], OVERRIDE_ALL, 1.0, "Look at our desired patient");
			
			if(flDistanceToPatient < 400.0)
			{
				if(!g_bMedigunAutoHeal[actor] || (hPatient != m_hPatient[actor] && BotAim(actor).IsHeadAimingOnTarget()))
					BotAim(actor).PressFireButton();
			}
			
			g_bUpdateLookingAroundForEnemies[actor] = false;
		}
		else
			g_bUpdateLookingAroundForEnemies[actor] = true;
	}
	
	return true;
}

stock bool CTFBotMedicHeal_ComputeFollowPosition(int client, float spot[3])
{
	//tf_bot_medic_cover_test_resolution 
	const int items = 20;
	float r = 100.0 + GetRandomFloat(0.0, 100.0);
	
	float myPos[3]; myPos = WorldSpaceCenter(client);

	float vecBestHideSpot[3];

	JSONArray HidingSpots = new JSONArray();
	
	JSONArray SpotCluster = null;
	for(int i = 0; i < items; i++) 
	{
		float xyz[3];
		xyz[0] = myPos[0] + r * Cosine(2 * FLOAT_PI * i / items);
		xyz[1] = myPos[1] + r *   Sine(2 * FLOAT_PI * i / items);   
		xyz[2] = myPos[2];
		
		TR_TraceRayFilter(myPos, xyz, 0x2006081, RayType_EndPoint, NextBotTraceFilterIgnoreActors);
		if(!CTFBotMedicHeal_IsVisibleToEnemy(client, xyz))
		{
			TR_GetEndPosition(xyz);
			
			//Move the endpoint away from walls
			float norm[3]; MakeVectorFromPoints(myPos, xyz, norm);
			NormalizeVector(norm, norm);
			ScaleVector(norm, 24.0);
			SubtractVectors(xyz, norm, xyz);
			
			//Start new cluster on hiding spot found.
			if(SpotCluster == null)	{
				SpotCluster = new JSONArray();
			}
			
			JSONObject HidingSpot = new JSONObject();
			HidingSpot.SetInt("index", i);
			HidingSpot.SetFloat("x", xyz[0]);
			HidingSpot.SetFloat("y", xyz[1]);
			HidingSpot.SetFloat("z", xyz[2]);		
			SpotCluster.Push(HidingSpot);
			delete HidingSpot;
			
			//Line(myPos, xyz, 0, 255, 0, 4.0);
			//Cross3D(xyz, 5.0, 0, 255, 0, 4.0);
		}
		else
		{
			if(SpotCluster != null)
			{
				HidingSpots.Push(SpotCluster);
				delete SpotCluster;
			}
			
			//Line(myPos, xyz, 255, 0, 0, 4.0);
			//Cross3D(xyz, 5.0, 255, 0, 0, 4.0);
		}
	}
	
	//PrintToServer("\nHiding spot clusters %i\n", HidingSpots.Length);
	
	int iLargestClusterIndex = -1;
	int iLargestCluster = -1;
	
	for (int i = 0; i < HidingSpots.Length; i++) 
	{
		JSONArray aSpotCluster = view_as<JSONArray>(HidingSpots.Get(i));
		
		//If this cluster has more hidingspots than our current largest,
		//Set it as newest largest.
		if(aSpotCluster.Length > iLargestCluster)
		{
			iLargestCluster = aSpotCluster.Length;
			iLargestClusterIndex = i;
		}
		
		//PrintToServer("- Cluster %i", i);
	/*	for (int o = 0; o < aSpotCluster.Length; o++)
		{
			JSONObject HidingSpot = view_as<JSONObject>(aSpotCluster.Get(o));
			
			PrintToServer("%1d %3d %4f %-5f %-8f", o, HidingSpot.GetInt("index"), HidingSpot.GetFloat("x"), HidingSpot.GetFloat("y"), HidingSpot.GetFloat("z"));
			
			delete HidingSpot;
		}*/
		
		delete aSpotCluster;
	}
	
	if(iLargestClusterIndex != -1)
	{
		JSONArray aSpotCluster = view_as<JSONArray>(HidingSpots.Get(iLargestClusterIndex));
		
		int iSpotsLookedAt = 0;
	
		for (int i = 0; i < aSpotCluster.Length; i++)
		{
			JSONObject HidingSpot = view_as<JSONObject>(aSpotCluster.Get(i));
			
			float vecSpot[3];
			vecSpot[0] = HidingSpot.GetFloat("x");
			vecSpot[1] = HidingSpot.GetFloat("y");
			vecSpot[2] = HidingSpot.GetFloat("z");
			iSpotsLookedAt++;
			
			delete HidingSpot;
				
			AddVectors(vecSpot, vecBestHideSpot, vecBestHideSpot);
		}
		
		vecBestHideSpot[0] /= iSpotsLookedAt;
		vecBestHideSpot[1] /= iSpotsLookedAt;
		vecBestHideSpot[2] /= iSpotsLookedAt;
	}
	else
	{
		delete HidingSpots;
		return false;
	}
	
	delete HidingSpots;
	
	//PrintToServer("\n Largest cluster is index %i containing %i hidingspots", iLargestClusterIndex, iLargestCluster);
	
	spot = vecBestHideSpot;
	
	return true;
}

stock bool CTFBotMedicHeal_IsStable(int client)
{
	if(GetTimeSinceLastInjury(client, view_as<int>(GetEnemyTeam(client))) < 3.0)
		return false
	
	if((GetClientHealth(client) / GetMaxHealth(client)) < 1.0)
		return false;
	
	if(TF2_IsPlayerInCondition(client, TFCond_OnFire))
		return false;
	
	return TF2_IsPlayerInCondition(client, TFCond_Bleeding);
}

bool CTFBotMedicHeal_IsVisibleToEnemy(int client, float position[3])
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if(i == client)
			continue;
			
		if(!IsClientInGame(i))
			continue;
			
		if(!IsPlayerAlive(i))
			continue;
			
		if(TF2_GetClientTeam(i) != GetEnemyTeam(client))
			continue;
		
		//Medics arent really a threat.
		if(TF2_GetPlayerClass(i) == TFClass_Medic)
			continue;
		
		if(IsLineOfFireClear(GetEyePosition(i), position))
			return true
	}
	
	return false;
}

int CTFBotMedicHeal_GetUberHealthThreshold(int actor)
{
	return IsVaccinator(actor) ? (RoundToFloor(GetMaxHealth(actor) / 1.3)) : 50;
}

int CTFBotMedicHeal_SelectPatient(int actor, int old_patient)
{
	if(IsValidClientIndex(old_patient) && IsPlayerAlive(old_patient) 
	&& GetClientTeam(old_patient) == GetClientTeam(actor))
		return old_patient;

	int playerarray[MAXPLAYERS+1];
	int playercount;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == actor)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		//Ignore dead
		if(!IsPlayerAlive(i))
			continue;
		
		//Ignore non teammates
		if(TF2_GetClientTeam(i) != TF2_GetClientTeam(actor))
			continue;
		
		//TODO: replace with PointInRespawnRoom
		if(IsInvulnerable(i))
			continue;
		
		//Where are you
		PF_UpdateLastKnownArea(i)
		NavArea lastKnownArea = PF_GetLastKnownArea(i);
		if(lastKnownArea == NavArea_Null)
			continue;
		
		//Ignore unpathable 
		if (!PF_IsPathToEntityPossible(actor, i))
			continue;
		
		playerarray[playercount] = i;
		playercount++;
	}
	
	if(playercount > 0)
	{
		return playerarray[GetRandomInt(0, playercount-1)];
	}

	return -1;
}

void CTFBotMedicHeal_OnEnd(int actor)
{
	m_hPatient[actor] = -1;
	g_bUpdateLookingAroundForEnemies[actor] = true;
	
	EquipWeaponSlot(actor, TFWeaponSlot_Primary);
}

stock bool CTFBotMedicHeal_IsPossible(int actor)
{
	if(TF2_GetPlayerClass(actor) != TFClass_Medic)
		return false;
	
	return true;
}