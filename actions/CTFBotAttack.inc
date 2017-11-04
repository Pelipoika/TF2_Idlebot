int m_hAttackTarget[MAXPLAYERS + 1];

float g_flRevalidateTarget[MAXPLAYERS + 1];

bool CTFBotAttack_IsPossible(int actor)
{
	return CTFBotAttack_SelectRandomReachableEnemy(actor) != -1;
}

bool CTFBotAttack_OnStart(int actor)
{
	m_hAttackTarget[actor] = CTFBotAttack_SelectRandomReachableEnemy(actor);
	
	if(m_hAttackTarget[actor] == -1)
		return false;
	
	EquipWeaponSlot(actor, TFWeaponSlot_Primary);
	
	g_flRevalidateTarget[actor] = GetGameTime() + 3.0;
	
	return true;
}

bool CTFBotAttack_Update(int actor)
{
	if (!IsValidClientIndex(m_hAttackTarget[actor]) || !IsPlayerAlive(m_hAttackTarget[actor])
	|| TF2_GetClientTeam(m_hAttackTarget[actor]) != GetEnemyTeam(actor))
	{
	//	PrintToServer("Curren target is no longer valid.");
		m_hAttackTarget[actor] = -1;
		CTFBotAttack_OnStart(actor);
		g_bPath[actor] = false;
		return false;
	}
	
	if(g_flRevalidateTarget[actor] <= GetGameTime())
	{
		g_flRevalidateTarget[actor] = GetGameTime() + 2.0;
	
		//Need new target.
		if(!PF_IsPathToEntityPossible(actor, m_hAttackTarget[actor], NULL_FLOAT))
		{
			m_hAttackTarget[actor] = -1;
			CTFBotAttack_OnStart(actor);
			g_bPath[actor] = false;
					
			return false;
		}	
	}
	
	g_bPath[actor] = true;
	
	int closesttoh = TF2_FindBotNearestToBombNearestToHatch(actor);
	if(closesttoh != -1)
	{
		m_hAttackTarget[actor] = closesttoh;
		PF_SetGoalVector(actor, PredictSubjectPosition(actor, m_hAttackTarget[actor]));
	}
	else
	{
		PF_SetGoalVector(actor, PredictSubjectPosition(actor, m_hAttackTarget[actor]));
	}
	
	int iTarget = -1;
	
	//Don't constantly switch target.
	if (TF2_GetPlayerClass(actor) == TFClass_Sniper && m_hAimTarget[actor] != -1 && IsLineOfFireClear(GetEyePosition(actor), GetEyePosition(m_hAttackTarget[actor])) && !IsInvulnerable(m_hAttackTarget[actor])) 
		iTarget = m_hAimTarget[actor];
	else
		iTarget = Entity_GetClosestClient(actor);
	
	if(iTarget != -1)
	{	
		BotAim(actor).AimHeadTowardsEntity(iTarget, CRITICAL, 0.3, "Looking at visible threat");
		
		float dist_to_target = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(m_hAttackTarget[actor]));
		
		bool bLOS = IsLineOfFireClear(GetEyePosition(actor), GetEyePosition(m_hAttackTarget[actor]));
		if(!bLOS)
		{
			g_bPath[actor] = true;
		}
		else
		{
			if(TF2_GetPlayerClass(actor) != TFClass_Sniper)
			{
				if((IsMeleeWeapon(actor) || IsWeapon(actor, TF_WEAPON_FLAMETHROWER)) && dist_to_target > 100.0)
					g_bPath[actor] = true;
				else if(IsCombatWeapon(actor) && dist_to_target > 500.0)
					g_bPath[actor] = true;
				else
					g_bPath[actor] = false;
			}
			else
				g_bPath[actor] = false;
		}
	}
	else
	{
		m_hAimTarget[actor] = -1;
	}
	
	return true;
}

void CTFBotAttack_OnEnd(int actor)
{
	m_hAttackTarget[actor] = -1;
}

int CTFBotAttack_SelectRandomReachableEnemy(int actor)
{
	TFTeam opposingTFTeam = GetEnemyTeam(actor);
	
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
		
		//Ignore non enemies
		if(TF2_GetClientTeam(i) != opposingTFTeam)
			continue;
		
		//TODO: replace with PointInRespawnRoom
		if(IsInvulnerable(i))
			continue;
		
		//Always ignore Sentry Busters
		if(TF2_IsBuster(i))
			continue;
		
		//Where are you
		NavArea lastKnownArea = PF_GetLastKnownArea(i);
		if(lastKnownArea == NavArea_Null)
			continue;
		
		//Cannot path into enemy spawn rooms
		if ((opposingTFTeam == TFTeam_Red  && HasTFAttributes(lastKnownArea, RED_SPAWN_ROOM)) ||
			(opposingTFTeam == TFTeam_Blue && HasTFAttributes(lastKnownArea, BLUE_SPAWN_ROOM)))
			continue;
		
		//Ignore unpathable 
		if (!PF_IsPathToEntityPossible(actor, i, NULL_FLOAT))
			continue;
			
		playerarray[playercount] = i;
		playercount++;
	}
	
	if(playercount)
	{
		return playerarray[GetRandomInt(0, playercount-1)];
	}
	
	return -1;
}