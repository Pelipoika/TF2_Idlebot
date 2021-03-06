int m_hAmmo[MAXPLAYERS + 1];

bool CTFBotGetAmmo_IsPossible(int actor)
{
	//Skip lag.
	if(m_hAmmo[actor] != -1 && IsValidAmmo(m_hAmmo[actor]))
	{
		return true;
	}

	JSONArray ammo = new JSONArray();
	ComputeHealthAndAmmoVectors(actor, ammo, FindConVar("tf_bot_ammo_search_range").FloatValue);
	
	if(ammo.Length <= 0)
	{
		delete ammo;
		return false;
	}

	bool bPossible = false;
	
	for (int i = 0; i < ammo.Length; i++)
	{
		JSONObject entity = view_as<JSONObject>(ammo.Get(i));
		
		if(!IsValidAmmo(entity.GetInt("entity_index")))
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

bool CTFBotGetAmmo_OnStart(int actor)
{
	JSONArray ammo = new JSONArray();
	ComputeHealthAndAmmoVectors(actor, ammo, FindConVar("tf_bot_ammo_search_range").FloatValue);
	
	if(ammo.Length <= 0)
	{
		delete ammo;
		return false;
	}
	
	float flSmallestDistance = 99999.0;
	
	for (int i = 0; i < ammo.Length; i++)
	{
		JSONObject entity = view_as<JSONObject>(ammo.Get(i));
		
		if(!IsValidAmmo(entity.GetInt("entity_index")))
		{
			delete entity;
			continue;
		}
		
		float flDistance = entity.GetFloat("path_length")
		if (flDistance <= flSmallestDistance)
		{
			m_hAmmo[actor] = entity.GetInt("entity_index");
			flSmallestDistance = flDistance;
		}
		
		delete entity;
	}
	
	delete ammo;
	
	if(m_hAmmo[actor] != -1)
	{
		g_bPath[actor] = true;
		g_bUpdateLookingAroundForEnemies[actor] = true;
		return true;
	}

	g_bUpdateLookingAroundForEnemies[actor] = true;
	return false;
}

bool CTFBotGetAmmo_Update(int actor)
{
	if(!IsValidAmmo(m_hAmmo[actor]))
	{
		ChangeAction(actor, ACTION_IDLE, "CTFBotGetAmmo_Update ammo is not valid");
		StartMainAction(actor, true);
		return false;
	}
	
	g_bPath[actor] = true;
	PF_SetGoalVector(actor, WorldSpaceCenter(m_hAmmo[actor]));
	
	return true;
}

bool CTFBotGetAmmo_OnEnd(int actor)
{
	m_hAmmo[actor] = -1;
}

bool IsValidAmmo(int pack)
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
	
	if(StrContains(class, "tf_ammo_pack", false) == -1 
	&& StrContains(class, "item_ammo", false) == -1 
	&& StrContains(class, "obj_dispenser", false) == -1
	&& StrContains(class, "func_regen", false) == -1)
	{
		return false;
	}
	
	return true;
}