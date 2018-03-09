//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.cpp
//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.h

bool CTFBotMvMEngineerBuildSentryGun_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer && GetAmmoCount(actor, TF_AMMO_METAL) >= 130;
}

bool CTFBotMvMEngineerBuildSentryGun_OnStart(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = true;
	
	return true;
}

bool CTFBotMvMEngineerBuildSentryGun_Update(int actor)
{
	if (m_aNestArea[actor] == NavArea_Null) 
	{
		ChangeAction(actor, ACTION_IDLE, "No hint entity");
		return false;
	}
	
	//Bail.
	if(CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor))
	{
		m_aNestArea[actor] = PickBuildArea(actor);
		FakeClientCommand(actor, "destroy 2");
		
		//ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "The spot we were planning to build at is now too far back.");
		return true;
	}
	
	float areaCenter[3];
	m_aNestArea[actor].GetCenter(areaCenter);
	
	float range_to_hint = GetVectorDistance(GetAbsOrigin(actor), areaCenter);
	
	if (range_to_hint < 200.0) 
	{
		//Start building a sentry
		if(!IsWeapon(actor, TF_WEAPON_BUILDER))
			FakeClientCommand(actor, "build 2");
		
		g_bUpdateLookingAroundForEnemies[actor] = false;
		
		g_iAdditionalButtons[actor] |= IN_DUCK;
		
		BotAim(actor).AimHeadTowards(areaCenter, OVERRIDE_ALL, 0.1, "Placing sentry");
	}
	
	if(range_to_hint > 70.0)
	{
		//PrintToServer("%f %f %f", areaCenter[0], areaCenter[1], areaCenter[2]);
	
		PF_SetGoalVector(actor, areaCenter);
		g_bPath[actor] = true;
	
		if(range_to_hint > 300.0)
		{
			//Fuck em up.
			EquipWeaponSlot(actor, TFWeaponSlot_Melee);
		}
	
		g_bUpdateLookingAroundForEnemies[actor] = true;
		
		return false;
	}
	
	g_bPath[actor] = false;
	
	if(IsWeapon(actor, TF_WEAPON_BUILDER))
	{
		int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(actor), Prop_Send, "m_hObjectBeingBuilt");
		
		bool m_bPlacementOK = !!GetEntData(objBeingBuilt, FindSendPropInfo("CObjectSentrygun", "m_iKills") - 4);
		
		BotAim(actor).PressFireButton();
		
		if(!m_bPlacementOK && BotAim(actor).IsHeadAimingOnTarget() && BotAim(actor).GetHeadSteadyDuration() > 0.6 )
		{
			//That spot was no good.
			//Time to pick a new spot.
			m_aNestArea[actor] = PickBuildArea(actor);
			
			return false;
		}
	}
	
	int sentry = TF2_GetObject(actor, TFObject_Sentry);
	if(sentry == INVALID_ENT_REFERENCE)
		return false;
	
	ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Built a sentry");
	FakeClientCommand(actor, "tournament_player_readystate 1");
		
	return true;
}

void CTFBotMvMEngineerBuildSentryGun_OnEnd(int actor)
{
	g_bUpdateLookingAroundForEnemies[actor] = true;
}