enum State
{
	FIRE, // wait for the weapon switch delay, then use it
	WAIT, // wait for the item to take effect, then finish
};

enum TauntState
{
	BEFORE, // taunt has not yet begun
	DURING, // taunt is in progress
	AFTER,  // taunt has completed
};

int m_hItem[MAXPLAYERS + 1] =  { -1, ... };

TauntState m_TauntState[MAXPLAYERS + 1];
State m_State[MAXPLAYERS + 1];

float m_flSwitchTime[MAXPLAYERS + 1];
float m_ctTimeout[MAXPLAYERS + 1];

bool CTFBotUseItem_OnStart(int actor)
{
	PrintToServer("[%8.3f] CTFBotUseItem(#%d): OnStart", GetGameTime(), actor, actor);

	m_TauntState[actor] = BEFORE;
	m_State[actor] = FIRE;
	
	int item = GetPlayerWeaponSlot(actor, TFWeaponSlot_Secondary);
	
	m_hItem[actor] = item;
	
	m_flSwitchTime[actor] = GetEntPropFloat(item, Prop_Send, "m_flNextPrimaryAttack");
	
	EquipWeaponSlot(actor, TFWeaponSlot_Secondary);
	
	m_ctTimeout[actor] = GetGameTime() + 10.0;
	
	return true;
}

stock bool CTFBotUseItem_Update(int actor)
{
	if (m_ctTimeout[actor] < GetGameTime())
	{
		PrintToServer("[%8.3f] CTFBotUseItem(#%d): Timeout elapsed!", GetGameTime(), actor, actor);
		ChangeAction(actor, ACTION_IDLE);
		return false;
	}
	
	EquipWeaponSlot(actor, TFWeaponSlot_Secondary);
	
	switch(m_State[actor])
	{
		case FIRE:
		{
			if (GetGameTime() >= m_flSwitchTime[actor])
			{
				PrintToServer("[%8.3f] CTFBotUseItem(#%d): Using item now", GetGameTime(), actor, actor);
				
				BotAim(actor).PressFireButton(1.0);
				m_State[actor] = WAIT;
			}
		}
		case WAIT:
		{
			if (CTFBotUseItem_IsDone(actor)) 
			{
				PrintToServer("[%8.3f] CTFBotUseItem(#%d): Done using item", GetGameTime(), actor, actor);
				ChangeAction(actor, ACTION_IDLE);
				return false;
			}
		}
	}
	
	return true;
}

void CTFBotUseItem_OnEnd(int actor)
{
	PrintToServer("[%8.3f] CTFBotUseItem(#%d): OnEnd", GetGameTime(), actor, actor);
	BotAim(actor).ReleaseFireButton();
}

bool CTFBotUseItem_IsPossible(int actor)
{
	/* the bot cannot actually PressFireButton in these cases */
	if (TF2_IsPlayerInCondition(actor, TFCond_Dazed))
		return false;
		
	float health_ratio = view_as<float>(GetClientHealth(actor)) / view_as<float>(GetMaxHealth(actor));
	//Can't eat while slowed.
	if(TF2_GetPlayerClass(actor) == TFClass_Heavy && health_ratio > FindConVar("tf_bot_health_critical_ratio").FloatValue || TF2_IsPlayerInCondition(actor, TFCond_Slowed))
		return false;
	
	return true;
}

bool CTFBotUseItem_IsDone(int actor)
{
	if(!IsValidEntity(m_hItem[actor]))
		return true;

	if(HasEntProp(m_hItem[actor], Prop_Send, "m_flRageMeter"))
		return GetEntPropFloat(m_hItem[actor], Prop_Send, "m_flRageMeter") <= 0.0;
		
	if(HasEntProp(m_hItem[actor], Prop_Send, "m_flEnergyDrinkMeter"))
		return GetEntPropFloat(m_hItem[actor], Prop_Send, "m_flEnergyDrinkMeter") <= 0.0;
	
	if(StrEqual(EntityNetClass(m_hItem[actor]), "CTFBuffItem") && HasEntProp(actor, Prop_Send, "m_bRageDraining"))
		return !!GetEntProp(actor, Prop_Send, "m_bRageDraining");
	
	return CTFBotUseLunchBoxItem_IsDone(actor);
}

bool CTFBotUseLunchBoxItem_IsDone(int actor)
{
	switch (m_TauntState[actor]) 
	{
		case BEFORE:
		{
			if (TF2_IsPlayerInCondition(actor, TFCond_Taunting))
			{
				m_TauntState[actor] = DURING;
			}
			
			return false;
		}
		case DURING:
		{
			if (!TF2_IsPlayerInCondition(actor, TFCond_Taunting)) 
			{
				m_TauntState[actor] = AFTER;
			}
			
			return false;
		}
		case AFTER:
		{
			return true;
		}
	}
	
	// should never happen
	return true;
}

