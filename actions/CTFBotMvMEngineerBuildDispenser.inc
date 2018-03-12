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
		ChangeAction(actor, ACTION_IDLE, "No hint entity");
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
		
		g_iAdditionalButtons[actor] |= IN_DUCK;
		
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