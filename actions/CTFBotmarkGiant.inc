int m_hTarget[MAXPLAYERS + 1];

int GetMarkForDeathWeapon(int player)
{
	for (int i = 0; i < 8; i++) 
	{
		int weapon = GetPlayerWeaponSlot(player, i);
		if(!IsValidEntity(weapon)) 
			continue;
		
		int m_iItemDefinitionIndex = GetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex")
		if(m_iItemDefinitionIndex == 355) //Fan O'War
		{
			return weapon;
		}
	}
	
	return INVALID_ENT_REFERENCE;
}

bool IsPlayerMarkable(int bot, int victim)
{
	/* must be ingame */
	if(!IsClientInGame(victim))
		return false;

	/* must be alive */
	if (!IsPlayerAlive(victim)) 
		return false;
	
	/* must be an enemy */
	if (GetTeamNumber(bot) == GetTeamNumber(victim)) 
		return false;
	
	/* must be a giant */
	if (!IsMiniBoss(victim)) 
		return false;
	
	/* must not be a sentry buster */
	if(TF2_IsBuster(victim))
		return false;
	
	/* must not already be marked for death */
	if (TF2_IsPlayerInCondition(victim, TFCond_MarkedForDeath)) 
		return false;
	
	/* must not be invulnerable */
	if (IsInvulnerable(victim)) 
		return false;
	
	/* must not be a sentry buster */
//	CTFBot *vbot = ToTFBot(victim);
//	if (vbot != nullptr && vbot->GetMission() == CTFBot::MISSION_DESTROY_SENTRIES) return false;
	
	return true;
}

bool CTFBotMarkGiant_OnStart(int actor)
{
	int weapon = GetMarkForDeathWeapon(actor);
	if (weapon == INVALID_ENT_REFERENCE) {
		PrintToServer("CTFBotMarkGiant_OnStart Don't have a mark-for-death weapon.");
		return false;
	}
	
	ArrayList potential_victims = new ArrayList();
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == actor)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		if (IsPlayerMarkable(actor, i)) {
			potential_victims.Push(i);
		}
	}
	
	if (potential_victims.Length == 0) {
		PrintToServer("CTFBotMarkGiant_OnStart No eligible mark victims.");
		delete potential_victims;
		m_hTarget[actor] = -1;
		return false;
	}
	
	m_hTarget[actor] = potential_victims.Get(GetRandomInt(0, potential_victims.Length - 1));
	
	delete potential_victims;
	
	EquipWeapon(actor, weapon);

	return true;
}

bool CTFBotMarkGiant_Update(int actor)
{
	if (!IsValidClientIndex(m_hTarget[actor]) || !IsPlayerAlive(m_hTarget[actor])) {
		PrintToServer("CTFBotMarkGiant_Update Mark target is no longer valid.");
		m_hTarget[actor] = -1;
		ChangeAction(actor, ACTION_ATTACK);
		return false;
	}
	
	if (!IsPlayerMarkable(actor, m_hTarget[actor])) {
		PrintToServer("CTFBotMarkGiant_Update Mark target is no longer markable.");
		m_hTarget[actor] = -1;
		ChangeAction(actor, ACTION_ATTACK);
		return false;
	}
	
	PF_SetGoalEntity(actor, m_hTarget[actor]);
	
	float dist_to_target = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(m_hTarget[actor]));
	if (dist_to_target < 512.0)
	{
		BotAim(actor).AimHeadTowardsEntity(m_hTarget[actor], OVERRIDE_ALL, 0.1, "Look at our mark-for-death target");
	}
	
	if (dist_to_target > GetDesiredAttackRange(actor)) {
		g_bPath[actor] = true;
	}else{
		g_bPath[actor] = false;
	}
	
	return true;
}

void CTFBotMarkGiant_OnEnd(int actor)
{
	m_hTarget[actor] = -1;
//	actor->PopRequiredWeapon();
}

bool CTFBotMarkGiant_IsPossible(int actor)
{
	if (GetMarkForDeathWeapon(actor) == INVALID_ENT_REFERENCE) 
		return false;
	
	bool victim_exists = false;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == actor)
			continue;

		if(!IsClientConnected(i))
			continue;
		
		if (IsPlayerMarkable(actor, i)) 
		{
			victim_exists = true;
		}
	}
	
	return victim_exists;
}