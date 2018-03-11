
float m_ctSentrySafe[MAXPLAYERS + 1];               // +0x4814
float m_ctSentryCooldown[MAXPLAYERS + 1];           // +0x4820
float m_ctTeleCooldown[MAXPLAYERS + 1];             // +0x482c

float m_ctFindNestHint[MAXPLAYERS + 1];             // +0x4838
float m_ctAdvanceNestSpot[MAXPLAYERS + 1];          // +0x4844

float m_ctRecomputePathMvMEngiIdle[MAXPLAYERS + 1];

NavArea m_aNestArea[MAXPLAYERS + 1] = {NavArea_Null, ...};

stock bool CTFBotMvMEngineerIdle_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer;
}

bool CTFBotMvMEngineerIdle_OnStart(int actor)
{
	//g_bUpdateLookingAroundForEnemies[actor] = false;
	
	m_ctFindNestHint[actor] = 0.0;
	m_ctRecomputePathMvMEngiIdle[actor] = 0.0;
	
	return true;
}

bool CTFBotMvMEngineerIdle_Update(int actor)
{
	bool bShouldAdvance = CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor);
	if(bShouldAdvance)
	{
		m_aNestArea[actor] = PickBuildArea(actor);
		
		FakeClientCommand(actor, "destroy 2");
		
		//RIGHT NOW
		m_ctFindNestHint[actor] = GetGameTime();
		
		PrintToServer("ADVANCE");
	}
			
	int sentry = TF2_GetObject(actor, TFObject_Sentry);
	if ((m_aNestArea[actor] == NavArea_Null || bShouldAdvance) || sentry == INVALID_ENT_REFERENCE)
	{
		//HasStarted && !IsElapsed
		if (m_ctFindNestHint[actor] > 0.0 && m_ctFindNestHint[actor] > GetGameTime()) 
		{
			if(m_aNestArea[actor] != NavArea_Null)
				g_bPath[actor] = true;
			
			return false;
		}
		
		//Start
		m_ctFindNestHint[actor] = GetGameTime() + (GetRandomFloat(1.0, 2.0));
		
		m_aNestArea[actor] = PickBuildArea(actor);
	}
	
	if (m_aNestArea[actor] != NavArea_Null)
	{
		if (sentry != INVALID_ENT_REFERENCE)
		{
			if(GetEntProp(sentry, Prop_Send, "m_iMaxHealth") >= GetEntProp(sentry, Prop_Send, "m_iHealth") && !GetEntProp(sentry, Prop_Send, "m_bBuilding")
			&& GetEntProp(sentry, Prop_Send, "m_iUpgradeLevel") >= 3
			&& GetEntProp(sentry, Prop_Send, "m_iAmmoShells") > 0)
			{
				m_ctSentrySafe[actor] = GetGameTime() + 3.0;
			}
			
			m_ctSentryCooldown[actor] = GetGameTime() + 3.0;	
		}
		else 
		{
			/* do not have a sentry; retreat for a few seconds if we had a
			 * sentry before this; then build a new sentry */
			if (m_ctSentryCooldown[actor] < GetGameTime()) 
			{
				m_ctSentryCooldown[actor] = GetGameTime() + 3.0;

				ChangeAction(actor, ACTION_MVM_ENGINEER_BUILD_SENTRYGUN, "No sentry - building a new one");
				FakeClientCommand(actor, "destroy 0");
				
				return true;
			}
		/*	else 
			{
				return ActionResult<CTFBot>::SuspendFor(new CTFBotRetreatToCover(1.0), "Lost my sentry - retreat!");
			}*/
		}
	}	
	
	//TODO: 
	//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_idle.cpp#L150-L200
	int dispenser = TF2_GetObject(actor, TFObject_Dispenser);
	if (m_aNestArea[actor] != NavArea_Null)
	{
		//Won't build a dispenser unless sentry is safe.
		if(m_ctSentrySafe[actor] < GetGameTime())
		{
			m_ctTeleCooldown[actor] = GetGameTime() + 3.0;
		}
		
		if(m_ctTeleCooldown[actor] < GetGameTime() && dispenser == INVALID_ENT_REFERENCE)
		{
			m_ctTeleCooldown[actor] = GetGameTime() + 3.0;
			
			ChangeAction(actor, ACTION_MVM_ENGINEER_BUILD_DISPENSER, "Sentry safe, No dispenser - building a dispenser");
			
			return true;
		}
	}
	
	if (dispenser != INVALID_ENT_REFERENCE && m_ctTeleCooldown[actor] < GetGameTime())
	{
		if(GetEntProp(dispenser, Prop_Send, "m_iUpgradeLevel") < 3 || GetEntProp(dispenser, Prop_Send, "m_iMaxHealth") < GetEntProp(dispenser, Prop_Send, "m_iHealth"))
		{
			float dist = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(dispenser));
			
			if (m_ctRecomputePathMvMEngiIdle[actor] < GetGameTime()) 
			{
				m_ctRecomputePathMvMEngiIdle[actor] = GetGameTime() + GetRandomFloat(1.0, 2.0);
				
				float dir[3];
				SubtractVectors(GetAbsAngles(dispenser), GetAbsOrigin(actor), dir);
				NormalizeVector(dir, dir);
				
				float goal[3]; goal = GetAbsOrigin(dispenser);
				goal[0] -= (50.0 * dir[0]);
				goal[1] -= (50.0 * dir[1]);
				goal[2] -= (50.0 * dir[2]);
				
				PF_SetGoalVector(actor, goal);
				
				m_iRouteType[actor] = FASTEST_ROUTE;
				g_bPath[actor] = true;
			}
			
			if (dist < 90.0) 
			{
				g_iAdditionalButtons[actor] |= IN_DUCK;
				
				EquipWeaponSlot(actor, TFWeaponSlot_Melee);
				
				g_bUpdateLookingAroundForEnemies[actor] = false;
				
				BotAim(actor).AimHeadTowards(WorldSpaceCenter(dispenser), CRITICAL, 1.0, "Work on my Dispenser");
				BotAim(actor).PressFireButton();
			}
			
			return false;
		}
	}
	
	if (sentry != INVALID_ENT_REFERENCE) 
	{
		float dist = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(sentry));
		
		if (m_ctRecomputePathMvMEngiIdle[actor] < GetGameTime()) 
		{
			m_ctRecomputePathMvMEngiIdle[actor] = GetGameTime() + GetRandomFloat(1.0, 2.0);
			
			float dir[3];
			GetAngleVectors(GetTurretAngles(sentry), dir, NULL_VECTOR, NULL_VECTOR);
			
			float goal[3]; goal = GetAbsOrigin(sentry);
			goal[0] -= (50.0 * dir[0]);
			goal[1] -= (50.0 * dir[1]);
			goal[2] -= (50.0 * dir[2]);
			
			PF_SetGoalVector(actor, goal);
			
			m_iRouteType[actor] = SAFEST_ROUTE;
			g_bPath[actor] = true;
		}
		
		if (dist < 90.0) 
		{
			g_iAdditionalButtons[actor] |= IN_DUCK;
			
			EquipWeaponSlot(actor, TFWeaponSlot_Melee);
			
			g_bUpdateLookingAroundForEnemies[actor] = false;
			
			BotAim(actor).AimHeadTowards(WorldSpaceCenter(sentry), CRITICAL, 1.0, "Work on my Sentry");
			BotAim(actor).PressFireButton();
		}
	}
	
	return true;
}

stock bool IsBaseObject(int entity)
{
	return HasEntProp(entity, Prop_Send, "m_bPlasmaDisable");
}

stock void CTFBotMvMEngineerIdle_OnEnd(int actor)
{
	//Something here so we don't get a compile warning
}

stock bool CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(int actor)
{
	if (m_aNestArea[actor] == NavArea_Null)
		return false;
	
	if (m_ctAdvanceNestSpot[actor] <= 0.0)
	{
		m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
		return false;
	}
	
	int obj = TF2_GetObject(actor, TFObject_Sentry);
	if (obj != INVALID_ENT_REFERENCE &&	GetEntProp(obj, Prop_Send, "m_iHealth") < GetEntProp(obj, Prop_Send, "m_iMaxHealth"))
	{
		m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
		return false;
	}
	
	//IsElapsed
	if (GetGameTime() > m_ctAdvanceNestSpot[actor])
	{
		m_ctAdvanceNestSpot[actor] = -1.0;
	}
	
	JSONObject bombinfo = new JSONObject();
	if (!GetBombInfo(bombinfo)) 
	{
		delete bombinfo;
		return false;
	}
	
	float m_flBombTargetDistance = view_as<float>(LoadFromAddress(view_as<Address>(m_aNestArea[actor]) + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
	
	bool bigger = (m_flBombTargetDistance > bombinfo.GetFloat("hatch_dist_back"));
	
	//PrintToServer("m_flBombTargetDistance %f > bombinfo.hatch_dist_back %f = %s", m_flBombTargetDistance, bombinfo.GetFloat("hatch_dist_back"), bigger ? "Yes" : "No");

	delete bombinfo;
	return bigger;
}

stock NavArea PickBuildArea(int client)
{
	int iAreaCount = LoadFromAddress(navarea_count, NumberType_Int32);

	//Check that this map has any nav areas
	if ( iAreaCount <= 0 ) {
		return NavArea_Null;
	}
	
	JSONObject bombinfo = new JSONObject();
	
	if (!GetBombInfo(bombinfo)) 
	{	
		delete bombinfo;
		return PickBuildAreaPreRound(client);
	}
	
	float vecTargetPos[3];
	vecTargetPos[0] = bombinfo.GetFloat("closest_pos_x");
	vecTargetPos[1] = bombinfo.GetFloat("closest_pos_y");
	vecTargetPos[2] = bombinfo.GetFloat("closest_pos_z");
	
	//GetNearestNavArea_Vec(const float pos[3], bool anyZ = false, float maxDist = 10000.0, bool checkLOS = false, bool checkGround = true, int team = TEAM_ANY);
	NavArea bombArea = TheNavMesh.GetNearestNavArea_Vec(vecTargetPos, false, 90000.0, false, true, TEAM_ANY);
	if(bombArea == NavArea_Null)
	{
		//PrintToServer("bombArea IS NULL");
	
		delete bombinfo;
		return NavArea_Null;
	}
	
	if(HasTFAttributes(bombArea, (BLUE_SPAWN_ROOM | RED_SPAWN_ROOM)))
	{
		//PrintToServer("bombArea IN SPAWN");
	
		delete bombinfo;
		return NavArea_Null;
	}

	int iValidAreas = 0;
	int iSkippedAreas = 0;
	
	ArrayList ForwardAreas = new ArrayList();
	ArrayList SemiGoodAreas = new ArrayList();
	
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
		
		if (HasTFAttributes(area, (BLUE_SPAWN_ROOM | RED_SPAWN_ROOM)))
			continue;
		
		float m_flBombTargetDistanceAtArea = view_as<float>(LoadFromAddress(view_as<Address>(area)     + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
		float m_flBombTargetDistanceAtBomb = view_as<float>(LoadFromAddress(view_as<Address>(bombArea) + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
		
		float center[3];
		area.GetCenter(center);
		center[2] += 60.0;
		
		float flDistance = GetVectorDistance(center, vecTargetPos);
		
		//Skip areas too close and too far
		if(flDistance <= 750.0)
		{
			SemiGoodAreas.Push(area);
		
			iSkippedAreas++;
			continue;
		}
		
		bool bForwardOfBomb = (m_flBombTargetDistanceAtArea < m_flBombTargetDistanceAtBomb);
		if(!bForwardOfBomb)
		{
			SemiGoodAreas.Push(area);
		
			iSkippedAreas++;
			continue;
		}
		
		if(flDistance >= 1300.0)
		{
			iSkippedAreas++;
			continue;
		}
		
		if (IsLineOfFireClear(center, vecTargetPos))
		{
			iValidAreas++;
			ForwardAreas.Push(area);
		}
	}
	
	//PrintToServer("%i / %i areas potentially visible! skipped %i areas", iValidAreas, iAreaCount, iSkippedAreas);
	
	NavArea randomArea = bombArea;
	
	if(ForwardAreas.Length > 0)
		randomArea = ForwardAreas.Get(GetRandomInt(0, ForwardAreas.Length - 1));
	else if(SemiGoodAreas.Length > 0)
		randomArea = SemiGoodAreas.Get(GetRandomInt(0, SemiGoodAreas.Length - 1));
	
	delete bombinfo;
	delete ForwardAreas;
	delete SemiGoodAreas;
	
	return randomArea;
}

//Returns a nav area visible to a random enemy respawn room exit.
stock NavArea PickBuildAreaPreRound(int client)
{
	int iAreaCount = LoadFromAddress(navarea_count, NumberType_Int32);

	//Check that this map has any nav areas
	if ( iAreaCount <= 0 ) {
		return NavArea_Null;
	}

	ArrayList EnemySpawnExits = new ArrayList();
	//ArrayList EnemySpawnExitsInSomeRange = new ArrayList();
	
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
		
		if (GetEnemyTeam(client) == TFTeam_Blue && !HasTFAttributes(area, BLUE_SPAWN_ROOM|SPAWN_ROOM_EXIT))
			continue;
			
		if (GetEnemyTeam(client) == TFTeam_Red && !HasTFAttributes(area, RED_SPAWN_ROOM|SPAWN_ROOM_EXIT))
			continue;
		
		EnemySpawnExits.Push(area);
	}
	
	if(EnemySpawnExits.Length <= 0)
	{
		delete EnemySpawnExits;
		return NavArea_Null;
	}
	
	NavArea RandomEnemySpawnExit = EnemySpawnExits.Get(GetRandomInt(0, EnemySpawnExits.Length - 1));
	
	float vecExitCenter[3];
	RandomEnemySpawnExit.GetCenter(vecExitCenter);
	vecExitCenter[2] += 64.0;

	ArrayList AreasCloser        = new ArrayList();	//Not necessarily visible but still <= 3000.0
	ArrayList VisibleAreas       = new ArrayList();
	ArrayList VisibleAreasCloser = new ArrayList();	//<= 3000.0
	
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
		
		if (HasTFAttributes(area, (BLUE_SPAWN_ROOM|RED_SPAWN_ROOM)))
			continue;
			
		float center[3]; area.GetCenter(center);
		
		float flDistance = GetVectorDistance(center, vecExitCenter);
		
		if(flDistance <= 2500.0)
		{
			AreasCloser.Push(area);
		}
		
		if (!area.IsVisible(vecExitCenter))
			continue;
		
		if(flDistance <= 3000.0) {
			VisibleAreasCloser.Push(area);
		} else {
			VisibleAreas.Push(area);
		}
	}
	
	PrintToServer("%i VisibleAreas | %i VisibleAreasCloser | %i AreasCloser", VisibleAreas.Length, VisibleAreasCloser.Length, AreasCloser.Length);
	
	NavArea bestArea = NavArea_Null;
	
	if(VisibleAreasCloser.Length > 0) {
		bestArea = VisibleAreasCloser.Get(GetRandomInt(0, VisibleAreasCloser.Length - 1));
	} else if(VisibleAreas.Length > 0) {
		bestArea = VisibleAreas.Get(GetRandomInt(0, VisibleAreas.Length - 1));
	} else if(AreasCloser.Length > 0) {
		bestArea = AreasCloser.Get(GetRandomInt(0, AreasCloser.Length - 1));
	}
	
	delete AreasCloser;
	delete VisibleAreas;
	delete VisibleAreasCloser;
		
	return bestArea;
}
