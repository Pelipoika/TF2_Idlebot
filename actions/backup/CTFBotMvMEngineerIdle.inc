
float m_ctSentrySafe[MAXPLAYERS + 1];               // +0x4814
float m_ctSentryCooldown[MAXPLAYERS + 1];           // +0x4820
//float m_ctTeleCooldown[MAXPLAYERS + 1];             // +0x482c

float m_ctFindNestHint[MAXPLAYERS + 1];             // +0x4838
float m_ctAdvanceNestSpot[MAXPLAYERS + 1];          // +0x4844

float m_ctRecomputePathMvMEngiIdle[MAXPLAYERS + 1];

int m_hHintTele[MAXPLAYERS + 1] = {INVALID_ENT_REFERENCE, ...};   					// +0x4858
int m_hHintSentry[MAXPLAYERS + 1] = {INVALID_ENT_REFERENCE, ...}; 					// +0x485c
int m_hHintNest[MAXPLAYERS + 1] = {INVALID_ENT_REFERENCE, ...};   					// +0x4860

bool m_bTriedToDetonateStaleNest[MAXPLAYERS + 1];   // +0x4864

enum HintType
{
	TELEPORTER_EXIT = 0,
	SENTRY_GUN      = 1,
	ENGINEER_NEST   = 2,
};

bool CTFBotMvMEngineerIdle_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer;
}

bool CTFBotMvMEngineerIdle_OnStart(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = false;
	
	m_ctFindNestHint[actor] = 0.0;
	m_ctRecomputePathMvMEngiIdle[actor] = 0.0;
	
	m_bTriedToDetonateStaleNest[actor] = false;
	
	return true;
}

bool CTFBotMvMEngineerIdle_Update(int actor)
{
	if(!IsPlayerAlive(actor))
	{
		ChangeAction(actor, ACTION_IDLE, "CTFBotMvMEngineerIdle: We are dead.");
		return false;
	}
	
	bool bShouldAdvance = CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor);
	if(bShouldAdvance)
	{
		FakeClientCommand(actor, "destroy 2");
	
		m_hHintNest[actor] = INVALID_ENT_REFERENCE;
		m_hHintSentry[actor] = INVALID_ENT_REFERENCE;
		
		//RIGHT NOW
		m_ctFindNestHint[actor] = GetGameTime();
	}
	
	if (EntRefToEntIndex(m_hHintNest[actor]) == INVALID_ENT_REFERENCE || bShouldAdvance) 
	{
		//HasStarted && !IsElapsed
		if (m_ctFindNestHint[actor] > 0.0 && m_ctFindNestHint[actor] > GetGameTime()) 
		{
			return false;
		}
		
		//Start
		m_ctFindNestHint[actor] = GetGameTime() + (GetRandomFloat(1.0, 2.0));
		
		//DO NOT INITIALIZE THIS VALUE OR YOU WILL NECK.
		int h_nest;
		
		if (!CTFBotMvMEngineerIdle_FindHint(false, h_nest)) 
			return false;
		
		if(h_nest <= 0)
			return false;
		
		m_hHintNest[actor] = EntIndexToEntRef(h_nest);
		PrintToServer("m_hHintNest %i", h_nest);
		
		m_hHintSentry[actor] = EntIndexToEntRef(GetSentryHint(EntRefToEntIndex(m_hHintNest[actor])));
		PrintToServer("m_hHintSentry %i", EntRefToEntIndex(m_hHintSentry[actor]));
	}
	
	int sentry = TF2_GetObject(actor, TFObject_Sentry);
	
	if (EntRefToEntIndex(m_hHintSentry[actor]) != INVALID_ENT_REFERENCE) 
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
				CTFBotMvMEngineerBuildSentryGun_OnStart(actor, EntRefToEntIndex(m_hHintSentry[actor]));
				
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
			goal[0] -= (50.0 * dir[0]);
			goal[1] -= (50.0 * dir[1]);
			goal[2] -= (50.0 * dir[2]);
			
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

void CTFBotMvMEngineerIdle_OnEnd(int actor)
{
	//Something here so we don't get a compile warning
	m_bTriedToDetonateStaleNest[actor] = false;
}

stock bool CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(int actor)
{
	if (EntRefToEntIndex(m_hHintNest[actor]) == INVALID_ENT_REFERENCE) 
		return false;
	
	if (m_ctAdvanceNestSpot[actor] <= 0.0)
	{
		m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
		return false;
	}
	
	int obj = -1;
	while ((obj = FindEntityByClassname(obj, "obj_*")) != -1)
	{
		if(GetEntPropEnt(obj, Prop_Send, "m_hBuilder") == actor)
		{
			if (GetEntProp(obj, Prop_Send, "m_iHealth") < GetEntProp(obj, Prop_Send, "m_iMaxHealth"))
			{
				m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
				return false;
			}
		}
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
	
	NavArea hint_area = TheNavMesh.GetNearestNavArea_Vec(GetAbsOrigin(EntRefToEntIndex(m_hHintNest[actor])), false, 1000.0);
	if (hint_area == NavArea_Null)
	{
		delete bombinfo;
		return false;
	}
	
	float m_flBombTargetDistance = view_as<float>(LoadFromAddress(view_as<Address>(hint_area) + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
	
	bool bigger = (m_flBombTargetDistance > bombinfo.GetFloat("hatch_dist_back"));

	//PrintToServer("m_flBombTargetDistance %f > bombinfo.hatch_dist_back %f = %s", m_flBombTargetDistance, bombinfo.GetFloat("hatch_dist_back"), bigger ? "Yes" : "No");

	delete bombinfo;
	return bigger;
}


stock bool CTFBotMvMEngineerIdle_FindHint(bool out_of_range_ok, int &the_hint)
{
	ArrayList hints = new ArrayList();
	
	int iEnt = -1;
	while((iEnt = FindEntityByClassname(iEnt, "bot_hint_engineer_nest")) != -1)
	{
		if (/*!GetEntProp(iEnt, Prop_Data, "m_isDisabled") && */GetOwnerEntity(iEnt) == INVALID_ENT_REFERENCE) 
		{
			hints.Push(iEnt);
		}
	}
	
	JSONObject bomb_info = new JSONObject();
	GetBombInfo(bomb_info);
	
	ArrayList hints1 = new ArrayList(); // in fwd~back, stale
	ArrayList hints2 = new ArrayList(); // in fwd~back, within min bomb distance
	ArrayList hints3 = new ArrayList(); // in fwd~infinity
	ArrayList hints4 = new ArrayList(); // others
	
	for (int i = 0; i < hints.Length - 1; i++)
	{
		int hint = hints.Get(i);
		
		NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetAbsOrigin(hint));
		if (area == NavArea_Null) 
		{
			PrintToServer("Sentry hint has NULL nav area!\n");
			continue;
		}
		
		//m_flBombTargetDistance
		float dist = view_as<float>(LoadFromAddress(view_as<Address>(area) + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
		
		if (dist > bomb_info.GetFloat("hatch_dist_fwd") 
		 && dist < bomb_info.GetFloat("hatch_dist_back")) 
		{
			if (IsStaleNest(hint))
			{
				hints1.Push(hint);
			} 
			else
			{
				float closest_pos[3];
				closest_pos[0] = bomb_info.GetFloat("closest_pos_x");
				closest_pos[1] = bomb_info.GetFloat("closest_pos_y");
				closest_pos[2] = bomb_info.GetFloat("closest_pos_z");
			
				hints2.Push(hint);
			}
		} 
		else if (dist > bomb_info.GetFloat("hatch_dist_back")) 
		{
			hints3.Push(hint);
		}
		else 
		{
			hints4.Push(hint);
		}
	}
	
	int hint;
	if (hints1.Length > 0) 
	{
		hint = hints1.Get(GetRandomInt(0, hints1.Length - 1));
	}
	else if (hints2.Length > 0) 
	{
		hint = hints2.Get(GetRandomInt(0, hints2.Length - 1));
	}
	else if (out_of_range_ok) 
	{
		hint = CTFBotMvMEngineerIdle_SelectOutOfRangeNest(hints3);
		if (hint == INVALID_ENT_REFERENCE) 
		{
			hint = CTFBotMvMEngineerIdle_SelectOutOfRangeNest(hints4);
		}
	}
	
	if (hint != INVALID_ENT_REFERENCE) 
	{
		the_hint = hint;
	}
	
	//Anti leak
	delete bomb_info;
	delete hints;
	delete hints1;
	delete hints2;
	delete hints3;
	delete hints4;
	
	return (hint != INVALID_ENT_REFERENCE);
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

stock HintType GetHintType(int hint)
{
	HintType type = TELEPORTER_EXIT;

	switch(GetClassName(hint))
	{
		case "bot_hint_sentrygun":       return SENTRY_GUN;
		case "bot_hint_teleporter_exit": return TELEPORTER_EXIT;
		case "bot_hint_engineer_nest":   return ENGINEER_NEST;
	}
	
	return type;
}