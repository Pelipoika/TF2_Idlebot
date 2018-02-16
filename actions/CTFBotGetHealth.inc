int m_hHealth[MAXPLAYERS + 1];

bool CTFBotGetHealth_IsPossible(int actor)
{
//	if (GetEntProp(actor, Prop_Send, "m_nNumHealers") > 0)
//		return false;
	
	float health_ratio = view_as<float>(GetClientHealth(actor)) / view_as<float>(GetMaxHealth(actor));
	float ratio = clamp((health_ratio - FindConVar("tf_bot_health_critical_ratio").FloatValue) / (FindConVar("tf_bot_health_ok_ratio").FloatValue - FindConVar("tf_bot_health_critical_ratio").FloatValue), 0.0, 1.0);
	
//	if (TF2_IsPlayerInCondition(actor, TFCond_OnFire))
//		ratio = 0.0;
	
	float far_range = FindConVar("tf_bot_health_search_far_range").FloatValue;
	float max_range = ratio * (FindConVar("tf_bot_health_search_near_range").FloatValue - far_range);
	max_range += far_range;
	
	//Skip lag.
	if(m_hHealth[actor] != -1 && IsValidHealth(m_hHealth[actor]))
		return true;

//	PrintToServer("ratio %f max_range %f", ratio, max_range);

	JSONArray ammo = new JSONArray();
	ComputeHealthAndAmmoVectors(actor, ammo, max_range);
	
	if(ammo.Length <= 0)
	{
		delete ammo;
		return false;
	}

	bool bPossible = false;
	
	for (int i = 0; i < ammo.Length; i++)
	{
		JSONObject entity = view_as<JSONObject>(ammo.Get(i));
		
		if(!IsValidHealth(entity.GetInt("entity_index")))
		{
			delete entity;
			continue;
		}
		
		bPossible = true;
		delete entity;
		break;
	}
	
	delete ammo;
	return bPossible;
}

bool CTFBotGetHealth_OnStart(int actor)
{
	float health_ratio = view_as<float>(GetClientHealth(actor)) / view_as<float>(GetMaxHealth(actor));
	float ratio = clamp((health_ratio - FindConVar("tf_bot_health_critical_ratio").FloatValue) / (FindConVar("tf_bot_health_ok_ratio").FloatValue - FindConVar("tf_bot_health_critical_ratio").FloatValue), 0.0, 1.0);
	
//	if (TF2_IsPlayerInCondition(actor, TFCond_OnFire))
//		ratio = 0.0;
	
	//((100 / 175) - 0.8) / (0.3 - 0.8)
	
	float far_range = FindConVar("tf_bot_health_search_far_range").FloatValue;
	float max_range = ratio * (FindConVar("tf_bot_health_search_near_range").FloatValue - far_range);
	max_range += far_range;
	
	JSONArray ammo = new JSONArray();
	ComputeHealthAndAmmoVectors(actor, ammo, max_range);
	
	if(ammo.Length <= 0)
	{
		delete ammo;
		return false;
	}
	
	float flSmallestDistance = 99999.0;
	
	for (int i = 0; i < ammo.Length; i++)
	{
		JSONObject entity = view_as<JSONObject>(ammo.Get(i));
		
		if(!IsValidHealth(entity.GetInt("entity_index")))
		{
			delete entity;
			continue;
		}
		
		float flDistance = entity.GetFloat("path_length");
		if (flDistance <= flSmallestDistance)
		{
			m_hHealth[actor] = entity.GetInt("entity_index");
			flSmallestDistance = flDistance;
		}
		
		delete entity;
	}
	
	delete ammo;
	
	if(m_hHealth[actor] != -1)
	{
		return true;
	}

	return false;
}

bool CTFBotGetHealth_Update(int actor)
{
	if(!IsValidHealth(m_hHealth[actor]))
	{
		ChangeAction(actor, ACTION_IDLE, "CTFBotGetHealth_Update health is not valid");

		return false;
	}
	
	PF_SetGoalVector(actor, WorldSpaceCenter(m_hHealth[actor]));
	
	return true;
}

bool CTFBotGetHealth_OnEnd(int actor)
{
	m_hHealth[actor] = -1;
}

bool IsValidHealth(int pack)
{
	if(!IsValidEntity(pack))
		return false;

	if(!HasEntProp(pack, Prop_Send, "m_fEffects"))
		return false;

	//It has been taken.
	if (GetEntProp(pack, Prop_Send, "m_fEffects") != 0)
		return false;

	char class[64]; 
	GetEntityClassname(pack, class, sizeof(class));
	
	if(StrContains(class, "item_health", false) == -1 
	&& StrContains(class, "obj_dispenser", false) == -1
	&& StrContains(class, "func_regen", false) == -1)
	{
		return false;
	}
	
	return true;
}
