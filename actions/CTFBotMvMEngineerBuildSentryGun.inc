//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.cpp
//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/engineer/mvm_engineer/tf_bot_mvm_engineer_build_sentry.h

int m_hintEntity[MAXPLAYERS + 1]; // +0x0034
int m_hSentry[MAXPLAYERS + 1];    // +0x0038

bool CTFBotMvMEngineerBuildSentryGun_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer && GetAmmoCount(actor, TF_AMMO_METAL) >= 130;
}

bool CTFBotMvMEngineerBuildSentryGun_OnStart(int actor, int hint = INVALID_ENT_REFERENCE)
{
	g_bUpdateLookingAroundForEnemies[actor] = true;
	m_hintEntity[actor] = hint;
	
	//Start building a sentry
	FakeClientCommand(actor, "destroy 2");
	FakeClientCommand(actor, "build 2");
	
	return true;
}

bool CTFBotMvMEngineerBuildSentryGun_Update(int actor)
{
	if (m_hintEntity[actor] == INVALID_ENT_REFERENCE) 
	{
		ChangeAction(actor, ACTION_IDLE, "No hint entity");
		return false;
	}
	
	//Start building a sentry
	if(!IsWeapon(actor, TF_WEAPON_BUILDER))
		FakeClientCommand(actor, "build 2");
	
	float range_to_hint = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(m_hintEntity[actor]));
	
	if (range_to_hint < 200.0) 
	{
		g_bUpdateLookingAroundForEnemies[actor] = false;
		
		g_iAdditionalButtons[actor] |= IN_DUCK;
		BotAim(actor).AimHeadTowards(GetAbsOrigin(m_hintEntity[actor]), OVERRIDE_ALL, 0.1, "Placing sentry");
	}
	
	PF_SetGoalVector(actor, GetAbsOrigin(m_hintEntity[actor]));
	g_bPath[actor] = true;
	
	if(range_to_hint > 70.0)
		return false;
	
	g_bUpdateLookingAroundForEnemies[actor] = false;
	g_bPath[actor] = false;
	
	//actor->DetonateObjectOfType(OBJ_SENTRYGUN, 0, true);
	FakeClientCommand(actor, "destroy 2");
	
	BotAim(actor).PressFireButton();
	
	m_hSentry[actor] = TF2_GetObject(actor, TFObject_Sentry);
	
	//int ent = SpawnSentry(actor, GetAbsOrigin(m_hintEntity[actor]), GetAbsAngles(m_hintEntity[actor]));
	if(m_hSentry[actor] == INVALID_ENT_REFERENCE)
		return false;
	
	DispatchKeyValueVector(m_hSentry[actor], "origin", GetAbsOrigin(m_hintEntity[actor]));
	DispatchKeyValueVector(m_hSentry[actor], "angles", GetAbsAngles(m_hintEntity[actor]));
	
	char name[64];
	GetEntPropString(m_hintEntity[actor], Prop_Data, "m_iName", name, sizeof(name));
	DispatchKeyValue(m_hSentry[actor], "targetname", name);
	
	SetOwnerEntity(m_hintEntity[actor], m_hSentry[actor]);
	
	m_hSentry[actor] = INVALID_ENT_REFERENCE;
	
	ChangeAction(actor, ACTION_MVM_ENGINEER_IDLE, "Built a sentry");
	FakeClientCommand(actor, "tournament_player_readystate 1");
	
	//PrintToServer("CTFBotMvMEngineerBuildSentryGun m_hHintTele %i m_hHintSentry %i m_hHintNest %i", m_hHintTele[actor], m_hHintSentry[actor], m_hHintNest[actor]);
	
	return true;
}

stock int TF2_GetObject(int client, TFObjectType type)
{
	int iObject = INVALID_ENT_REFERENCE;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		if(GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type && !GetEntProp(iObject, Prop_Send, "m_bPlacing"))
		{
			return iObject;
		}
	}
	
	return iObject;
}

void CTFBotMvMEngineerBuildSentryGun_OnEnd(int actor)
{
	//Something here so we don't get a compile warning
	if (m_hSentry[actor] != INVALID_ENT_REFERENCE) 
	{
		//m_hSentry[actor]->DropCarriedObject(actor);
		//UTIL_Remove(this->m_hSentry);
		//AcceptEntityInput(m_hSentry[actor], "Kill");
	}
	m_hSentry[actor] = INVALID_ENT_REFERENCE;
}