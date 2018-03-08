int m_hCurrencyPack[MAXPLAYERS + 1];

float GetTimeUntilRemoved(int powerup)
{
	return (GetNextThink(powerup, "PowerupRemoveThink") - GetGameTime());
}

// TODO: prioritize currency packs based on these criteria:
// - age (0~30 seconds)
// - type (green vs red)
// - credit value
// - distance

bool CTFBotCollectMoney_OnStart(int actor)
{
	CTFBotCollectMoney_SelectCurrencyPack(actor);
	
	BotAim(actor).ReleaseFireButton();

	g_bPath[actor] = true;

	return true;
}

bool CTFBotCollectMoney_Update(int actor)
{
	if (!IsValidCurrencyPack(m_hCurrencyPack[actor])) 
	{
		return false;
	}
	
	PF_SetGoalVector(actor, WorldSpaceCenter(m_hCurrencyPack[actor]));

	return true;
}

void CTFBotCollectMoney_OnEnd(int actor)
{
	m_hCurrencyPack[actor] = -1;
}

bool CTFBotCollectMoney_IsPossible(int actor)
{
	if (!TF2_IsMvM()) 
		return false;
		
//	if (TF2_GetPlayerClass(actor) != TFClass_Scout)
//		return false;
	
	if (!IsValidCurrencyPack(CTFBotCollectMoney_SelectCurrencyPack(actor)))
		return false;
	
	return true;
}

int CTFBotCollectMoney_SelectCurrencyPack(int actor)
{
	int iBestPack = INVALID_ENT_REFERENCE;
	float flLowestTime = 30.0;
	
	int x = INVALID_ENT_REFERENCE; 
	while ((x = FindEntityByClassname(x, "item_currency*")) != -1)
	{
		bool bDistributed = !!GetEntProp(x, Prop_Send, "m_bDistributed");
		if(bDistributed)
			continue;
		
		if(!(GetEntityFlags(x) & FL_ONGROUND))
			continue;
		
		float flTimeUntilRemoved = GetTimeUntilRemoved(x);
		
		if(flLowestTime > flTimeUntilRemoved)
		{
			flLowestTime = flTimeUntilRemoved;
			iBestPack = x;
		}
	}

	m_hCurrencyPack[actor] = iBestPack;
	return iBestPack;
}

bool IsValidCurrencyPack(int pack)
{
	if(!IsValidEntity(pack))
		return false;

	char class[64]; 
	GetEntityClassname(pack, class, sizeof(class));
	
	if(StrContains(class, "item_currency", false) == -1)
		return false;
	
	return true;
}