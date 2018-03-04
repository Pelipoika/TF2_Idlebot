
float m_ctSentrySafe[MAXPLAYERS + 1];               // +0x4814
float m_ctSentryCooldown[MAXPLAYERS + 1];           // +0x4820
//float m_ctTeleCooldown[MAXPLAYERS + 1];             // +0x482c

float m_ctFindNestHint[MAXPLAYERS + 1];             // +0x4838
float m_ctAdvanceNestSpot[MAXPLAYERS + 1];          // +0x4844

float m_ctRecomputePathMvMEngiIdle[MAXPLAYERS + 1];

NavArea m_aNestArea[MAXPLAYERS + 1] = {NavArea_Null, ...};

bool CTFBotMvMEngineerIdle_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer;
}

bool CTFBotMvMEngineerIdle_OnStart(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = false;
	
	m_ctFindNestHint[actor] = 0.0;
	m_ctRecomputePathMvMEngiIdle[actor] = 0.0;
	
	return true;
}

bool CTFBotMvMEngineerIdle_Update(int actor)
{
	bool bShouldAdvance = CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor);
	if(bShouldAdvance)
	{
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
			if(GetEntProp(sentry, Prop_Send, "m_iMaxHealth") >= GetEntProp(sentry, Prop_Send, "m_iHealth") 
			&& !GetEntProp(sentry, Prop_Send, "m_bBuilding"))
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
				
				return true;
			}
		/*	else 
			{
				return ActionResult<CTFBot>::SuspendFor(new CTFBotRetreatToCover(1.0), "Lost my sentry - retreat!");
			}*/
		}
	}	
	
	//TODO: Tele stuff here
	//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_idle.cpp#L150-L200
	
	if (sentry != INVALID_ENT_REFERENCE) 
	{
		float dist = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(sentry));
		
		if (dist < 90.0) {
			g_iAdditionalButtons[actor] |= IN_DUCK;
		}
		
		if (m_ctRecomputePathMvMEngiIdle[actor] < GetGameTime()) 
		{
			m_ctRecomputePathMvMEngiIdle[actor] = GetGameTime() + GetRandomFloat(1.0, 2.0);
			
			float dir[3];
			GetAngleVectors(GetTurretAngles(sentry), dir, NULL_VECTOR, NULL_VECTOR);
			
			float goal[3]; goal = GetAbsOrigin(sentry);
			goal[0] -= (60.0 * dir[0]);
			goal[1] -= (60.0 * dir[1]);
			goal[2] -= (60.0 * dir[2]);
			
			PF_SetGoalVector(actor, goal);
			
			m_iRouteType[actor] = SAFEST_ROUTE;
			g_bPath[actor] = true;
		}
		
		if (dist < 75.0) 
		{
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

int CTFBotMvMEngineerIdle_SelectOutOfRangeNest(ArrayList nests)
{
	if (nests.Length <= 0) {
		return INVALID_ENT_REFERENCE;
	}
	
	for (int i = 0; i < nests.Length - 1; i++)
	{
		int nest = nests.Get(i);
		
		PrintToServer("CALLING IsStaleNest ON %i", nest);
		
		if (IsStaleNest(nest)) {
			return nest;
		}
	}
	
	return nests.Get(GetRandomInt(0, nests.Length - 1));
}