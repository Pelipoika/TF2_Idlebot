// If threat is farther away than this, bot will switch back to its primary weapon and attack
const float tf_bot_melee_attack_abandon_range = 500.0;
	
float m_flAbandonRange[MAXPLAYERS + 1];

bool CTFBotMeleeAttack_IsPossible(int actor)
{
	return CTFBotMeleeAttack_SelectRandomReachableEnemy(actor) != -1;
}

bool CTFBotMeleeAttack_OnStart(int actor, float abandon_range = -1.0)
{
	if (abandon_range < 0.0) {
		m_flAbandonRange[actor] = tf_bot_melee_attack_abandon_range;
	} else {
		m_flAbandonRange[actor] = abandon_range;
	}
	
	return true;
}

bool CTFBotMeleeAttack_Update(int actor)
{
	int bot_nearest_to_me = TF2_FindBotNearestToMe(actor, m_flAbandonRange[actor]);

	if (bot_nearest_to_me <= 0)
	{
		ChangeAction(actor, ACTION_SNIPER_LURK, "No threat");	
		m_iRouteType[actor] = SAFEST_ROUTE;
		return false;
	}
	
	if(GetVectorDistance(GetAbsOrigin(bot_nearest_to_me), GetAbsOrigin(actor)) > m_flAbandonRange[actor])
	{
		ChangeAction(actor, ACTION_SNIPER_LURK, "Threat is too far away for a melee attack");
		m_iRouteType[actor] = SAFEST_ROUTE;
		return false;
	}
	
	EquipWeaponSlot(actor, TFWeaponSlot_Melee);
	//PrintToServer("EquipWeaponSlot MELEE");
	
	BotAim(actor).PressFireButton();
	
	PF_SetGoalEntity(actor, bot_nearest_to_me);
	
	return true;
}

void CTFBotMeleeAttack_OnEnd(int actor)
{
	//Something here so we don't get a compile warning
	m_flAbandonRange[actor] = 1.0;
}