//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.cpp
//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.h

bool CTFBotMvMEngineerBuildDispenser_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer && GetAmmoCount(actor, TF_AMMO_METAL) >= 100;
}

bool CTFBotMvMEngineerBuildDispenser_OnStart(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = true;
	
	return true;
}

bool CTFBotMvMEngineerBuildDispenser_Update(int actor)
{
	if (m_aNestArea[actor] == NavArea_Null) 
	{
		ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "No hint entity");
		return false;
	}
	
	if(TF2_GetObject(actor, TFObject_Sentry) == INVALID_ENT_REFERENCE)
	{
		//Fuck you.
		
		ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "No sentry");
		return false;
	}
	else
	{
		//sentry is not safe.
		if(m_ctSentrySafe[actor] < GetGameTime())
		{
			ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Sentry not safe");
			return false;
		}
	}
	
	if(CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor))
	{
		//Fuck you too.
		
		ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Need to advance nest");
		return false;
	}
	
	float areaCenter[3];
	m_aNestArea[actor].GetRandomPoint(areaCenter);
	
	float range_to_hint = GetVectorDistance(GetAbsOrigin(actor), areaCenter);
	
	if (range_to_hint < 200.0) 
	{
		//Start building a dispenser
		if(!IsWeapon(actor, TF_WEAPON_BUILDER))
			FakeClientCommand(actor, "build 0");
		
		//Look in "random" directions in an attempt to find a place to fit a dispenser.
		g_bUpdateLookingAroundForEnemies[actor] = true;
		
	//	if(g_flNextLookTime[actor] > GetGameTime())
	//		return false;
		
	//	g_flNextLookTime[actor] = GetGameTime() + GetRandomFloat(0.3, 1.0);
		
		float flRetreatRange = 100.0;
		
		NavArea lastArea = PF_GetLastKnownArea(actor);
		if(lastArea == NavArea_Null)
			return false;	
		
		int iTeamNum = GetClientTeam(actor);
								
		int eax = (iTeamNum + iTeamNum * 4); //eax
		int edi = view_as<int>(lastArea) + eax * 4 + 364;  //edi
		
		eax = edi + 0xC; // +12
		
		int connections = LoadFromAddress(view_as<Address>(eax), NumberType_Int32);
		
		if(connections > 0)
		{
			float vecRandomPoint[3];
			NavArea navArea = NavArea_Null;
			
			for (int i = 0; i <= 20; i++)
			{
				int iRandomArea = (4 * GetRandomInt(0, connections - 1));
				
				Address areas = view_as<Address>(LoadFromAddress(view_as<Address>(eax + 4), NumberType_Int32));
				navArea = view_as<NavArea>(LoadFromAddress(areas + view_as<Address>(iRandomArea), NumberType_Int32));
				
				if(navArea == NavArea_Null)
					continue;
				
				navArea.GetRandomPoint(vecRandomPoint);
				vecRandomPoint[2] += 18.0;
				
				float to[3]; SubtractVectors(GetAbsOrigin(actor), vecRandomPoint, to);
				if(GetVectorLength(to, true) > flRetreatRange * flRetreatRange)
				{
					if (IsLineOfFireClear(GetEyePosition(actor), vecRandomPoint))
					{
						break;
					}
				}
			}
			
			BotAim(actor).AimHeadTowards(vecRandomPoint, OVERRIDE_ALL, 1.0, "Looking toward team invasion areas in an attempt to find a spot for dispenser");
		}
		
		if(!IsStuck(actor))
		{
			g_iAdditionalButtons[actor] |= IN_DUCK;
		}
		
		
		//BotAim(actor).AimHeadTowards(areaCenter, OVERRIDE_ALL, 0.1, "Placing sentry");
	}
	
	if(range_to_hint > 70.0)
	{
		//PrintToServer("%f %f %f", areaCenter[0], areaCenter[1], areaCenter[2]);
	
		PF_SetGoalVector(actor, areaCenter);
		g_bPath[actor] = true;
	
		//if(range_to_hint > 300.0)
		//{
			//Fuck em up.
			//EquipWeaponSlot(actor, TFWeaponSlot_Melee);
		//}
		
		return false;
	}
	
	g_bPath[actor] = false;
	
	BotAim(actor).PressFireButton();
	
	int sentry = TF2_GetObject(actor, TFObject_Dispenser);
	if(sentry == INVALID_ENT_REFERENCE)
		return false;
	
	//DispatchKeyValueVector(sentry, "origin", areaCenter);
	//DispatchKeyValueVector(sentry, "angles", GetAbsAngles(EntRefToEntIndex(m_hintEntity[actor])));
	
	ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Built a dispenser");
	FakeClientCommand(actor, "tournament_player_readystate 1");
	
	return true;
}

void CTFBotMvMEngineerBuildDispenser_OnEnd(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = true;
}