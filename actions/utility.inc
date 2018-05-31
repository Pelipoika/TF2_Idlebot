enum
{
	TF_AMMO_DUMMY = 0,
	TF_AMMO_PRIMARY,
	TF_AMMO_SECONDARY,
	TF_AMMO_METAL,
	TF_AMMO_GRENADES1,
	TF_AMMO_GRENADES2,
	TF_AMMO_COUNT,
};

enum
{
	TF_LOADOUT_SLOT_PRIMARY   =  0,
	TF_LOADOUT_SLOT_SECONDARY =  1,
	TF_LOADOUT_SLOT_MELEE     =  2,
	TF_LOADOUT_SLOT_UTILITY   =  3,
	TF_LOADOUT_SLOT_BUILDING  =  4,
	TF_LOADOUT_SLOT_PDA       =  5,
	TF_LOADOUT_SLOT_PDA2      =  6,
	TF_LOADOUT_SLOT_HEAD      =  7,
	TF_LOADOUT_SLOT_MISC      =  8,
	TF_LOADOUT_SLOT_ACTION    =  9,
	TF_LOADOUT_SLOT_MISC2     = 10,
	TF_LOADOUT_SLOT_TAUNT     = 11,
	TF_LOADOUT_SLOT_TAUNT2    = 12,
	TF_LOADOUT_SLOT_TAUNT3    = 13,
	TF_LOADOUT_SLOT_TAUNT4    = 14,
	TF_LOADOUT_SLOT_TAUNT5    = 15,
	TF_LOADOUT_SLOT_TAUNT6    = 16,
	TF_LOADOUT_SLOT_TAUNT7    = 17,
	TF_LOADOUT_SLOT_TAUNT8    = 18,
};

int g_iPathLaserModelIndex = -1;

#define MAX_WEAPONS 48

Handle g_hSnapEyeAngles;

Handle g_hHasAmmo;
Handle g_hGetWeaponID;
Handle g_hGetProjectileSpeed;
Handle g_hGetProjectileGravity;

Handle g_hWorldSpaceCenter;
Handle g_hGetNextThink;
Handle g_hLookupBone;
Handle g_hGetBonePosition;
Handle g_hGetTimeSinceLastInjury;

Handle g_hGetMaxAmmo;
Handle g_hGetAmmoCount;

Handle g_hGetItemSchema;
Handle g_hGetAttributeDefinitionByName;

//CTFGameRules
Handle g_hCanUpgradeWithAttrib;
Handle g_hGetCostForUpgrade;
Handle g_hGetUpgradeTier;
Handle g_hIsUpgradeTierEnabled;

Address g_MannVsMachineUpgrades;
Address TheNavAreas;
Address navarea_count;

void InitGamedata()
{
	Handle hConf = LoadGameConfigFile("tf2.pets");

	//CTFGameRules
	StartPrepSDKCall(SDKCall_GameRules);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CTFGameRules::CanUpgradeWithAttrib");
	PrepSDKCall_AddParameter(SDKType_CBasePlayer, SDKPass_Pointer);		//CTFPlayer
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Slot
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Attribute index
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//CMannVsMachineUpgrades*
	PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_ByValue);			// return yes/no
	if ((g_hCanUpgradeWithAttrib = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFGameRules::CanUpgradeWithAttrib!");
	
	StartPrepSDKCall(SDKCall_GameRules);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CTFGameRules::GetCostForUpgrade");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//CMannVsMachineUpgrades*
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Slot
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Class
	PrepSDKCall_AddParameter(SDKType_CBasePlayer, SDKPass_Pointer);		//CTFPlayer
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);		// return cost
	if ((g_hGetCostForUpgrade = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFGameRules::GetCostForUpgrade!");
	
	StartPrepSDKCall(SDKCall_GameRules);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CTFGameRules::GetUpgradeTier");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//index
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);		// return tier
	if ((g_hGetUpgradeTier = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFGameRules::GetUpgradeTier!");
	
	StartPrepSDKCall(SDKCall_GameRules);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CTFGameRules::IsUpgradeTierEnabled");
	PrepSDKCall_AddParameter(SDKType_CBasePlayer, SDKPass_Pointer);		//CTFPlayer
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Slot
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);		//Tier
	PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_ByValue);			// return yes/no
	if ((g_hIsUpgradeTierEnabled = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFGameRules::IsUpgradeTierEnabled!");


	//CBasePlayer
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CBasePlayer::SnapEyeAngles");
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef);
	if ((g_hSnapEyeAngles = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CBasePlayer::SnapEyeAngles!");


	//CTFPlayer
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CTFPlayer::GetMaxAmmo");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hGetMaxAmmo = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFPlayer::GetMaxAmmo!");
	
	//CTFPlayer
	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFPlayer::GetAmmoCount");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hGetAmmoCount = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CTFPlayer::GetAmmoCount offset!");



	//CTFWeaponBaseGun::GetWeaponID()
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFWeaponBaseGun::GetWeaponID");
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);	//Returns WeaponID
	if ((g_hGetWeaponID = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for GetWeaponID offset!");
	
	//CTFWeaponBaseGun::HasAmmo()
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFWeaponBaseGun::HasAmmo");
	PrepSDKCall_SetReturnInfo(SDKType_Bool, SDKPass_ByValue);	//Returns true/false
	if ((g_hHasAmmo = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for HasAmmo offset!");
	
	//CTFWeaponBaseGun::GetProjectileSpeed()
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFWeaponBaseGun::GetProjectileSpeed");
	PrepSDKCall_SetReturnInfo(SDKType_Float, SDKPass_Plain);	//Returns SPEED
	if ((g_hGetProjectileSpeed = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for GetProjectileSpeed offset!");
	
	//CTFWeaponBaseGun::GetProjectileGravity()
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CTFWeaponBaseGun::GetProjectileGravity");
	PrepSDKCall_SetReturnInfo(SDKType_Float, SDKPass_Plain);	//Returns GRAVITY
	if ((g_hGetProjectileGravity = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for GetProjectileGravity offset!");
	
	
	
	
	//This call is used to get an entitys center position
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CBaseEntity::WorldSpaceCenter");
	PrepSDKCall_SetReturnInfo(SDKType_Vector, SDKPass_ByRef);
	if ((g_hWorldSpaceCenter = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CBaseEntity::WorldSpaceCenter offset!");
	
	//Get entitys next think tick.
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CBaseEntity::GetNextThink");
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_Float, SDKPass_Plain);
	if ((g_hGetNextThink = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CBaseEntity::GetNextThink signature!");
	
	//int CBaseAnimating::LookupBone( const char *szName )
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00\x75\x2A\x83\xBE\x6C\x04\x00\x00\x00\x75\x2A\xE8\x2A\x2A\x2A\x2A\x85\xC0\x74\x2A\x8B\xCE\xE8\x2A\x2A\x2A\x2A\x8B\x86\x6C\x04\x00\x00\x85\xC0\x74\x2A\x83\x38\x00\x74\x2A\xFF\x75\x08\x50\xE8\x2A\x2A\x2A\x2A\x83\xC4\x08\x5E", 68);
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if ((g_hLookupBone = EndPrepSDKCall()) == INVALID_HANDLE) SetFailState("Failed to create SDKCall for CBaseAnimating::LookupBone signature!");
	
	//void CBaseAnimating::GetBonePosition ( int iBone, Vector &origin, QAngle &angles )
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetSignature(SDKLibrary_Server, "\x55\x8B\xEC\x83\xEC\x30\x56\x8B\xF1\x80\xBE\x41\x03\x00\x00\x00", 16);
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_Vector, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	if ((g_hGetBonePosition = EndPrepSDKCall()) == INVALID_HANDLE) SetFailState("Failed to create SDKCall for CBaseAnimating::GetBonePosition signature!");
	
	
	
	//CBaseCombatCharacter
	
	//float CBaseCombatCharacter::GetTimeSinceLastInjury( int team /*= TEAM_ANY */ )
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Virtual, "CBaseCombatCharacter::GetTimeSinceLastInjury");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);	//team
	PrepSDKCall_SetReturnInfo(SDKType_Float, SDKPass_Plain);
	if ((g_hGetTimeSinceLastInjury = EndPrepSDKCall()) == INVALID_HANDLE) SetFailState("Failed to create SDKCall for CBaseCombatCharacter::GetTimeSinceLastInjury offset!");
	
	
	
	//GetItemSchema()
	StartPrepSDKCall(SDKCall_Static);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "GetItemSchema");
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);	//Returns address of CEconItemSchema
	if ((g_hGetItemSchema = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for GetItemSchema signature!"); 
	
	//CEconItemSchema::GetAttributeDefinitionByName(const char* name)
	StartPrepSDKCall(SDKCall_Raw);
	PrepSDKCall_SetFromConf(hConf, SDKConf_Signature, "CEconItemSchema::GetAttributeDefinitionByName");
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);	//Returns address of CEconItemAttributeDefinition
	if ((g_hGetAttributeDefinitionByName = EndPrepSDKCall()) == null) SetFailState("Failed to create SDKCall for CEconItemSchema::GetAttributeDefinitionByName signature!"); 



	g_MannVsMachineUpgrades = GameConfGetAddress(hConf, "MannVsMachineUpgrades");
	PrintToServer("Found \"g_MannVsMachineUpgrades\" @ 0x%X", g_MannVsMachineUpgrades);

	navarea_count = GameConfGetAddress(hConf, "navarea_count");
	PrintToServer("Found \"navarea_count\" @ 0x%X", navarea_count);
	
	//TheNavAreas is nicely above navarea_count
	TheNavAreas = view_as<Address>(LoadFromAddress(navarea_count + view_as<Address>(0x4), NumberType_Int32));
	PrintToServer("Found \"TheNavAreas\" @ 0x%X", TheNavAreas);
	
	
/*	JSONObject BombInfo = new JSONObject();
	bool success = GetBombInfo(BombInfo);
	
	char strJSON[5028];
	BombInfo.ToString(strJSON, sizeof(strJSON), JSON_INDENT(4));
	
	PrintToServer("\nsuccess ? %s\n%s\n", success ? "Yes" : "No", strJSON);

	delete BombInfo;
	*/

	delete hConf;
}

/*
struct BombInfo_t
{
	Vector closest_pos;
	float hatch_dist_fwd;
	float hatch_dist_back;
};
*/

//TODO Gamedata me
const int g_iBombTargetDistance = 548;	//windows 0x224h | 0x228h linux

stock bool GetBombInfo(JSONObject info)
{
	int iAreaCount = LoadFromAddress(navarea_count, NumberType_Int32);

	//Check that this map has any nav areas
	if ( iAreaCount <= 0 )
		return false;

	float hatch_dist = 0.0;
	
	for (int i = 0; i < (iAreaCount - 1); i++)
	{
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		
		//Skip spawn areas
		if (HasTFAttributes(area, (BLUE_SPAWN_ROOM)) || HasTFAttributes(area, (BLUE_SPAWN_ROOM))) {
			//PrintToServer("Skip spawn area.. #%i", area.GetID());
			continue;
		}
		
		float m_flBombTargetDistance = GetBombTargetDistance(area);
		
		hatch_dist = Max(Max(m_flBombTargetDistance, hatch_dist), 0.0);
	}
	
	int closest_flag = INVALID_ENT_REFERENCE;
	float closest_flag_pos[3];
	
	int flag = -1;
	while ((flag = FindEntityByClassname(flag, "item_teamflag")) != -1)
	{
		//Ignore bombs not in play
		if(GetEntProp(flag, Prop_Send, "m_nFlagStatus") == 0)
			continue;
		
		//Ignore bombs not on our team
		//if (GetEntProp(flag, Prop_Send, "m_iTeamNum") != view_as<int>(TFTeam_Blue))
			//continue;
			
		float flag_pos[3];
		
		int owner = GetOwnerEntity(flag);
		
		if (IsValidClientIndex(owner)) 	{
			flag_pos = GetAbsOrigin(owner);
		} else {
			flag_pos = WorldSpaceCenter(flag);
		}
		
		NavArea area = TheNavMesh.GetNearestNavArea_Vec(flag_pos);
		if(area == NavArea_Null)
			continue;
			
		if (HasTFAttributes(area, (BLUE_SPAWN_ROOM)) || HasTFAttributes(area, (BLUE_SPAWN_ROOM)))
			continue;
		
		float m_flBombTargetDistance = GetBombTargetDistance(area);
		
		if (m_flBombTargetDistance < hatch_dist) 
		{
			closest_flag = flag;
			hatch_dist = m_flBombTargetDistance;
			closest_flag_pos = flag_pos;
		}
	}
	
	//float range_back = FindConVar("tf_bot_engineer_mvm_sentry_hint_bomb_backward_range").FloatValue;
	//float range_fwd  = FindConVar("tf_bot_engineer_mvm_sentry_hint_bomb_forward_range").FloatValue;
	
	float range_fwd   = 2300.0;
	float range_back  = 1000.0;
	
	info.SetFloat("closest_pos_x", closest_flag_pos[0]);
	info.SetFloat("closest_pos_y", closest_flag_pos[1]);
	info.SetFloat("closest_pos_z", closest_flag_pos[2]);
	info.SetFloat("hatch_dist_back", hatch_dist + range_back);
	info.SetFloat("hatch_dist_fwd",  hatch_dist - range_fwd);
	
	return (closest_flag != INVALID_ENT_REFERENCE);
}

stock float GetBombTargetDistance(NavArea area)
{
	float m_flBombTargetDistance = view_as<float>(LoadFromAddress(view_as<Address>(area) + view_as<Address>(g_iBombTargetDistance), NumberType_Int32));
	return m_flBombTargetDistance;
}


//CTFNavArea::IsPotentiallyVisibleToTeam
stock bool IsPotentiallyVisibleToTeam(NavArea area, int team)
{
	int eax = (team + team * 4); //eax
	int edi = view_as<int>(area) + eax * 4 + 0x1D0;  //edi

	int count = LoadFromAddress(edi, NumberType_Int32);
	
	return team <= 3 && count > 0;
}

stock float[] GetTurretAngles(int sentry)
{
	if(!IsBaseObject(sentry))
		return NULL_VECTOR;
	
	float angle[3];
	
	int offset = FindSendPropInfo("CObjectSentrygun", "m_iAmmoRockets");
	int iAngleOffset = (offset - 36);
	
	angle[0] = GetEntDataFloat(sentry, iAngleOffset + 0);
	angle[1] = GetEntDataFloat(sentry, iAngleOffset + 4);
	angle[2] = GetEntDataFloat(sentry, iAngleOffset + 8);
	
	return angle;
}

stock int TF2_GetObject(int client, TFObjectType type)
{
	int iObject = INVALID_ENT_REFERENCE;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		
		if(GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type 
		&& !GetEntProp(iObject, Prop_Send, "m_bPlacing")
		&& !GetEntProp(iObject, Prop_Send, "m_bDisposableBuilding"))
		{			
			return iObject;
		}
	}
	
	return iObject;
}

stock int TF2_GetObjectCount(int client, TFObjectType type)
{
	int iObject = -1, iCount = 0;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		if(GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type)
		{
			iCount++;
		}
	}
	
	return iCount;
}

stock void TF2_DetonateObjectsOfType(int client, TFObjectType type)
{
	int iObject = -1;
	while ((iObject = FindEntityByClassname(iObject, "obj_*")) != -1)
	{
		TFObjectType iObjType = TF2_GetObjectType(iObject);
		if(GetEntPropEnt(iObject, Prop_Send, "m_hBuilder") == client && iObjType == type)
		{
			SetVariantInt(5000);
			AcceptEntityInput(iObject, "RemoveHealth", client);
		}
	}
}

stock void SnapEyeAngles(int client, float viewAngles[3])
{
	SDKCall(g_hSnapEyeAngles, client, viewAngles);
}

stock int LookupBone(int iEntity, const char[] szName)
{
	return SDKCall(g_hLookupBone, iEntity, szName);
}

stock void GetBonePosition(int iEntity, int iBone, float origin[3], float angles[3])
{
	SDKCall(g_hGetBonePosition, iEntity, iBone, origin, angles);
}

stock float GetTimeSinceLastInjury(int iEntity, int team = TEAM_ANY)
{
	return SDKCall(g_hGetTimeSinceLastInjury, iEntity, team);
}


//Attribute stuff

methodmap CEconItemAttributeDefinition
{
	property Address Address {
		public get() { 
			return view_as<Address>(this); 
		}
	}

	public int GetIndex() {
		int iAttribIndex = LoadFromAddress(this.Address + view_as<Address>(4), NumberType_Int32);
		
		if(iAttribIndex > 3018 || iAttribIndex < 0)
			iAttribIndex = LoadFromAddress(this.Address - view_as<Address>(4), NumberType_Int32); 
	
		return iAttribIndex
	}
}

public CEconItemAttributeDefinition GetAttributeDefinitionByName(const char[] szAttribute) 
{
	Address CEconItemSchema = SDKCall(g_hGetItemSchema);
	if(CEconItemSchema == Address_Null)
		return view_as<CEconItemAttributeDefinition>(Address_Null);
		
	return view_as<CEconItemAttributeDefinition>(SDKCall(g_hGetAttributeDefinitionByName, CEconItemSchema, szAttribute));
}

// MvM stuff

enum //CMannVsMachineUpgradeManager
{
	m_Upgrades = 12, //0x000C
	
	CMannVsMachineUpgradeManager_Size = 28
}; //Size=0x001C

#define MAX_UPGRADES 59	//FAKE NEWS

enum //CMannVsMachineUpgrades
{
	m_szAttribute = 0,   //0x0000
	m_szIcon = 128,      //0x0080
	m_flIncrement = 388, //0x0184
	m_flCap = 392,       //0x0188
	m_nCost = 396,       //0x018C
	m_iUIGroup = 400,    //0x0190
	m_iQuality = 404,    //0x0194
	m_iTier = 408,       //0x0198
	
	CMannVsMachineUpgrades_Size = 412
}; //Size=0x019C

methodmap CMannVsMachineUpgrades
{
	property Address Address 
	{
		public get() 
		{
			return view_as<Address>(this);
		}
	}
	
	public char m_szAttribute()
	{
		char attribute[128];
		
		for (int i = 0; i < sizeof(attribute); i++)	{
			attribute[i] = (LoadFromAddress(this.Address + view_as<Address>(i), NumberType_Int32));
		}
		
		return attribute;
	}
	
	public float m_flCap()
	{
		return float(LoadFromAddress(this.Address + view_as<Address>(m_flCap), NumberType_Int32));
	}
	
	public int m_iUIGroup()
	{
		return (LoadFromAddress(this.Address + view_as<Address>(m_iUIGroup), NumberType_Int32));
	}
}

methodmap CMannVsMachineUpgradeManager < CMannVsMachineUpgrades
{
	public CMannVsMachineUpgradeManager() 
	{
		return view_as<CMannVsMachineUpgradeManager>(g_MannVsMachineUpgrades);
	}
	
	public CMannVsMachineUpgrades GetUpgradeByIndex(int index)
	{
		Address Upgrades = ((this.Address) + view_as<Address>(m_Upgrades));
		Address pUpgrades = view_as<Address>(LoadFromAddress(Upgrades, NumberType_Int32));
		
		return view_as<CMannVsMachineUpgrades>(pUpgrades + view_as<Address>(index * CMannVsMachineUpgrades_Size));
	}
}

stock bool IsHealedByMedic(int client)
{
	for (int i = 0; i < GetEntProp(client, Prop_Send, "m_nNumHealers"); i++)
	{
		int iHealerIndex = GetHealerByIndex(client, i);
		
		//Not a player.
		if(!IsValidClientIndex(iHealerIndex))
			continue;
		
		return true;
	}
	
	return false;
}

stock int GetHealerByIndex(int client, int index)
{
	int m_aHealers = FindSendPropInfo("CTFPlayer", "m_nNumHealers") + 12;
	
	Address m_Shared = GetEntityAddress(client) + view_as<Address>(m_aHealers);
	Address aHealers = view_as<Address>(ReadInt(m_Shared));

	return ReadInt(Transpose(aHealers, (index * 0x24))) & 0xFFF;
}

stock int GetLastDamageType(int client)
{
	int m_LastDamageType = FindSendPropInfo("CTFPlayer", "m_flMvMLastDamageTime") + 20;
	return ReadInt(GetEntityAddress(client) + view_as<Address>(m_LastDamageType));
}

stock bool IsVaccinator(int client)
{
	return GetActiveWeapon(client) != INVALID_ENT_REFERENCE && GetEntProp(GetActiveWeapon(client), Prop_Send, "m_iItemDefinitionIndex") == 998;
}

stock int GetResistType(int client)
{
	return GetEntProp(GetActiveWeapon(client), Prop_Send, "m_nChargeResistType");
}

stock bool HasDemoShieldEquipped(int client)
{	
	return !!GetEntProp(client, Prop_Send, "m_bShieldEquipped");
}

stock bool HasParachuteEquipped(int client)
{
	return !!GetEntProp(client, Prop_Send, "m_bParachuteEquipped");
}

stock float GetNextThink(int entity, const char[] szContext)
{
	return SDKCall(g_hGetNextThink, entity, szContext);
}

stock void TF2_MoveTo(int client, float flGoal[3], float fVel[3], float fAng[3])
{
/*	float flPos[3];
	GetClientAbsOrigin(client, flPos);

	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);						
	
	fVel[0] = cos * newmove[0] - sin * newmove[1];
	fVel[1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(fVel, fVel);
	ScaleVector(fVel, 450.0);*/
	
	//WASD Movement
	float forward3D[3];
	EyeVectors(client, forward3D);
	
	float vForward[3];
	vForward[0] = forward3D[0];
	vForward[1] = forward3D[1];
	NormalizeVector(vForward, vForward);
		
	float right[3] 
	right[0] = vForward[1];
	right[1] = -vForward[0];

	float to[3]; 
	SubtractVectors(flGoal, GetAbsOrigin(client), to);

	/*float goalDistance = */
	NormalizeVector(to, to);

	float ahead = GetVectorDotProduct(to, vForward);
	float side  = GetVectorDotProduct(to, right);
	
	const float epsilon = 0.25;

	if ( ahead > epsilon )
	{
		fVel[0] = 450.0;
		//PressForwardButton();
	}
	else if ( ahead < -epsilon )
	{
		fVel[0] = -450.0;
		//PressBackwardButton();
	}

	if ( side <= -epsilon )
	{
		fVel[1] = -450.0;
		//PressLeftButton();
	}
	else if ( side >= epsilon )
	{
		fVel[1] = 450.0;
		//PressRightButton();
	}
}

stock float[] PredictSubjectPosition(int bot, int subject)
{
	float subjectPos[3]; subjectPos = GetAbsOrigin(subject);
	float subjectVel[3]; subjectVel = GetAbsVelocity(subject);
	float botPos[3];         botPos = GetAbsOrigin(bot);
	
	float to[3]; SubtractVectors(subjectPos, botPos, to);
	to[2] = 0.0;
	
	float flRangeSq = GetVectorLength(to, true);

	// don't lead if subject is very far away
	float flLeadRadiusSq = 1000.0;
	flLeadRadiusSq *= flLeadRadiusSq;
	if ( flRangeSq > flLeadRadiusSq )
		return subjectPos;

	// Normalize in place
	float range = SquareRoot(flRangeSq);
	to[0] /= (range + 0.0001);	// avoid divide by zero
	to[1] /= (range + 0.0001);	// avoid divide by zero
	to[2] /= (range + 0.0001);	// avoid divide by zero

	// estimate time to reach subject, assuming maximum speed
	float leadTime = 0.5 + ( range / ( GetEntPropFloat(bot, Prop_Send, "m_flMaxspeed") + 0.0001 ) );
	
	// estimate amount to lead the subject
	float lead[3];
	lead[0] = leadTime * subjectVel[0];
	lead[1] = leadTime * subjectVel[1];
	lead[2] = 0.0;

	if (GetVectorDotProduct(to, lead) < 0.0)
	{
		// the subject is moving towards us - only pay attention 
		// to his perpendicular velocity for leading
		
		float to2D[3];
		to2D[0] = to[0];
		to2D[1] = to[1];
		
		NormalizeVector(to2D, to2D);

		float perp[2];
		perp[0] = -to2D[1];
		perp[1] = to2D[0];

		float enemyGroundSpeed = lead[0] * perp[0] + lead[1] * perp[1];

		lead[0] = enemyGroundSpeed * perp[0];
		lead[1] = enemyGroundSpeed * perp[1];
	}

	// compute our desired destination
	float pathTarget[3]; AddVectors(subjectPos, lead, pathTarget);
	
	// validate this destination

	// don't lead through walls
	if (GetVectorLength(lead, true) > 36.0)
	{
		float fraction;
		if (!PF_IsPotentiallyTraversable(bot, subjectPos, pathTarget, IMMEDIATELY, fraction))
		{
			// tried to lead through an unwalkable area - clip to walkable space
			pathTarget[0] = subjectPos[0] + fraction * ( pathTarget[0] - subjectPos[0] );
			pathTarget[1] = subjectPos[1] + fraction * ( pathTarget[1] - subjectPos[1] );
			pathTarget[2] = subjectPos[2] + fraction * ( pathTarget[2] - subjectPos[2] );
			
			//PrintToServer("tried to lead through an unwalkable area - clip to walkable space | fraction %f", fraction); 
		}
	}

	// don't lead over cliffs
	NavArea leadArea = TheNavMesh.GetNearestNavArea_Vec( pathTarget );

	if (leadArea == NavArea_Null || leadArea.GetZ(pathTarget[0], pathTarget[1]) < (pathTarget[2] - 72.0))
	{
		// would fall off a cliff
		return subjectPos;
	}
	
	return pathTarget;
}

stock int GetBomb()
{
	int iBomb = -1;
	while ((iBomb = FindEntityByClassname(iBomb, "item_teamflag")) != -1)
	{
		//Ignore bombs not in play
		if(GetEntProp(iBomb, Prop_Send, "m_nFlagStatus") == 0)
			continue;
		
		//Ignore bombs not on our team
		if (GetEntProp(iBomb, Prop_Send, "m_iTeamNum") != view_as<int>(TFTeam_Blue))
			continue;
		
		break;
	}
	
	return iBomb;
}

stock int TF2_FindBotNearestToBombNearestToHatch(int client)
{
	int iBomb = TF2_FindBombNearestToHatch();
	if(iBomb == 0)
		return 0;
	
	float flOrigin[3]; flOrigin = WorldSpaceCenter(iBomb);
	
	float flBestDistance = 999999.0;
	int iBestEntity = 0;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == client)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(TF2_GetClientTeam(i) != GetEnemyTeam(client))
			continue;
		
		//Always ignore Sentry Busters
		if(TF2_IsBuster(i))
			continue;
		
		//Where are you
		NavArea lastKnownArea = PF_GetLastKnownArea(i);
		if(lastKnownArea == NavArea_Null)
			continue;
		
		//Cannot path into enemy spawn rooms
		if (HasTFAttributes(lastKnownArea, RED_SPAWN_ROOM|BLUE_SPAWN_ROOM))
			continue;
		
		float flDistance = GetVectorDistance(WorldSpaceCenter(i), flOrigin);
		if(flDistance <= flBestDistance)
		{
			flBestDistance = flDistance;
			iBestEntity = i;
		}
	}
	
	return iBestEntity;
}

stock int TF2_FindBotNearestToMe(int client, float max_distance)
{
	float flOrigin[3]; flOrigin = WorldSpaceCenter(client);
	
	float flBestDistance = 999999.0;
	int iBestEntity = 0;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == client)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(TF2_GetClientTeam(i) != GetEnemyTeam(client))
			continue;
		
		//Always ignore Sentry Busters
		if(TF2_IsBuster(i))
			continue;
		
		float flDistance = GetVectorDistance(WorldSpaceCenter(i), flOrigin);
		if(flDistance <= flBestDistance && flDistance <= max_distance)
		{
			flBestDistance = flDistance;
			iBestEntity = i;
		}
	}
	
	return iBestEntity;
}

stock int TF2_FindBombNearestToHatch()
{
	float flOrigin[3]; flOrigin = TF2_GetBombHatchPosition();
	
	float flBestDistance = 999999.0;
	int iBestEntity = -1;
	
	int iEnt = -1;
	while ((iEnt = FindEntityByClassname(iEnt, "item_teamflag")) != -1)
	{
		//Ignore bombs not in play
		if(GetEntProp(iEnt, Prop_Send, "m_nFlagStatus") == 0)
			continue;
		
		//Ignore bombs not on blue team.
		if (GetEntProp(iEnt, Prop_Send, "m_iTeamNum") != view_as<int>(TFTeam_Blue))
			continue;
	
		float flDistance = GetVectorDistance(flOrigin, WorldSpaceCenter(iEnt));
		if(flDistance <= flBestDistance)
		{
			flBestDistance = flDistance;
			iBestEntity = iEnt;
		}
	}
	
	return iBestEntity;
}

stock float[] TF2_GetBombHatchPosition()
{
	float flOrigin[3];

	int iHole = FindEntityByClassname(-1, "func_capturezone");
	if(IsValidEntity(iHole))
	{
		flOrigin = WorldSpaceCenter(iHole);
	}
	
	return flOrigin;
}

stock NavArea PickBuildArea(int client, float SentryRange = 1300.0)
{
	int iAreaCount = LoadFromAddress(navarea_count, NumberType_Int32);

	//Check that this map has any nav areas
	if ( iAreaCount <= 0 ) {
		return NavArea_Null;
	}
	
	JSONObject bombinfo = new JSONObject();
	
	if (!GetBombInfo(bombinfo)) 
	{	
		delete bombinfo;
		return PickBuildAreaPreRound(client);
	}
	
	float vecTargetPos[3];
	vecTargetPos[0] = bombinfo.GetFloat("closest_pos_x");
	vecTargetPos[1] = bombinfo.GetFloat("closest_pos_y");
	vecTargetPos[2] = bombinfo.GetFloat("closest_pos_z") + 40.0;
	
	NavArea bombArea = TheNavMesh.GetNearestNavArea_Vec(vecTargetPos, false, 90000.0, false, true, TEAM_ANY);
	if(bombArea == NavArea_Null)
	{
		return NavArea_Null;
	}
	
	if(HasTFAttributes(bombArea, BLUE_SPAWN_ROOM) 
	|| HasTFAttributes(bombArea, RED_SPAWN_ROOM))
	{
		return NavArea_Null;
	}

	//Areas forward of the bomb within some distance and visible to bomb.
	ArrayList ForwardVisibleAreas = new ArrayList();
	//Areas forward of the bomb but not necessarily visible.
	ArrayList ForwardAreas        = new ArrayList();
	//Areas visible to the bomb but not nescessarily forward of it.
	ArrayList VisibleAreasAround  = new ArrayList();
	
	//Loop all nav areas
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
		
		//Area in spawn
		if(HasTFAttributes(area, BLUE_SPAWN_ROOM) || HasTFAttributes(area, RED_SPAWN_ROOM))		
			continue;
		
		//TODO
		//Better solution because this will break on all non mvm maps.
		//Most likely areachable area
		if(!HasTFAttributes(area, BOMB_DROP))
			continue;
		
		float m_flBombTargetDistanceAtArea = GetBombTargetDistance(area);
		float m_flBombTargetDistanceAtBomb = GetBombTargetDistance(bombArea);
		
		if(m_flBombTargetDistanceAtArea < 180.0)
			continue;
		
		float areaCenter[3]; area.GetCenter(areaCenter);
		areaCenter[2] += 50.0;
		
		float flAreaDistanceToBomb = GetVectorDistance(areaCenter, vecTargetPos);
		
		if(flAreaDistanceToBomb >= SentryRange)
			continue;

		bool bAreaVisibleToBomb = area.IsVisible(vecTargetPos);
		
		if(bAreaVisibleToBomb)
		{
			VisibleAreasAround.Push(area);
		}
		
		if(m_flBombTargetDistanceAtBomb > m_flBombTargetDistanceAtArea)
		{
			if(flAreaDistanceToBomb <= SentryRange * GetRandomFloat(0.8, 1.75) && bAreaVisibleToBomb)
			{
				ForwardVisibleAreas.Push(area);
			}
			
			ForwardAreas.Push(area);
		}
	}
	
	PrintToServer("PickBuildArea %i ForwardVisibleAreas | %i ForwardAreas | %i VisibleAreasAroundBomb", ForwardVisibleAreas.Length, ForwardAreas.Length, VisibleAreasAround.Length);
	
	NavArea randomArea = NavArea_Null;
	
	if(ForwardVisibleAreas.Length     > 0) randomArea = ForwardVisibleAreas.Get(GetRandomInt(0, ForwardVisibleAreas.Length - 1));
	else if(ForwardAreas.Length       > 0) randomArea =        ForwardAreas.Get(GetRandomInt(0, ForwardAreas.Length        - 1));
	else if(VisibleAreasAround.Length > 0) randomArea =  VisibleAreasAround.Get(GetRandomInt(0, VisibleAreasAround.Length  - 1));
	
	delete ForwardVisibleAreas;
	delete ForwardAreas;
	delete VisibleAreasAround;
	
	delete bombinfo;
	
	return randomArea;
}

//Returns a nav area visible to a random enemy respawn room exit.
stock NavArea PickBuildAreaPreRound(int client, float SentryRange = 1300.0)
{
	int iAreaCount = LoadFromAddress(navarea_count, NumberType_Int32);

	//Check that this map has any nav areas
	if ( iAreaCount <= 0 ) {
		return NavArea_Null;
	}
	
	ArrayList EnemySpawnExits = new ArrayList();	

	//Collect enemy exit areas after spawn door.
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
			
		//BLOCKED
		if(HasTFAttributes(area, BLOCKED))
			continue;
		
		//BLOCKED
		if(HasTFAttributes(area, BLOCKED_AFTER_POINT_CAPTURE))
			continue;
		
		//BLOCKED
		if(HasTFAttributes(area, BLOCKED_UNTIL_POINT_CAPTURE))
			continue;
		
		//Area in spawn but not an exit
		if(GetBombTargetDistance(area) <= 0.0 && !HasTFAttributes(area, SPAWN_ROOM_EXIT))
			continue;
		
		//Area not an enemy spawn room exit
		if (GetEnemyTeam(client) == TFTeam_Blue && !HasTFAttributes(area, BLUE_SPAWN_ROOM))
			continue;
		
		//Area not an enemy spawn room exit
		if (GetEnemyTeam(client) == TFTeam_Red && !HasTFAttributes(area, RED_SPAWN_ROOM))
			continue;
		
		float flLowestBombTargetDistance = 999999.0;
		NavArea bestConnection = NavArea_Null;
		
		//Check spawn exit connections 
		for (NavDirType dir = NORTH; dir < NUM_DIRECTIONS; ++dir)
		{			
			//Only connections with BOMB_DROP attribute are considered good.
			for (int iConnection = 0; iConnection < area.GetAdjacentCount(dir); ++iConnection)
			{			
				NavArea adjArea = area.GetAdjacentArea(dir, iConnection);
				
				//Area still in spawn... BAD
				if(HasTFAttributes(adjArea, BLUE_SPAWN_ROOM) 
				|| HasTFAttributes(adjArea, RED_SPAWN_ROOM))
					continue;
				
				float flBombTargetDistance = GetBombTargetDistance(adjArea);
				
				//Area most likely in spawn
				if(flBombTargetDistance <= 0.0)
					continue;
				
				if (flBombTargetDistance <= flLowestBombTargetDistance)
				{
					bestConnection = adjArea;
					flLowestBombTargetDistance = flBombTargetDistance;
				}
			}
		}
		
		if(bestConnection != NavArea_Null)
		{
			EnemySpawnExits.Push(bestConnection);
			//g_hAreasToDraw.Push(bestConnection);
		}
	}
	
	//We've failed men.
	if(EnemySpawnExits.Length <= 0)
	{
		delete EnemySpawnExits;
		return NavArea_Null;
	}
	
	//Random valid exit point.
	NavArea RandomEnemySpawnExit = EnemySpawnExits.Get(GetRandomInt(0, EnemySpawnExits.Length - 1));

	//Search outward of the random exit untill we are some distance away.
	float vecExitCenter[3];
	RandomEnemySpawnExit.GetCenter(vecExitCenter);
	vecExitCenter[2] += 45.0;
	
	//PrintToServer("%f %f %f", vecExitCenter[0], vecExitCenter[1], vecExitCenter[2]);

	ArrayList AreasCloser                  = new ArrayList();	//Not necessarily visible but still <= 3000.0
	ArrayList VisibleAreas                 = new ArrayList();
	ArrayList VisibleAreasAfterSentryRange = new ArrayList();	//>= SentryRange
	
	for (int i = 0; i < iAreaCount; i++)
	{	
		NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
		if(area == NavArea_Null)
			continue;
		
		if (HasTFAttributes(area, (BLUE_SPAWN_ROOM|RED_SPAWN_ROOM)))
			continue;
			
		//TODO
		//Better solution because this will break on all non mvm maps.
		if(!HasTFAttributes(area, BOMB_DROP))
			continue;
			
		float center[3]; area.GetCenter(center);
		
		float flDistance = GetVectorDistance(center, vecExitCenter);
		
		if(flDistance <= SentryRange) {
			AreasCloser.Push(area);
		}
		
		if (!area.IsVisible(vecExitCenter)) {
			continue;
		}
		
		if(flDistance > (SentryRange * 0.75) && flDistance <= SentryRange * 1.25) {
			VisibleAreasAfterSentryRange.Push(area);
		}
		
		if(flDistance <= SentryRange) {
			VisibleAreasAfterSentryRange.Push(area);	
		}
		
		VisibleAreas.Push(area);
	}
	
	PrintToServer("PickBuildAreaPreRound %i VisibleAreas | %i VisibleAreasAfterSentryRange | %i AreasCloser", VisibleAreas.Length, VisibleAreasAfterSentryRange.Length, AreasCloser.Length);
	
	NavArea bestArea = NavArea_Null;
	
	if(VisibleAreasAfterSentryRange.Length > 0) bestArea = VisibleAreasAfterSentryRange.Get(GetRandomInt(0, VisibleAreasAfterSentryRange.Length - 1));
	else if(VisibleAreas.Length > 0)            bestArea =                 VisibleAreas.Get(GetRandomInt(0, VisibleAreas.Length                 - 1));
	else if(AreasCloser.Length > 0)             bestArea =                  AreasCloser.Get(GetRandomInt(0, AreasCloser.Length                  - 1));
	
	delete AreasCloser;
	delete VisibleAreas;
	delete VisibleAreasAfterSentryRange;
	
	return bestArea;
}

public int EquipRequiredWeapon(int client)
{
	//int iSlot = TFWeaponSlot_Primary;

//	if(IsMeleeOnly)
//	{
//		iSlot = TFWeaponSlot_Melee;
//	}
	
	//return iSlot;
}

stock int GetMaxHealth(int client)
{
	return GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxHealth", _, client);
}

stock int GetMaxHealthForBuffing(int client)
{
	return GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxBuffedHealth", _, client);
}

float VMX_VectorNormalize(float a1[3])
{
	float flLength = GetVectorLength(a1, true) + 0.0000000001;
	float v4 = (1.0 / SquareRoot(flLength)); 
	float den = v4 * ((3.0 - ((v4 * v4) * flLength)) * 0.5);
	
	ScaleVector(a1, den);
	
	return den * flLength;
}

stock bool IsInvulnerable(int client)
{
	if(TF2_IsPlayerInCondition(client, TFCond_Ubercharged)
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedHidden)
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedCanteen)
	|| TF2_IsPlayerInCondition(client, TFCond_PreventDeath))
	{
		return true;
	}
	
	return false;
}

stock bool IsWeapon(int client, int iWeaponID)
{
	int iWeapon = GetActiveWeapon(client);
	if(IsValidEntity(iWeapon))
	{
		return SDKCall(g_hGetWeaponID, iWeapon) == iWeaponID;
	}
	
	return false;
}

stock bool HasAmmo(int weapon)
{
	return SDKCall(g_hHasAmmo, weapon);
}

stock char EntityNetClass(int entity)
{
	char class[64];
	GetEntityNetClass(entity, class, sizeof(class));
	
	return class;
}

stock float GetTimeSinceWeaponFired(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return 9999.0;
		
	float flLastFireTime = GetEntPropFloat(iWeapon, Prop_Send, "m_flLastFireTime");
	if(flLastFireTime <= 0.0)
		return 9999.0;
		
	return GetGameTime() - flLastFireTime;
}

stock float GetProjectileSpeed(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return 0.0;		
	
	float flProjectileSpeed = SDKCall(g_hGetProjectileSpeed, iWeapon);
	if(flProjectileSpeed == 0.0)
	{
		//Some projectiles speeds are hardcoded so we manually return them here.
		switch(SDKCall(g_hGetWeaponID, iWeapon))
		{
			case TF_WEAPON_ROCKETLAUNCHER:   flProjectileSpeed = 1100.0;
			case TF_WEAPON_DIRECTHIT:        flProjectileSpeed = 1980.0;
			case TF_WEAPON_FLAREGUN:         flProjectileSpeed = 2000.0;
			case TF_WEAPON_RAYGUN_REVENGE:   flProjectileSpeed = 2000.0; //Manmelter
			case TF_WEAPON_FLAMETHROWER:     flProjectileSpeed = 1500.0;
			case TF_WEAPON_SYRINGEGUN_MEDIC: flProjectileSpeed = 990.0;
		}
	}
	
	return flProjectileSpeed;
}

stock float GetProjectileGravity(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(IsValidEntity(iWeapon))
	{
		return SDKCall(g_hGetProjectileGravity, iWeapon);
	}
	
	return 0.0;
}

stock float GetCurrentCharge(int iWeapon)
{
	if(!HasEntProp(iWeapon, Prop_Send, "m_flChargeBeginTime"))
		return 0.0;
	
	float flCharge = 0.0;
	
	float flChargeBeginTime = GetEntPropFloat(iWeapon, Prop_Send, "m_flChargeBeginTime");
	if(flChargeBeginTime != 0.0)
	{
		flCharge = Min(1.0, GetGameTime() - flChargeBeginTime);
	}
	
	return flCharge;
}

stock float GetDesiredAttackRange(int client)
{
	int weapon = GetActiveWeapon(client);
	if (!IsValidEntity(weapon))
		return 0.0;
	
	if(IsWeapon(client, TF_WEAPON_KNIFE))
		return 70.0;
	
	if (IsMeleeWeapon(client))
	{
		if(TF2_IsPlayerInCondition(client, TFCond_Charging))
			return 80.0;
			
		return 100.0;
	}
	
	if (!IsWeapon(client, TF_WEAPON_FLAMETHROWER))
	{
		float flRange = 99999.0;
		
		if (!IsSniperRifle(client))
		{
			if(IsWeapon(client, TF_WEAPON_ROCKETLAUNCHER) || TF2_IsMvM())
				flRange = 500.0;
			else
				flRange = 1250.0;
		}
		
		return flRange;
	}
	
	if(IsExplosiveProjectileWeapon(client))
		return 500.0;
	
	return 100.0;
}

stock float GetMaxAttackRange(int client)
{
	int weapon = GetActiveWeapon(client);
	if (!IsValidEntity(weapon))
		return 0.0;
	
	if(IsWeapon(client, TF_WEAPON_KNIFE))
		return 70.0;
	
	if (IsMeleeWeapon(client))
	{
		if(TF2_IsPlayerInCondition(client, TFCond_Charging))
			return 80.0;
			
		return 100.0;
	}
	
	if (IsSniperRifle(client))
		return 9999999.0;
	
	if (IsWeapon(client, TF_WEAPON_FLAMETHROWER)) 
	{
		if (TF2_IsMvM()) 
		{
			if(IsMiniBoss(client)) 
			{
				return 875.0;
			} 
			else 
			{
				return 350.0;
			}
		} 
		else 
		{
			return 250.0;
		}
	}
	
	if (IsWeapon(client, TF_WEAPON_ROCKETLAUNCHER))
		return 3000.0;
	
	return 9999999.0;
}

stock bool IsAmmoLow(int client)
{
	if(g_iCurrentAction[client] == ACTION_MVM_ENGINEER_BUILD_SENTRYGUN && !CTFBotMvMEngineerBuildSentryGun_IsPossible(client)
	|| g_iCurrentAction[client] == ACTION_MVM_ENGINEER_BUILD_DISPENSER && !CTFBotMvMEngineerBuildDispenser_IsPossible(client))
		return true;

	int Primary = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary);

	if(IsValidEntity(Primary) && !HasAmmo(Primary))
	{
		return true;
	}

	if(!IsWeapon(client, TF_WEAPON_WRENCH))
	{
		if(!IsMeleeWeapon(client))
		{
			float flAmmoRation = float(GetAmmoCount(client, TF_AMMO_PRIMARY)) / float(GetMaxAmmo(client, TF_AMMO_PRIMARY));
			return flAmmoRation < 0.2;
		}
		
		return false;
	}
	
	return GetAmmoCount(client, TF_AMMO_METAL) <= 0;
}

stock int GetMaxAmmo(int client, int iAmmoType, int iClassNumber = -1)
{
	return SDKCall(g_hGetMaxAmmo, client, iAmmoType, iClassNumber);
}

stock int GetAmmoCount(int client, int iAmmoType)
{
	return SDKCall(g_hGetAmmoCount, client, iAmmoType);
}

stock bool IsContinuousFireWeapon(int client) 
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return false;
	
	if(IsSniperRifle(client))
		return false;
	
	if(!IsCombatWeapon(client))
		return false;

	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_PIPEBOMBLAUNCHER: return false;
		case TF_WEAPON_GRENADELAUNCHER: return false;
		case TF_WEAPON_ROCKETLAUNCHER: return false;
		case TF_WEAPON_PISTOL_SCOUT: return false;
		case TF_WEAPON_COMPOUND_BOW: return false;
		case TF_WEAPON_DIRECTHIT: return false;
		case TF_WEAPON_FLAREGUN: return false;
		case TF_WEAPON_PISTOL: return false;
		case TF_WEAPON_JAR: return false;
    }
    
	return true;
}

stock bool IsCombatWeapon(int client)
{
	int iWeapon = GetActiveWeapon(client);
    
	return IsCombatWeapon_Index(iWeapon);
}

stock bool IsCombatWeapon_Index(int iWeapon)
{
	if(!IsValidEntity(iWeapon))
		return false;

	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_PDA:                  return false;
		case TF_WEAPON_PDA_ENGINEER_BUILD:   return false;
		case TF_WEAPON_PDA_ENGINEER_DESTROY: return false;
		case TF_WEAPON_PDA_SPY:              return false;
		case TF_WEAPON_BUILDER:              return false;
		case TF_WEAPON_MEDIGUN:              return false;
		case TF_WEAPON_DISPENSER:            return false;
		case TF_WEAPON_INVIS:                return false;
		case TF_WEAPON_LUNCHBOX:             return false;
		case TF_WEAPON_BUFF_ITEM:            return false;
		case TF_WEAPON_PUMPKIN_BOMB:         return false;
		case TF_WEAPON_PARACHUTE:            return false;
    }
    
	return true;
}

stock bool IsHitScanWeapon(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return false;

	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_SHOTGUN_PRIMARY: return true;
		case TF_WEAPON_SHOTGUN_SOLDIER: return true;
		case TF_WEAPON_SHOTGUN_HWG: return true;
		case TF_WEAPON_SHOTGUN_PYRO: return true;
		case TF_WEAPON_SCATTERGUN: return true;
		case TF_WEAPON_SNIPERRIFLE: return true;
		case TF_WEAPON_MINIGUN: return true;
		case TF_WEAPON_SMG: return true;
		case TF_WEAPON_PISTOL: return true;
		case TF_WEAPON_PISTOL_SCOUT: return true;
		case TF_WEAPON_REVOLVER: return true;
		case TF_WEAPON_SENTRY_BULLET: return true;
		case TF_WEAPON_SENTRY_ROCKET: return true;
		case TF_WEAPON_SENTRY_REVENGE: return true;
		case TF_WEAPON_HANDGUN_SCOUT_PRIMARY: return true;
		case TF_WEAPON_HANDGUN_SCOUT_SEC: return true;
		case TF_WEAPON_SODA_POPPER: return true;
		case TF_WEAPON_SNIPERRIFLE_DECAP: return true;
		case TF_WEAPON_PEP_BRAWLER_BLASTER: return true;
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: return true;
		case TF_WEAPON_CHARGED_SMG: return true;
    }
    
	return false;
}

stock bool IsSniperRifle(int client)
{
	int iWeapon = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary);
	if(!IsValidEntity(iWeapon))
		return false;
	
	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_SNIPERRIFLE:         return true;
		case TF_WEAPON_SNIPERRIFLE_DECAP:   return true;
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: return true;
	}
	
	return false;
}

stock int GetWeaponID(int weapon)
{
	return SDKCall(g_hGetWeaponID, weapon);
}

stock bool IsExplosiveProjectileWeapon(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return false;
	
	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_PIPEBOMBLAUNCHER: return true;
		case TF_WEAPON_GRENADELAUNCHER:  return true;
		case TF_WEAPON_PARTICLE_CANNON:  return true;
		case TF_WEAPON_ROCKETLAUNCHER:   return true;
		case TF_WEAPON_DIRECTHIT:        return true;
		case TF_WEAPON_CANNON:           return true;
		case TF_WEAPON_JAR:              return true;
	}
	
	return false;
}

stock bool IsMeleeWeapon(int client)
{
	int iWeapon = GetActiveWeapon(client);
	if(!IsValidEntity(iWeapon))
		return false;
	
	switch(SDKCall(g_hGetWeaponID, iWeapon))
	{
		case TF_WEAPON_BAT:           return true;
		case TF_WEAPON_SLAP:          return true;
		case TF_WEAPON_CLUB:          return true;
		case TF_WEAPON_KNIFE:         return true;
		case TF_WEAPON_FISTS:         return true;
		case TF_WEAPON_SWORD:         return true;
		case TF_WEAPON_SHOVEL:        return true;
		case TF_WEAPON_BOTTLE:        return true;
		case TF_WEAPON_WRENCH:        return true;
		case TF_WEAPON_BONESAW:       return true;
		case TF_WEAPON_CROWBAR:       return true;
		case TF_WEAPON_FIREAXE:       return true;
		case TF_WEAPON_BAT_WOOD:      return true;
		case TF_WEAPON_BAT_FISH:      return true;
		case TF_WEAPON_BAT_GIFTWRAP:  return true;
		case TF_WEAPON_HARVESTER_SAW: return true;
	}
	
	return false;
}


stock char CurrentRouteTypeToName(int client)
{
	char name[PLATFORM_MAX_PATH];

	switch(m_iRouteType[client])
	{
		case DEFAULT_ROUTE: name = "DEFAULT";
		case FASTEST_ROUTE: name = "FASTEST";
		case SAFEST_ROUTE:  name = "SAFEST";
		case RETREAT_ROUTE: name = "RETREAT";
	}
	
	return name;
}

stock char CurrentActionToName(int action)
{
	char name[PLATFORM_MAX_PATH];

	switch(action)
	{
		case ACTION_MARK_GIANT:    name = "MARK GIANT";
		case ACTION_COLLECT_MONEY: name = "COLLECT MONEY";
		case ACTION_UPGRADE:       name = "UPGRADE";
		case ACTION_GOTO_UPGRADE:  name = "GO TO UPGRADE STATION";
		case ACTION_ATTACK:        name = "ATTACK";
		case ACTION_GET_AMMO:      name = "GET AMMO";
		case ACTION_GET_HEALTH:    name = "GET HEALTH";
		case ACTION_MOVE_TO_FRONT: name = "MOVE TO FRONT";
		case ACTION_USE_ITEM:      name = "USE ITEM";
		case ACTION_IDLE:          name = "IDLE";
		case ACTION_SNIPER_LURK:   name = "SNIPER LURK";
		case ACTION_MEDIC_HEAL:    name = "MEDIC HEAL";
		case ACTION_MELEE_ATTACK:  name = "MELEE ATTACK";
		case ACTION_MVM_ENGINEER_IDLE:  name = "MVM ENGINEER IDLE";
		case ACTION_MVM_ENGINEER_BUILD_SENTRYGUN: name = "MVM ENGINEER BUILD SENTRY";
		case ACTION_MVM_ENGINEER_BUILD_DISPENSER: name = "MVM ENGINEER BUILD DISPENSER";
	}
	
	return name;
}

stock char CurrentWeaponIDToName(int client)
{
	char name[PLATFORM_MAX_PATH];

	switch(GetWeaponID(GetActiveWeapon(client)))
	{
		case TF_WEAPON_NONE: name = "TF_WEAPON_NONE"
		case TF_WEAPON_BAT: name = "TF_WEAPON_BAT"
		case TF_WEAPON_BAT_WOOD: name = "TF_WEAPON_BAT_WOOD"
		case TF_WEAPON_BOTTLE: name = "TF_WEAPON_BOTTLE"
		case TF_WEAPON_FIREAXE: name = "TF_WEAPON_FIREAXE"
		case TF_WEAPON_CLUB: name = "TF_WEAPON_CLUB"
		case TF_WEAPON_CROWBAR: name = "TF_WEAPON_CROWBAR"
		case TF_WEAPON_KNIFE: name = "TF_WEAPON_KNIFE"
		case TF_WEAPON_FISTS: name = "TF_WEAPON_FISTS"
		case TF_WEAPON_SHOVEL: name = "TF_WEAPON_SHOVEL"
		case TF_WEAPON_WRENCH: name = "TF_WEAPON_WRENCH"
		case TF_WEAPON_BONESAW: name = "TF_WEAPON_BONESAW"
		case TF_WEAPON_SHOTGUN_PRIMARY: name = "TF_WEAPON_SHOTGUN_PRIMARY"
		case TF_WEAPON_SHOTGUN_SOLDIER: name = "TF_WEAPON_SHOTGUN_SOLDIER"
		case TF_WEAPON_SHOTGUN_HWG: name = "TF_WEAPON_SHOTGUN_HWG"
		case TF_WEAPON_SHOTGUN_PYRO: name = "TF_WEAPON_SHOTGUN_PYRO"
		case TF_WEAPON_SCATTERGUN: name = "TF_WEAPON_SCATTERGUN"
		case TF_WEAPON_SNIPERRIFLE: name = "TF_WEAPON_SNIPERRIFLE"
		case TF_WEAPON_MINIGUN: name = "TF_WEAPON_MINIGUN"
		case TF_WEAPON_SMG: name = "TF_WEAPON_SMG"
		case TF_WEAPON_SYRINGEGUN_MEDIC: name = "TF_WEAPON_SYRINGEGUN_MEDIC"
		case TF_WEAPON_TRANQ: name = "TF_WEAPON_TRANQ"
		case TF_WEAPON_ROCKETLAUNCHER: name = "TF_WEAPON_ROCKETLAUNCHER"
		case TF_WEAPON_GRENADELAUNCHER: name = "TF_WEAPON_GRENADELAUNCHER"
		case TF_WEAPON_PIPEBOMBLAUNCHER: name = "TF_WEAPON_PIPEBOMBLAUNCHER"
		case TF_WEAPON_FLAMETHROWER: name = "TF_WEAPON_FLAMETHROWER"
		case TF_WEAPON_GRENADE_NORMAL: name = "TF_WEAPON_GRENADE_NORMAL"
		case TF_WEAPON_GRENADE_CONCUSSION: name = "TF_WEAPON_GRENADE_CONCUSSION"
		case TF_WEAPON_GRENADE_NAIL: name = "TF_WEAPON_GRENADE_NAIL"
		case TF_WEAPON_GRENADE_MIRV: name = "TF_WEAPON_GRENADE_MIRV"
		case TF_WEAPON_GRENADE_MIRV_DEMOMAN: name = "TF_WEAPON_GRENADE_MIRV_DEMOMAN"
		case TF_WEAPON_GRENADE_NAPALM: name = "TF_WEAPON_GRENADE_NAPALM"
		case TF_WEAPON_GRENADE_GAS: name = "TF_WEAPON_GRENADE_GAS"
		case TF_WEAPON_GRENADE_EMP: name = "TF_WEAPON_GRENADE_EMP"
		case TF_WEAPON_GRENADE_CALTROP: name = "TF_WEAPON_GRENADE_CALTROP"
		case TF_WEAPON_GRENADE_PIPEBOMB: name = "TF_WEAPON_GRENADE_PIPEBOMB"
		case TF_WEAPON_GRENADE_SMOKE_BOMB: name = "TF_WEAPON_GRENADE_SMOKE_BOMB"
		case TF_WEAPON_GRENADE_HEAL: name = "TF_WEAPON_GRENADE_HEAL"
		case TF_WEAPON_GRENADE_STUNBALL: name = "TF_WEAPON_GRENADE_STUNBALL"
		case TF_WEAPON_GRENADE_JAR: name = "TF_WEAPON_GRENADE_JAR"
		case TF_WEAPON_GRENADE_JAR_MILK: name = "TF_WEAPON_GRENADE_JAR_MILK"
		case TF_WEAPON_PISTOL: name = "TF_WEAPON_PISTOL"
		case TF_WEAPON_PISTOL_SCOUT: name = "TF_WEAPON_PISTOL_SCOUT"
		case TF_WEAPON_REVOLVER: name = "TF_WEAPON_REVOLVER"
		case TF_WEAPON_NAILGUN: name = "TF_WEAPON_NAILGUN"
		case TF_WEAPON_PDA: name = "TF_WEAPON_PDA"
		case TF_WEAPON_PDA_ENGINEER_BUILD: name = "TF_WEAPON_PDA_ENGINEER_BUILD"
		case TF_WEAPON_PDA_ENGINEER_DESTROY: name = "TF_WEAPON_PDA_ENGINEER_DESTROY"
		case TF_WEAPON_PDA_SPY: name = "TF_WEAPON_PDA_SPY"
		case TF_WEAPON_BUILDER: name = "TF_WEAPON_BUILDER"
		case TF_WEAPON_MEDIGUN: name = "TF_WEAPON_MEDIGUN"
		case TF_WEAPON_GRENADE_MIRVBOMB: name = "TF_WEAPON_GRENADE_MIRVBOMB"
		case TF_WEAPON_FLAMETHROWER_ROCKET: name = "TF_WEAPON_FLAMETHROWER_ROCKET"
		case TF_WEAPON_GRENADE_DEMOMAN: name = "TF_WEAPON_GRENADE_DEMOMAN"
		case TF_WEAPON_SENTRY_BULLET: name = "TF_WEAPON_SENTRY_BULLET"
		case TF_WEAPON_SENTRY_ROCKET: name = "TF_WEAPON_SENTRY_ROCKET"
		case TF_WEAPON_DISPENSER: name = "TF_WEAPON_DISPENSER"
		case TF_WEAPON_INVIS: name = "TF_WEAPON_INVIS"
		case TF_WEAPON_FLAREGUN: name = "TF_WEAPON_FLAREGUN"
		case TF_WEAPON_LUNCHBOX: name = "TF_WEAPON_LUNCHBOX"
		case TF_WEAPON_JAR: name = "TF_WEAPON_JAR"
		case TF_WEAPON_COMPOUND_BOW: name = "TF_WEAPON_COMPOUND_BOW"
		case TF_WEAPON_BUFF_ITEM: name = "TF_WEAPON_BUFF_ITEM"
		case TF_WEAPON_PUMPKIN_BOMB: name = "TF_WEAPON_PUMPKIN_BOMB"
		case TF_WEAPON_SWORD: name = "TF_WEAPON_SWORD"
		case TF_WEAPON_DIRECTHIT: name = "TF_WEAPON_DIRECTHIT"
		case TF_WEAPON_LIFELINE: name = "TF_WEAPON_LIFELINE"
		case TF_WEAPON_LASER_POINTER: name = "TF_WEAPON_LASER_POINTER"
		case TF_WEAPON_DISPENSER_GUN: name = "TF_WEAPON_DISPENSER_GUN"
		case TF_WEAPON_SENTRY_REVENGE: name = "TF_WEAPON_SENTRY_REVENGE"
		case TF_WEAPON_JAR_MILK: name = "TF_WEAPON_JAR_MILK"
		case TF_WEAPON_HANDGUN_SCOUT_PRIMARY: name = "TF_WEAPON_JAR_MILK"
		case TF_WEAPON_BAT_FISH: name = "TF_WEAPON_BAT_FISH"
		case TF_WEAPON_CROSSBOW: name = "TF_WEAPON_CROSSBOW"
		case TF_WEAPON_STICKBOMB: name = "TF_WEAPON_STICKBOMB"
		case TF_WEAPON_HANDGUN_SCOUT_SEC: name = "TF_WEAPON_HANDGUN_SCOUT_SEC"
		case TF_WEAPON_SODA_POPPER: name = "TF_WEAPON_SODA_POPPER"
		case TF_WEAPON_SNIPERRIFLE_DECAP: name = "TF_WEAPON_SNIPERRIFLE_DECAP"
		case TF_WEAPON_RAYGUN: name = "TF_WEAPON_RAYGUN"
		case TF_WEAPON_PARTICLE_CANNON: name = "TF_WEAPON_PARTICLE_CANNON"
		case TF_WEAPON_MECHANICAL_ARM: name = "TF_WEAPON_MECHANICAL_ARM"
		case TF_WEAPON_DRG_POMSON: name = "TF_WEAPON_DRG_POMSON"
		case TF_WEAPON_BAT_GIFTWRAP: name = "TF_WEAPON_BAT_GIFTWRAP"
		case TF_WEAPON_GRENADE_ORNAMENT: name = "TF_WEAPON_GRENADE_ORNAMENT"
		case TF_WEAPON_RAYGUN_REVENGE: name = "TF_WEAPON_RAYGUN_REVENGE"
		case TF_WEAPON_PEP_BRAWLER_BLASTER: name = "TF_WEAPON_PEP_BRAWLER_BLASTER"
		case TF_WEAPON_CLEAVER: name = "TF_WEAPON_CLEAVER"
		case TF_WEAPON_GRENADE_CLEAVER: name = "TF_WEAPON_GRENADE_CLEAVER"
		case TF_WEAPON_STICKY_BALL_LAUNCHER: name = "TF_WEAPON_STICKY_BALL_LAUNCHER"
		case TF_WEAPON_GRENADE_STICKY_BALL: name = "TF_WEAPON_GRENADE_STICKY_BALL"
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: name = "TF_WEAPON_SHOTGUN_BUILDING_RESCUE"
		case TF_WEAPON_CANNON: name = "TF_WEAPON_CANNON"
		case TF_WEAPON_THROWABLE: name = "TF_WEAPON_THROWABLE"
		case TF_WEAPON_GRENADE_THROWABLE: name = "TF_WEAPON_GRENADE_THROWABLE"
		case TF_WEAPON_PDA_SPY_BUILD: name = "TF_WEAPON_PDA_SPY_BUILD"
		case TF_WEAPON_GRENADE_WATERBALLOON: name = "TF_WEAPON_GRENADE_WATERBALLOON"
		case TF_WEAPON_HARVESTER_SAW: name = "TF_WEAPON_HARVESTER_SAW"
		case TF_WEAPON_SPELLBOOK: name = "TF_WEAPON_SPELLBOOK"
		case TF_WEAPON_SPELLBOOK_PROJECTILE: name = "TF_WEAPON_SPELLBOOK_PROJECTILE"
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: name = "TF_WEAPON_SNIPERRIFLE_CLASSIC"
		case TF_WEAPON_PARACHUTE: name = "TF_WEAPON_PARACHUTE"
		case TF_WEAPON_GRAPPLINGHOOK: name = "TF_WEAPON_GRAPPLINGHOOK"
		case TF_WEAPON_PASSTIME_GUN: name = "TF_WEAPON_PASSTIME_GUN"
		case TF_WEAPON_CHARGED_SMG: name = "TF_WEAPON_CHARGED_SMG"
		case TF_WEAPON_BREAKABLE_SIGN: name = "TF_WEAPON_BREAKABLE_SIGN"
		case TF_WEAPON_ROCKETPACK: name = "TF_WEAPON_ROCKETPACK"
		case TF_WEAPON_SLAP: name = "TF_WEAPON_SLAP"
		case TF_WEAPON_JAR_GAS: name = "TF_WEAPON_JAR_GAS"
		case TF_WEAPON_GRENADE_JAR_GAS: name = "TF_WEAPON_GRENADE_JAR_GAS"
		case TF_WEAPON_FLAME_BALL: name = "TF_WEAPON_FLAME_BALL"
	}
	
	return name;
}


bool IsLineOfFireClear(float from[3], float to[3])
{
	Handle trace = TR_TraceRayFilterEx(from, to, CONTENTS_SOLID|CONTENTS_MOVEABLE|0x40|CONTENTS_MONSTER, RayType_EndPoint, NextBotTraceFilterIgnoreActors);
	
	float flFraction = TR_GetFraction(trace);
	
	delete trace;
	
	if (flFraction >= 1.0/* && !trace.allsolid*/) 
	{
		return !(flFraction == 0.0);	//allsolid
	}
	
	return false;
}

public bool NextBotTraceFilterIgnoreActors(int entity, int contentsMask, any iExclude)
{
	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	
	if(StrEqual(class, "entity_medigun_shield"))
	{
		return false;
	}
	else if(StrEqual(class, "func_respawnroomvisualizer"))
	{
		return false;
	}
	else if(StrContains(class, "tf_projectile_", false) != -1)
	{
		return false;
	}
	else if(StrContains(class, "obj_", false) != -1)
	{
		return false;
	}
	else if(StrEqual(class, "entity_revive_marker"))
	{
		return false;
	}

	return !PF_IsEntityACombatCharacter(entity);
}

stock float AngleDiff( float destAngle, float srcAngle )
{
	return AngleNormalize(destAngle - srcAngle);
}

stock float AngleNormalize( float angle )
{
	angle = angle - 360.0 * RoundToFloor(angle / 360.0);
	while (angle > 180.0) angle -= 360.0;
	while (angle < -180.0) angle += 360.0;
	return angle;
}

stock float ApproachAngle( float target, float value, float speed )
{
	float delta = AngleDiff(target, value);
	
	if (speed < 0.0) 
		speed = -speed;
	
	if (delta > speed) 
		value += speed;
	else if (delta < -speed) 
		value -= speed;
	else
		value = target;
	
	return AngleNormalize(value);
}

stock float[] WorldSpaceCenter(int entity)
{
	if(entity <= 0)
		return NULL_VECTOR;

	float vecPos[3];
	SDKCall(g_hWorldSpaceCenter, entity, vecPos);
	return vecPos;
}

stock float[] GetAbsVelocity(int client)
{
	if(client <= 0)
		return NULL_VECTOR;
	
	float v[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", v);
	return v;
}

stock float[] GetAbsOrigin(int client)
{
	if(client <= 0)
		return NULL_VECTOR;

	float v[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsOrigin", v);
	return v;
}

stock float[] GetAbsAngles(int client)
{
	if(client <= 0)
		return NULL_VECTOR;

	float v[3];
	GetEntPropVector(client, Prop_Data, "m_angRotation", v);
	return v;
}

stock float[] GetEyePosition(int client)
{
	if(client <= 0)
		return NULL_VECTOR;
		
	float v[3];
	GetClientEyePosition(client, v);
	return v;
}

stock float[] GetEyeAngles(int client)
{
	if(client <= 0)
		return NULL_VECTOR;

	float v[3];
	GetClientEyeAngles(client, v);
	return v;
}

stock int GetOwnerEntity(int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
}

stock void SetOwnerEntity(int entity, int owner)
{
	SetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity", owner);
}

stock int GetActiveWeapon(int client)
{
	return GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
}

stock void EyeVectors(int client, float fw[3] = NULL_VECTOR, float right[3] = NULL_VECTOR, float up[3] = NULL_VECTOR)
{
	GetAngleVectors(GetEyeAngles(client), fw, right, up);
}

/*
//Useless; it's the same as teammate pushaway, except bots use this.
stock void AvoidPlayers(int client, float &forwardmove, float &sidemove)
{
	float vForward[3], vLeft[3];
	EyeVectors(client, vForward, vLeft, NULL_VECTOR);
	
	float origin[3];
	float flRadius = 50.0;
	
	if (TF2_IsMvM())// In MvM the radius is 150
		flRadius = 150.0;
		
	float flRadiusSquared = flRadius * flRadius;
	float flRadiusNormalized = 1.0 / flRadius;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == client)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		//Ignore dead
		if(!IsPlayerAlive(i))
			continue;
		
		//Ignore non enemies
		if(TF2_GetClientTeam(i) != TF2_GetClientTeam(client))
			continue;
		
		float vecToTeammate[3];
		SubtractVectors(GetAbsOrigin(client), GetAbsOrigin(i), vecToTeammate);
		
		float flDistanceToTeammate = GetVectorLength(vecToTeammate, true);
		
		//PrintToServer("flRadiusSquared %f  flDistanceToTeammate %f", flRadiusSquared, flDistanceToTeammate);
		
		if ( flRadiusSquared > flDistanceToTeammate )
		{
			float v11 = 1.0 / SquareRoot(flDistanceToTeammate + 0.000000001);
			float v12 = v11 * ((3.0 - ((v11 * v11) * (flDistanceToTeammate + 0.000000001))) * 0.5);
			float v13 = 1.0 - (((flDistanceToTeammate + 0.000000001) * flRadiusNormalized) * v12);
			origin[0] = ((vecToTeammate[0] * v12) * v13) + origin[0];
			origin[1] = ((vecToTeammate[1] * v12) * v13) + origin[1];
			origin[2] = ((vecToTeammate[2] * v12) * v13) + origin[2];
		}
	}
	
	if (origin[0] <= -0.0099999998 || origin[0] >= 0.0099999998
	 || origin[1] <= -0.0099999998 || origin[1] >= 0.0099999998
	 || origin[2] <= -0.0099999998 || origin[2] >= 0.0099999998)
	{
		*(CTFBot + 6880) = 1;                   // m_bAvoiding
		float v15 = (((origin[2] * origin[2]) + (origin[0] * origin[0])) + 0.000000001) + (origin[1] * origin[1]);
		float v16 = SquareRoot(v15);
		float v17 = (1.0 / v16) * ((3.0 - (((1.0 / v16) * (1.0 / v16)) * v15)) * 0.5);
		float v18 = origin[0] * v17;
		float v19 = origin[1] * v17;
		float v20 = v17 * origin[2];
		float v21 = vLeft[0] * v18;
		float v23 = v18 * vForward[0];
		*(CTFBot + 1721) = v18 * 50.0;
		*(CTFBot + 1722) = v19 * 50.0;
		*(CTFBot + 1723) = v20 * 50.0;
		forwardmove = ((((vLeft[1] * v19) + v21) + (vLeft[2] * v20)) * 50.0)       + forwardmove;
		sidemove    = ((((v19 * vForward[1]) + v23) + (v20 * vForward[2])) * 50.0) + sidemove;
		
		//PrintToServer("forwardmove %f sidemove %f", v24, v25);
	}
}*/

//There's a skill check in here too right at the start, but we don't care about it
//if ( actor + 2411) > 1 ) // m_iSkill 
stock bool AvoidBumpingEnemies(int actor, float[3] out)
{
	if(TF2_GetPlayerClass(actor) != TFClass_Spy)
		return false;

	int ClosestPlayer = 0;
	float flClosestDistance = 200.0;
	
	TFTeam iTeam = GetEnemyTeam(actor);
	
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (TF2_GetClientTeam(i) != iTeam)
			continue;
		
		// Bumping into stealthed enemies is cool beacuse we can't see them!
		if (IsStealthed(i))
			continue;
		
		// Bumping into disguised enemies is cool.
		if (TF2_IsPlayerInCondition(i, TFCond_Disguised))
			continue;
		
		float flDistance = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(i));
		if (flClosestDistance > flDistance)
		{
			flClosestDistance = flDistance;
			ClosestPlayer = i;
		}
	}
	
	if (ClosestPlayer)
	{
		/*if (IsHindrance(actor, player) == ANSWER_UNDEFINED)
		{
			BotAim(actor).ReleaseForwardButton();
			BotAim(actor).ReleaseLeftButton();
			BotAim(actor).ReleaseRightButton();
			BotAim(actor).ReleaseBackwardButton();

			float vecActorFeet[3]; vecActorFeet = //ILocomotion::GetFeet()
			
			float vecTarget[3]; 
			vecTarget[0] = (vecActorOrigin.x + vecActorFeet.x) - vecPlayerOrigin.x;
			vecTarget[1] = (vecActorOrigin.y + vecActorFeet.y) - vecPlayerOrigin.y;
			vecTarget[2] = (vecActorOrigin.z + vecActorFeet.z) - vecPlayerOrigin.z;
			
			Approach(vecTarget, 1.0);
		}*/
		
		//float vecTarget[3]; 
		SubtractVectors(GetAbsOrigin(actor), GetAbsOrigin(ClosestPlayer), out);
		
		return true;
	}
	
	return false;
}

stock TFTeam GetEnemyTeam(int ent)
{
	TFTeam enemy_team = TF2_GetClientTeam(ent);
	switch(enemy_team)
	{
		case TFTeam_Red:  enemy_team = TFTeam_Blue;
		case TFTeam_Blue: enemy_team = TFTeam_Red;
	}
	
	return enemy_team;
}

stock float Max(float one, float two)
{
	if(one > two)
		return one;
	else if(two > one)
		return two;
		
	return two;
}

stock float Min(float one, float two)
{
	if(one < two)
		return one;
	else if(two < one)
		return two;
		
	return two;
}

stock float[] TF2_GetLookAheadPosition(int client)
{
	int iMaxSegments = TF2_IsPlayerInCondition(client, TFCond_Charging) ? 1 : 5;
	int iSegmentsLookedAt = 0;
	
	float vecOut[3];
	for (int i = 0; i < iMaxSegments; i++)
	{
		float vecSegment[3];
		
		bool bSegMentExists = PF_GetFutureSegment(client, i, vecSegment);
		if(bSegMentExists)
		{
			iSegmentsLookedAt++;
			
			AddVectors(vecSegment, vecOut, vecOut);
		}
	}
	
	if(iSegmentsLookedAt > 0)
	{
		vecOut[0] /= iSegmentsLookedAt;
		vecOut[1] /= iSegmentsLookedAt;
		vecOut[2] /= iSegmentsLookedAt;
		
		vecOut[2] += 50.0;
	}
	
	return vecOut;
}

stock int Entity_GetClosestClient(int iEnt)
{
	float flPos1[3];
	GetEntPropVector(iEnt, Prop_Send, "m_vecOrigin", flPos1);
	
	int iBestTarget = -1;
	float flBestLength = 50000.0;
	
	for(int i = 1; i <= MaxClients; i++)
	{
		if(i == iEnt)
			continue;
		
		if(!IsClientInGame(i))
			continue;
		
		if(!IsPlayerAlive(i))
			continue;
		
		if(IsInvulnerable(i))
			continue;
			
		//Always ignore Sentry Busters
		if(TF2_IsBuster(i))
			continue;
			
		if(!Entity_Cansee(iEnt, i))
			continue;
			
		if(GetClientTeam(i) != GetClientTeam(iEnt))
		{
			float flPos2[3];
			GetClientEyePosition(i, flPos2);
			
			float flDistance = GetVectorDistance(flPos1, flPos2);

			if(flDistance < flBestLength)
			{
				iBestTarget = i;
				flBestLength = flDistance;
			}
		}
	}
	
	if(iBestTarget > 0 && iBestTarget <= MaxClients && IsClientInGame(iBestTarget))
	{
		return iBestTarget;
	}
	
	return iBestTarget;
}

stock bool Entity_Cansee(int iLooker, int iTarget)
{
	if(TF2_GetClientTeam(iTarget) != GetEnemyTeam(iLooker))
		return false;
		
	if(!ShouldSpyTransmit(iTarget))
		return false;
		
	TR_TraceRayFilter(GetEyePosition(iLooker), WorldSpaceCenter(iTarget), MASK_SHOT|CONTENTS_GRATE, RayType_EndPoint, TraceFilterSelf, iLooker);
	if(TR_DidHit() && TR_GetEntityIndex() == iTarget)
	{
		return true;
	}
	
	return false;
}

stock bool ShouldSpyTransmit(int client)
{
	// Players who are burning/jarated/bleeding, or who are cloaked and bump into something, are not ignored
	if(TF2_IsPlayerInCondition(client, TFCond_CloakFlicker)
	|| TF2_IsPlayerInCondition(client, TFCond_Bleeding)	
	|| TF2_IsPlayerInCondition(client, TFCond_Jarated)
	|| TF2_IsPlayerInCondition(client, TFCond_Milked)
	|| TF2_IsPlayerInCondition(client, TFCond_OnFire)) {
		return true;
	}
	
	// Spies are only ignored when more than 75% cloaked
	if(IsStealthed(client)) {
		return (GetPercentInvisible(client) <= 0.75);
	}
	
	// Spies who are not fully disguised are not ignored
	if(!TF2_IsPlayerInCondition(client, TFCond_Disguised) 
	 || TF2_IsPlayerInCondition(client, TFCond_Disguising)) {
		return true;
	}
	
	return false;
}

stock bool IsStealthed(int client)
{
	if(TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		return true;
	
	if(TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;
		
	return TF2_IsPlayerInCondition(client, TFCond_StealthedUserBuffFade);
}

stock void DisguiseAsMemberOfEnemyTeam(int client)
{
	TF2_DisguisePlayer(client, GetEnemyTeam(client), view_as<TFClassType>(GetRandomInt(1, 9)));
}
/*
stock bool IsAnyEnemySentryAbleToAttackMe(int client)
{
	int v5; // edi@6
	int v6; // eax@9
	int sentry; // ebx@9
	char v8; // al@23
	char v9; // [sp+Fh] [bp-19h]@2
	
	if ( TF2_IsPlayerInCondition(client, TFCond_Disguised) || (TF2_IsPlayerInCondition(client, TFCond_Disguising) || (v9 = IsStealthed(client)) != 0 )
	{
		v9 = 0;
	}
	else
	{
		v5 = 0;
		if ( dword_18DFB8C > 0 )
		{
			while ( 1 )
			{
				v6 = *(_DWORD *)(IBaseObjectAutoList::m_IBaseObjectAutoListAutoList + 4 * v5);
				sentry = v6 - 2148;
				
				if ( !v6 )
					sentry = 0;
				
				if ( CBaseObject::ObjectType((CBaseObject *)sentry) == 2
					&& !(unsigned __int8)CBaseObject::HasSapper((CBaseObject *)sentry)
					&& !(unsigned __int8)CBaseObject::IsPlasmaDisabled((CBaseObject *)sentry)
					&& !*(_BYTE *)(sentry + 2568)
					&& !*(_BYTE *)(sentry + 2584)
					&& !*(_BYTE *)(sentry + 2570) )
				{
					if ( GetVectorDistance(GetAbsOrigin(sentry), GetAbsOrigin(client), true) <= 1210000.0 && IsThreatAimingTowardMe(this, sentry, 0.95) )
					{
						//CBaseCombatCharacter::IsLineOfSightClear(CBaseEntity*, CBaseCombatCharacter::LineOfSightCheckType)
						if ( IsLineOfSightClear(this, sentry, 1) )
							break;
					}
				}
				if ( dword_18DFB8C <= ++v5 )
				return (unsigned __int8)v9;
			}
			v9 = v8;
		}
	}
	return (unsigned __int8)v9;
}
*/
stock bool UTIL_IsVisibleToTeam(float vecPos[3], int team)
{
    for (int i = 1; i <= MaxClients; i++)
    {
        if(!IsClientInGame(i))
            continue;
            
        if(!IsPlayerAlive(i))
            continue;
            
        if(GetClientTeam(i) != team)
            continue;
        
        if(!IsLineOfFireClear(vecPos, GetEyePosition(i)))
            continue;
            
        return true;
    }
    
    return false;
}

stock bool IsThreatAimingTowardMe(int entity, int looker, float flDot)
{
	float vecToLooker[3];
	MakeVectorFromPoints(GetAbsOrigin(entity), GetAbsOrigin(looker), vecToLooker);
	
	float flLength = GetVectorLength(vecToLooker, true);
	
	NormalizeVector(vecToLooker, vecToLooker);
	
	float vForward[3];
	if ( IsValidClientIndex(looker) )
	{
		EyeVectors(entity, vForward, NULL_VECTOR, NULL_VECTOR);
	}
	else
	{
		if ( !HasEntProp(looker, Prop_Send, "m_hEnemy") || (v10 * flLength) >= 1100.0 )
			return false;
		
		//CObjectSentrygun::GetTurretAngles()
		AngleVectors(GetTurretAngles(CObjectSentryGun), vForward);
	}
	
	if ( GetVectorDotProduct(vForward, vecToLooker) > flDot )
		return true;
		
	return false;
}

stock char[] GetClassName(int entity)
{
	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	return class;
}

stock float GetPercentInvisible(int client)
{
	int offset = FindSendPropInfo("CTFPlayer", "m_flInvisChangeCompleteTime") - 8;
	return GetEntDataFloat(client, offset);
}

public bool TraceFilterSelf(int entity, int contentsMask, any iExclude)
{
	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	
	if(StrEqual(class, "player"))
	{
		if(GetClientTeam(entity) == GetClientTeam(iExclude))
		{
			return false;
		}
	}
	else if(StrEqual(class, "entity_medigun_shield"))
	{
		if(GetEntProp(entity, Prop_Send, "m_iTeamNum") == GetClientTeam(iExclude))
		{
			return false;
		}
	}
	else if(StrEqual(class, "func_respawnroomvisualizer"))
	{
		return false;
	}
	else if(StrContains(class, "tf_projectile_", false) != -1)
	{
		return false;
	}
	else if(StrContains(class, "obj_", false) != -1)
	{
		return false;
	}
	else if(StrEqual(class, "entity_revive_marker"))
	{
		return false;
	}
	
	return !(entity == iExclude);
}

stock bool IsValidClientIndex(int client)
{
	if(client > 0 && client <= MaxClients && IsClientInGame(client))
		return true;
		
	return false;
}

stock bool TF2_IsMvM()
{
	return view_as<bool>(GameRules_GetProp("m_bPlayingMannVsMachine"));
}

stock bool TF2_IsBuster(int client)
{
	char model[PLATFORM_MAX_PATH];
	GetClientModel(client, model, PLATFORM_MAX_PATH);
	return StrEqual(model, "models/bots/demo/bot_sentry_buster.mdl");
}

stock bool IsMiniBoss(int client)
{
	return !!GetEntProp(client, Prop_Send, "m_bIsMiniBoss");
}

stock int GetTeamNumber(int entity)
{
	return GetEntProp(entity, Prop_Send, "m_iTeamNum");
}

stock void EquipWeaponSlot(int client, int slot)
{
	int iWeapon = GetPlayerWeaponSlot(client, slot);
	if(IsValidEntity(iWeapon))
		EquipWeapon(client, iWeapon);
}

stock void EquipWeapon(int client, int weapon)
{
	char class[64];
	GetEntityClassname(weapon, class, sizeof(class));

	FakeClientCommand(client, "use %s", class);
//	SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", weapon);
}

bool IsStandingAtUpgradeStation(int client)
{
	return !!GetEntProp(client, Prop_Send, "m_bInUpgradeZone");
}

public float clamp(float a, float b, float c) { return (a > c ? c : (a < b ? b : a)); }

char g_strHealthAndAmmoEntities[][] = 
{
	"func_regenerate",
	"item_ammopack*",
	"item_health*",
	"obj_dispenser",
	"tf_ammo_pack"
}

void ComputeHealthAndAmmoVectors(int client, JSONArray array, float max_range)
{
	for (int i = 0; i < sizeof(g_strHealthAndAmmoEntities); i++)
	{
		int ammo = -1;
		while ((ammo = FindEntityByClassname(ammo, g_strHealthAndAmmoEntities[i])) != -1)
		{
			if(GetTeamNumber(ammo) == view_as<int>(GetEnemyTeam(client)))
				continue;
		
			if(GetVectorDistance(WorldSpaceCenter(client), WorldSpaceCenter(ammo)) > max_range)
				continue;
			
			
			if (HasEntProp(ammo, Prop_Send, "m_bBuilding"))
			{
				//Can't get anything from still building buildings.
				if(GetEntProp(ammo, Prop_Send, "m_bBuilding"))
				{
					continue;
				}
				
				if(TF2_GetObjectType(ammo) == TFObject_Dispenser)
				{
					//Skip empty dispenser.
					if(GetEntProp(ammo, Prop_Send, "m_iAmmoMetal") <= 0)
					{
						continue;
					}
				}
			}
			
			float length;
			if(!PF_IsPathToVectorPossible(client, WorldSpaceCenter(ammo), length))
				continue;
			
			if(length < max_range)
			{
				JSONObject entity = new JSONObject();
				entity.SetFloat("path_length", length);
				entity.SetInt("entity_index", ammo);
				
				array.Push(entity);
				
				delete entity;
			}
		}
	}
}

stock Address Transpose(Address pAddr, int iOffset)		
{		
    return view_as<Address>(view_as<int>(pAddr) + iOffset);		
}
stock int Dereference(Address pAddr, int iOffset = 0)		
{		
    if(pAddr == Address_Null)		
    {
        return -1;		
    }
    
    return ReadInt(Transpose(pAddr, iOffset));		
} 		
stock int ReadInt(Address pAddr)		
{		
    if(pAddr == Address_Null)		
    {
        return -1;		
    }
    		
    return LoadFromAddress(pAddr, NumberType_Int32);		
}  


//-----------------------------------------------------------------------------
// Purpose: Draw a colored 3D cross of the given size at the given position
//-----------------------------------------------------------------------------
stock void Cross3D(float position[3], float size, int r, int g, int b, float flDuration)
{
	//...
	g_iPathLaserModelIndex = PrecacheModel("materials/sprites/laserbeam.vmt");

	Line( AddVectorFloatOnAxis(position, size, 0.0, 0.0), SubtractVectorFloatOnAxis(position, size, 0.0, 0.0), r, g, b, flDuration );
	Line( AddVectorFloatOnAxis(position, 0.0, size, 0.0), SubtractVectorFloatOnAxis(position, 0.0, size, 0.0), r, g, b, flDuration );
	Line( AddVectorFloatOnAxis(position, 0.0, 0.0, size), SubtractVectorFloatOnAxis(position, 0.0, 0.0, size), r, g, b, flDuration );
}

//-----------------------------------------------------------------------------
// Purpose: Draws a line from one position to another
//-----------------------------------------------------------------------------
stock void Line(float origin[3], float target[3], int r, int g, int b, float duration)
{
	int color[4];
	color[0] = r;
	color[1] = g;
	color[2] = b;
	color[3] = 255;

	TE_SetupBeamPoints(origin, target, g_iPathLaserModelIndex, g_iPathLaserModelIndex, 0, 0, duration, 0.5, 0.5, 5, 5.0, color, 30);
	TE_SendToAllInRange(origin, RangeType_Visibility);
}

stock float[] AddVectorFloatOnAxis(float vector[3], float x = 0.0, float y = 0.0, float z = 0.0)
{
	float v[3]; v = vector;
	
	v[0] += x;
	v[1] += y;
	v[2] += z;

	return v;
}

stock float[] SubtractVectorFloatOnAxis(float vector[3], float x = 0.0, float y = 0.0, float z = 0.0)
{
	float v[3]; v = vector;
	
	v[0] -= x;
	v[1] -= y;
	v[2] -= z;

	return v;
}
