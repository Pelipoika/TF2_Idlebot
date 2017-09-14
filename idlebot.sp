//Thanks to sigsegv for his reversing work.

#include <sdktools>
#include <sdkhooks>
#include <tf2_stocks>
#include <PathFollower>
#include <PathFollower_Nav>
#include <ripext>

#pragma newdecls required

bool g_bEmulate[MAXPLAYERS + 1];
bool g_bPath[MAXPLAYERS + 1];

float g_flNextUpdate[MAXPLAYERS + 1];
float m_ctUseWeaponAbilities[MAXPLAYERS + 1];

float m_ctVelocityLeft[MAXPLAYERS + 1];
float m_ctVelocityRight[MAXPLAYERS + 1];

//Boat
float g_vecCurrentGoal[MAXPLAYERS + 1][3];
int g_iAdditionalButtons[MAXPLAYERS + 1];
float g_flAdditionalVelocity[MAXPLAYERS + 1][3];

enum RouteType
{
	DEFAULT_ROUTE,
	FASTEST_ROUTE,
	SAFEST_ROUTE,
	RETREAT_ROUTE,
};

RouteType m_iRouteType[MAXPLAYERS + 1];

int g_iCurrentAction[MAXPLAYERS + 1];
bool g_bStartedAction[MAXPLAYERS + 1];

//Idle
float g_flLastInput[MAXPLAYERS + 1];
bool g_bIdle[MAXPLAYERS + 1];

char g_szBotModels[][] =
{
	"models/bots/scout/bot_scout.mdl",
	"models/bots/sniper/bot_sniper.mdl",
	"models/bots/soldier/bot_soldier.mdl",
	"models/bots/demo/bot_demo.mdl",
	"models/bots/medic/bot_medic.mdl",
	"models/bots/heavy/bot_heavy.mdl",
	"models/bots/pyro/bot_pyro.mdl",
	"models/bots/spy/bot_spy.mdl",
	"models/bots/engineer/bot_engineer.mdl"
};

#include <actions/utility>
#include <actions/CTFBotAim>

#define ACTION_IDLE 0
#define ACTION_ATTACK 1
#define ACTION_MARK_GIANT 2
#define ACTION_COLLECT_MONEY 3
#define ACTION_GOTO_UPGRADE 4
#define ACTION_UPGRADE 5
#define ACTION_GET_AMMO 6
#define ACTION_MOVE_TO_FRONT 7
#define ACTION_GET_HEALTH 8
#define ACTION_USE_ITEM 9
//#define ACTION_MEDIC_HEAL 8

#include <actions/CTFBotAttack>
#include <actions/CTFBotMarkGiant>
#include <actions/CTFBotCollectMoney>
#include <actions/CTFBotGoToUpgradeStation>
#include <actions/CTFBotUpgrade>
#include <actions/CTFBotGetAmmo>
#include <actions/CTFBotGetHealth>
#include <actions/CTFBotMoveToFront>
#include <actions/CTFBotUseItem>
//#include <actions/CTFBotMedicHeal>

Handle g_hHudInfo;

public Plugin myinfo = 
{
	name = "[TF2] MvM AFK Bot",
	author = "Pelipoika",
	description = "",
	version = "1.0",
	url = "http://www.sourcemod.net/plugins.php?author=Pelipoika&search=1"
};

public void OnPluginStart()
{
	RegAdminCmd("sm_defender", Command_Robot, ADMFLAG_ROOT);
	
	for (int i = 1; i <= MaxClients; i++)
		OnClientPutInServer(i);
	
	g_hHudInfo = CreateHudSynchronizer();
	
	InitGamedata();
	InitTFBotAim();
}

public void OnClientPutInServer(int client)
{
	g_bEmulate[client] = false;
	g_iAdditionalButtons[client] = 0;
	g_vecCurrentGoal[client] = NULL_VECTOR;
	g_bPath[client] = true;
	g_iCurrentAction[client] = 5;
	g_bStartedAction[client] = false;
	g_flNextUpdate[client] = GetGameTime();
	
	g_flLastInput[client] = GetGameTime();
	g_bIdle[client] = false;
	
	m_ctVelocityLeft[client] = 0.0;
	m_ctVelocityRight[client] = 0.0;
	m_ctUseWeaponAbilities[client] = 0.0;
	
	m_iRouteType[client] = DEFAULT_ROUTE;
	
	CTFBotMarkGiant_OnEnd(client);
	CTFBotCollectMoney_OnEnd(client);
//	CTFBotPurchaseUpgrades_OnEnd(client);
	CTFBotGoToUpgradeStation_OnEnd(client);
	CTFBotAttack_OnEnd(client);
	CTFBotGetAmmo_OnEnd(client);
	CTFBotGetHealth_OnEnd(client);
	CTFBotMoveToFront_OnEnd(client);
	CTFBotUseItem_OnEnd(client);
}

public Action Command_Robot(int client, int args)
{
	if(client > 0 && client <= MaxClients && IsClientInGame(client))
	{
		if(TF2_IsMvM() && TF2_GetClientTeam(client) != TFTeam_Red)
		{
			ReplyToCommand(client, "For RED team only");
			return Plugin_Handled;
		}
	
		SetDefender(client, !g_bEmulate[client]);
	}
	
	return Plugin_Handled;
}

stock bool SetDefender(int client, bool bEnabled)
{
	if(TF2_GetClientTeam(client) == TFTeam_Unassigned)
	{
		FakeClientCommand(client, "autoteam");
		FakeClientCommand(client, "joinclass random");
	}

	if(!bEnabled && g_bEmulate[client])
	{
		SetVariantString("");
		AcceptEntityInput(client, "SetCustomModel");
		SetEntProp(client, Prop_Send, "m_bUseClassAnimations", 1);
		
		PrintCenterText(client, "");
		
		SendConVarValue(client, FindConVar("sv_client_predict"), "-1");
		SetEntProp(client, Prop_Data, "m_bLagCompensation", true);
		SetEntProp(client, Prop_Data, "m_bPredictWeapons", true);
		
		g_bEmulate[client] = false;
		
		if(PF_Exists(client))
			PF_StopPathing(client);
	}
	else if(!g_bEmulate[client])
	{
		SetVariantString(g_szBotModels[view_as<int>(TF2_GetPlayerClass(client)) - 1]);
		AcceptEntityInput(client, "SetCustomModel");
		SetEntProp(client, Prop_Send, "m_bUseClassAnimations", 1);
	
		SendConVarValue(client, FindConVar("sv_client_predict"), "0");
		SetEntProp(client, Prop_Data, "m_bLagCompensation", false);
		SetEntProp(client, Prop_Data, "m_bPredictWeapons", false);
		
		if(!PF_Exists(client))
		{
			PF_Create(client, 14.0, 50.0, 1000.0, 0.6, MASK_PLAYERSOLID, 200.0, 0.2);
			PF_EnableCallback(client, PFCB_Approach, PluginBot_Approach);
			PF_EnableCallback(client, PFCB_ClimbUpToLedge, PluginBot_Jump);
			PF_EnableCallback(client, PFCB_IsEntityTraversable, PluginBot_IsEntityTraversable);
			PF_EnableCallback(client, PFCB_GetPathCost, PluginBot_PathCost);
		}
		
		BotAim(client).Reset();
		g_bEmulate[client] = true;
	}
}

float g_flCollectSentries;
ArrayList g_hSentries;

RoundState g_RoundState;
public void OnGameFrame()
{
	g_RoundState = GameRules_GetRoundState();
	
	if(g_flCollectSentries < GetGameTime())
	{
		g_flCollectSentries = GetGameTime() + 4.0;
		
		if(g_hSentries == null)
			g_hSentries = new ArrayList();
	
		g_hSentries.Clear();
		
		int iEnt = -1;
		while ((iEnt = FindEntityByClassname(iEnt, "obj_sentrygun")) != -1)
		{
			PF_UpdateLastKnownArea(iEnt);
			
			g_hSentries.Push(EntIndexToEntRef(iEnt));
		}
	}
}

public Action OnPlayerRunCmd(int client, int &iButtons, int &iImpulse, float fVel[3], float fAng[3], int &iWeapon)
{
	if(IsFakeClient(client) || !IsClientConnected(client))
		return Plugin_Continue;
	
	if(iButtons != 0 || iImpulse != 0 || GetClientTeam(client) == 1)
		g_flLastInput[client] = GetGameTime();

//	PrintToServer("OnPlayerRunCmd %N team %i", client, GetTeamNumber(client));

	float flLastInput   = (GetGameTime() - g_flLastInput[client]);
	float flMaxIdleTime = (FindConVar("mp_idlemaxtime").FloatValue * 60.0);
	
	if(flLastInput > (flMaxIdleTime / 2))
	{
		if(g_bIdle[client])
			PrintCenterText(client, "The server plays for you while you are away. Press any key to take control");
		else
			PrintCenterText(client, "You seem to be away... (%.0fs / %.0fs)\n%s", flLastInput, flMaxIdleTime, (flLastInput > (flMaxIdleTime / 1.2)) ? "The server will take over soon..." : "");
	}
	
	if (flLastInput > flMaxIdleTime)
	{
		if(!g_bIdle[client] && !g_bEmulate[client])
		{
			g_bIdle[client] = true;
			SetDefender(client, true);
		}
	}
	else
	{
		if(g_bIdle[client])
		{
			g_bIdle[client] = false;
			SetDefender(client, false);
		}
	}
	
	if(!IsPlayerAlive(client))
		return Plugin_Continue;
	
	if(!g_bEmulate[client] || !PF_Exists(client))
		return Plugin_Continue;
	
	bool bChanged = false;
	if(g_iAdditionalButtons[client] != 0)
	{
		iButtons |= g_iAdditionalButtons[client];
		g_iAdditionalButtons[client] = 0;
		bChanged = true;
	}
	
	//Always crouch jump
	if(GetEntProp(client, Prop_Send, "m_bJumping"))
	{
		iButtons |= IN_DUCK
		bChanged = true;
	}
	
	if(m_ctReload[client]  > GetGameTime()) { iButtons |= IN_RELOAD;  bChanged = true; }
	if(m_ctFire[client]    > GetGameTime()) { iButtons |= IN_ATTACK;  bChanged = true; }
	if(m_ctAltFire[client] > GetGameTime()) { iButtons |= IN_ATTACK2; bChanged = true; }
	
//	EquipRequiredWeapon();
//	UpdateLookingAroundForEnemies();
	BotAim(client).Upkeep();
	BotAim(client).FireWeaponAtEnemy();
	Dodge(client);
	
	SetHudTextParams(0.1, 0.1, 0.1, 255, 255, 200, 255, 0, 0.0, 0.0, 0.0);
	ShowSyncHudText(client, g_hHudInfo, "%s\nRouteType %s\nIsHeadAimingOnTarget %i\nIsHeadSteady %i\nGetHeadSteadyDuration %.1f", CurrentActionToName(g_iCurrentAction[client]), 
																																CurrentRouteTypeToName(client),
																																BotAim(client).IsHeadAimingOnTarget(), 
																																BotAim(client).IsHeadSteady(),
																																BotAim(client).GetHeadSteadyDuration());																					
	
	RunCurrentAction(client);
	
	if(g_RoundState == RoundState_BetweenRounds)
	{
		if(CTFBotCollectMoney_IsPossible(client))
		{
			ChangeAction(client, ACTION_COLLECT_MONEY);
			m_iRouteType[client] = FASTEST_ROUTE;
		}
		else if(!IsStandingAtUpgradeStation(client) && g_iCurrentAction[client] != ACTION_MOVE_TO_FRONT && !GameRules_GetProp("m_bPlayerReady", 1, client))
		{
			ChangeAction(client, ACTION_GOTO_UPGRADE);
			m_iRouteType[client] = DEFAULT_ROUTE;
		}
	}
	else if(g_RoundState == RoundState_RoundRunning)
	{
		OpportunisticallyUseWeaponAbilities(client);
	
		bool low_health = false;
		
		float health_ratio = view_as<float>(GetClientHealth(client)) / view_as<float>(GetMaxHealth(client));
		
		if ((GetTimeSinceWeaponFired(client) > 2.0 || TF2_GetPlayerClass(client) == TFClass_Sniper)	&& health_ratio < FindConVar("tf_bot_health_critical_ratio").FloatValue){
			low_health = true;
		} 
		else if (health_ratio < FindConVar("tf_bot_health_ok_ratio").FloatValue){
			low_health = true;
		}
		
		bool bCanCheck = g_flNextUpdate[client] < GetGameTime();
		if(bCanCheck)
		{
			g_flNextUpdate[client] = GetGameTime() + 1.0;
			
			SetEntProp(client, Prop_Data, "m_bLagCompensation", false);
			SetEntProp(client, Prop_Data, "m_bPredictWeapons", false);
		}
		
		if ((g_iCurrentAction[client] == ACTION_GET_HEALTH || bCanCheck) && low_health && CTFBotGetHealth_IsPossible(client)) 
		{
			ChangeAction(client, ACTION_GET_HEALTH);
			m_iRouteType[client] = SAFEST_ROUTE;
		}
		else if ((g_iCurrentAction[client] == ACTION_GET_AMMO || bCanCheck) && IsAmmoLow(client) && CTFBotGetAmmo_IsPossible(client)) 
		{
			ChangeAction(client, ACTION_GET_AMMO);
			m_iRouteType[client] = SAFEST_ROUTE;
		}
		else
		{
			if(g_iCurrentAction[client] != ACTION_USE_ITEM && bCanCheck)
			{
				if(TF2_GetPlayerClass(client) != TFClass_Scout)
				{
					if(CTFBotAttack_IsPossible(client))
					{
						ChangeAction(client, ACTION_ATTACK);
						m_iRouteType[client] = FASTEST_ROUTE;
					}
					else if(CTFBotCollectMoney_IsPossible(client))
					{
						ChangeAction(client, ACTION_COLLECT_MONEY);
						m_iRouteType[client] = SAFEST_ROUTE;
					}
					else
					{
						ChangeAction(client, ACTION_IDLE);
						m_iRouteType[client] = DEFAULT_ROUTE;
					}
				}
				else
				{
					if(CTFBotCollectMoney_IsPossible(client))
					{
						ChangeAction(client, ACTION_COLLECT_MONEY);
						m_iRouteType[client] = FASTEST_ROUTE;
					}
					else if(CTFBotMarkGiant_IsPossible(client))
					{
						ChangeAction(client, ACTION_MARK_GIANT);	
						m_iRouteType[client] = SAFEST_ROUTE;
					}
					else if(CTFBotAttack_IsPossible(client))
					{
						ChangeAction(client, ACTION_ATTACK);
						m_iRouteType[client] = DEFAULT_ROUTE;
					}
					else
					{
						ChangeAction(client, ACTION_IDLE);
						m_iRouteType[client] = DEFAULT_ROUTE;
					}
				}
			}
		}	
	}
	
	if(g_bPath[client])
	{
		PF_StartPathing(client);
		
		float flDistance = GetVectorDistance(g_vecCurrentGoal[client], WorldSpaceCenter(client));
		if(flDistance > 10.0)
		{
			if(TF2_GetPlayerClass(client) == TFClass_Sniper && IsSniperRifle(client))
			{
				m_iRouteType[client] = SAFEST_ROUTE;
				
				if(!TF2_IsPlayerInCondition(client, TFCond_Slowed))
					TF2_MoveTo(client, g_vecCurrentGoal[client], fVel, fAng);
			}
			else
				TF2_MoveTo(client, g_vecCurrentGoal[client], fVel, fAng);
				
			bChanged = true;
		}
	}
	else
	{
		PF_StopPathing(client);
	}
	
	if(m_ctVelocityLeft[client] > GetGameTime()) {
		fVel[1] += g_flAdditionalVelocity[client][1];
		bChanged = true;
	}	
	if(m_ctVelocityRight[client] > GetGameTime()) {
		fVel[1] += g_flAdditionalVelocity[client][1];
		bChanged = true;
	}
	
	return bChanged ? Plugin_Changed : Plugin_Continue;
}

stock bool OpportunisticallyUseWeaponAbilities(int client)
{
	if (!(m_ctUseWeaponAbilities[client] < GetGameTime())) {
		return false;
	}
	
	m_ctUseWeaponAbilities[client] = GetGameTime() + GetRandomFloat(0.1, 0.2);
	
	if (TF2_GetPlayerClass(client) == TFClass_DemoMan && HasDemoShieldEquipped(client)) 
	{
		float eye_vec[3];
		EyeVectors(client, eye_vec);
		
		float absOrigin[3]; absOrigin = GetAbsOrigin(client);
		absOrigin[0] += (100.0 * eye_vec[0]);
		absOrigin[1] += (100.0 * eye_vec[1]);
		absOrigin[2] += (100.0 * eye_vec[2]);
		
		float fraction;
		if (PF_IsPotentiallyTraversable(client, GetAbsOrigin(client), absOrigin, IMMEDIATELY, fraction))
		{
			BotAim(client).PressAltFireButton();
		}
	}
	else if(HasParachuteEquipped(client))
	{
		bool burning = TF2_IsPlayerInCondition(client, TFCond_OnFire);
		float health_ratio = view_as<float>(GetClientHealth(client)) / view_as<float>(GetMaxHealth(client));
		
		if (TF2_IsPlayerInCondition(client, TFCond_Parachute)) 
		{
			if (health_ratio >= 0.5 && !burning && !l_is_above_ground(client, 150.0)) 
			{
				g_iAdditionalButtons[client] |= IN_JUMP;
			} 
		} 
		else 
		{
			float velocity[3]; 
			velocity = GetAbsVelocity(client);
		
			if (health_ratio >= 0.5 && !burning && velocity[2] < 0.0 && !l_is_above_ground(client, 300.0)) 
			{
				g_iAdditionalButtons[client] |= IN_JUMP;
			} 
		}
	}
	
	//The Hitmans Heatmaker
	if (IsSniperRifle(client) && TF2_IsPlayerInCondition(client, TFCond_Slowed))
	{
		if(GetEntPropFloat(client, Prop_Send, "m_flRageMeter") >= 0.0 && !GetEntProp(client, Prop_Send, "m_bRageDraining"))
		{
			BotAim(client).PressReloadButton();
			return true;
		}
	}
	
	//Phlogistinator
	if(IsWeapon(client, TF_WEAPON_FLAMETHROWER))
	{
		if(GetEntPropFloat(client, Prop_Send, "m_flRageMeter") >= 100.0 && !GetEntProp(client, Prop_Send, "m_bRageDraining"))
		{
			BotAim(client).PressAltFireButton();
			return true;
		}
	}
	
	if(!CTFBotUseItem_IsPossible(client))
		return false;

	int elementCount = GetEntPropArraySize(client, Prop_Send, "m_hMyWeapons");
	for (int i = 0; i < elementCount; i++)
	{
		int weapon = GetEntPropEnt(client, Prop_Send, "m_hMyWeapons", i);
		if (!IsValidEntity(weapon)) 
			continue;
			
		if (GetWeaponID(weapon) == TF_WEAPON_BUFF_ITEM) 
		{
			if (StrEqual(EntityNetClass(weapon), "CTFBuffItem") && GetEntPropFloat(client, Prop_Send, "m_flRageMeter") >= 100.0)
			{
				ChangeAction(client, ACTION_USE_ITEM);
				return true;
			}
			
			continue;
		}
		
		if (GetWeaponID(weapon) == TF_WEAPON_LUNCHBOX) 
		{
			if (!HasAmmo(weapon) || (TF2_GetPlayerClass(client) == TFClass_Scout && GetEntPropFloat(client, Prop_Send, "m_flEnergyDrinkMeter") < 100.0))
			{
				continue;
			}
			
			ChangeAction(client, ACTION_USE_ITEM);
			return true;
		}
		
	/*	if (GetWeaponID(weapon) == TF_WEAPON_BAT_WOOD && GetAmmoCount(client, TF_AMMO_GRENADES1) > 0) 
		{
			const CKnownEntity *threat = this->GetVisionInterface()->GetPrimaryKnownThreat(false);
			if (threat == nullptr || !threat->IsVisibleRecently()) 
			{
				continue;
			}
			
			this->PressAltFireButton();
		}*/
	}
	
	return false;
}

bool l_is_above_ground(int actor, float min_height)
{
	float from[3]; from = GetAbsOrigin(actor);
	float to[3]; to = GetAbsOrigin(actor);
	to[2] -= min_height;
	
	return !PF_IsPotentiallyTraversable(actor, from, to, IMMEDIATELY, NULL_FLOAT);
}

stock char CurrentRouteTypeToName(int client)
{
	char name[PLATFORM_MAX_PATH];

	//Stop
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

	//Stop
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
	}
	
	return name;
}

//Stop whatever current action we're doing properly, and change to another.
//Return whether or not a new action was started.
stock bool ChangeAction(int client, int new_action)
{
	//Don't allow starting the same function twice.
	if(new_action == g_iCurrentAction[client])
		return false;
	
	PrintToServer("\"%N\" Change Action \"%s\" -> \"%s\"", client, CurrentActionToName(g_iCurrentAction[client]), CurrentActionToName(new_action));
	
	g_bPath[client] = false;
	g_bStartedAction[client] = false;
	
	//Stop
	switch(g_iCurrentAction[client])
	{
		case ACTION_MARK_GIANT:    CTFBotMarkGiant_OnEnd(client);
		case ACTION_COLLECT_MONEY: CTFBotCollectMoney_OnEnd(client);
		case ACTION_UPGRADE:       CTFBotPurchaseUpgrades_OnEnd(client);
		case ACTION_GOTO_UPGRADE:  CTFBotGoToUpgradeStation_OnEnd(client);
		case ACTION_ATTACK:        CTFBotAttack_OnEnd(client);
		case ACTION_GET_AMMO:      CTFBotGetAmmo_OnEnd(client);
		case ACTION_GET_HEALTH:    CTFBotGetHealth_OnEnd(client);
		case ACTION_MOVE_TO_FRONT: CTFBotMoveToFront_OnEnd(client);
		case ACTION_USE_ITEM:      CTFBotUseItem_OnEnd(client);
	}
	
	//Start
	switch(new_action)
	{
		case ACTION_IDLE:          g_bPath[client] = false;
		case ACTION_MARK_GIANT:    g_bStartedAction[client] = CTFBotMarkGiant_OnStart(client);
		case ACTION_COLLECT_MONEY: g_bStartedAction[client] = CTFBotCollectMoney_OnStart(client);
		case ACTION_UPGRADE:       g_bStartedAction[client] = CTFBotPurchaseUpgrades_OnStart(client);
		case ACTION_GOTO_UPGRADE:  g_bStartedAction[client] = CTFBotGoToUpgradeStation_OnStart(client);
		case ACTION_ATTACK:        g_bStartedAction[client] = CTFBotAttack_OnStart(client);
		case ACTION_GET_AMMO:      g_bStartedAction[client] = CTFBotGetAmmo_OnStart(client);
		case ACTION_GET_HEALTH:    g_bStartedAction[client] = CTFBotGetHealth_OnStart(client);
		case ACTION_MOVE_TO_FRONT: g_bStartedAction[client] = CTFBotMoveToFront_OnStart(client);
		case ACTION_USE_ITEM:      g_bStartedAction[client] = CTFBotUseItem_OnStart(client);
	}
	
	//Store
	g_iCurrentAction[client] = new_action;
	
	SetVariantString(g_szBotModels[view_as<int>(TF2_GetPlayerClass(client)) - 1]);
	AcceptEntityInput(client, "SetCustomModel");
	SetEntProp(client, Prop_Send, "m_bUseClassAnimations", 1);

	return g_bStartedAction[client];
}

stock bool RunCurrentAction(int client)
{
	//Update
	switch(g_iCurrentAction[client])
	{
		case ACTION_MARK_GIANT:    g_bStartedAction[client] = CTFBotMarkGiant_Update(client);
		case ACTION_COLLECT_MONEY: g_bStartedAction[client] = CTFBotCollectMoney_Update(client);
		case ACTION_UPGRADE:       g_bStartedAction[client] = CTFBotPurchaseUpgrades_Update(client);
		case ACTION_GOTO_UPGRADE:  g_bStartedAction[client] = CTFBotGoToUpgradeStation_Update(client);
		case ACTION_ATTACK:        g_bStartedAction[client] = CTFBotAttack_Update(client);
		case ACTION_GET_AMMO:      g_bStartedAction[client] = CTFBotGetAmmo_Update(client);
		case ACTION_MOVE_TO_FRONT: g_bStartedAction[client] = CTFBotMoveToFront_Update(client);
		case ACTION_GET_HEALTH:    g_bStartedAction[client] = CTFBotGetHealth_Update(client);
		case ACTION_USE_ITEM:      g_bStartedAction[client] = CTFBotUseItem_Update(client);
	}

	return g_bStartedAction[client];
}

stock void Dodge(int actor)
{
	if (IsInvulnerable(actor) ||
		TF2_IsPlayerInCondition(actor, TFCond_Zoomed)   ||
		TF2_IsPlayerInCondition(actor, TFCond_Taunting))
	{
		return;
	}
	
	if (TF2_GetPlayerClass(actor) == TFClass_Engineer  ||
		TF2_IsPlayerInCondition(actor, TFCond_Disguised)  ||
		TF2_IsPlayerInCondition(actor, TFCond_Disguising) ||
		IsStealthed(actor)) 
	{
		return;
	}
	
	if (IsWeapon(actor, TF_WEAPON_COMPOUND_BOW)) 
	{
		if (GetCurrentCharge(GetActiveWeapon(actor)) != 0.0) 
		{
			return;
		}
	}
	/*else {
		if (!this->IsLineOfFireClear(threat->GetLastKnownPosition())) {
			return;
		}
	}*/
	
	if(g_bPath[actor])
		return;
	
	if(m_hAimTarget[actor] <= 0)
		return;
		
	float eye_vec[3];
	EyeVectors(actor, eye_vec);
	
	float side_dir[3];
	side_dir[0] = -eye_vec[1];
	side_dir[1] = eye_vec[0];
	
	NormalizeVector(side_dir, side_dir);
	
	int random = GetRandomInt(0, 100);
	if (random < 33) 
	{
		float strafe_left[3]; strafe_left = GetAbsOrigin(actor);
		strafe_left[0] += 25.0 * side_dir[0];
		strafe_left[1] += 25.0 * side_dir[1];
		strafe_left[2] += 0.0;
		
		if (!PF_HasPotentialGap(actor, GetAbsOrigin(actor), strafe_left, NULL_FLOAT)) 
		{
			g_flAdditionalVelocity[actor][1] = -500.0;
			m_ctVelocityLeft[actor] = GetGameTime() + 0.5;
		}
	} 
	else if (random > 66) 
	{
		float strafe_right[3]; strafe_right = GetAbsOrigin(actor);
		strafe_right[0] += 25.0 * side_dir[0];
		strafe_right[1] += 25.0 * side_dir[1];
		strafe_right[2] += 0.0;
		
		if (!PF_HasPotentialGap(actor, GetAbsOrigin(actor), strafe_right, NULL_FLOAT)) 
		{
			g_flAdditionalVelocity[actor][1] = 500.0;
			m_ctVelocityRight[actor] = GetGameTime() + 0.5;
		}
	}
}

public void PluginBot_Approach(int bot_entidx, const float vec[3])
{
	g_vecCurrentGoal[bot_entidx] = vec;
	
	if(m_hAimTarget[bot_entidx] <= 0)
		BotAim(bot_entidx).AimHeadTowards(TF2_GetLookAheadPosition(bot_entidx), BORING, 0.3, "Aiming towards our goal");
}

public bool PluginBot_IsEntityTraversable(int bot_entidx, int other_entidx) 
{
	//Traversing "teammates" is okay.
	if(IsValidClientIndex(other_entidx) && GetTeamNumber(bot_entidx) == GetTeamNumber(other_entidx))
		return true;
	
	//Traversing our target should always be possible in order for PF_IsPotentiallyTraversable to work in PredictSubject.
	return (m_hAttackTarget[bot_entidx] == other_entidx) ? true : false; 
}

public void PluginBot_Jump(int bot_entidx, const float vecPos[3], const float dir[2])
{
//	const float watchForClimbRange = 75.0;
//	if (!PF_IsDiscontinuityAhead(bot_entidx, CLIMB_UP, watchForClimbRange))
//	{
	if(GetEntityFlags(bot_entidx) & FL_ONGROUND)
	{
		g_iAdditionalButtons[bot_entidx] |= IN_JUMP;
	}
//	}
}

public float PluginBot_PathCost(int bot_entidx, NavArea area, NavArea from_area, float length)
{
//	PrintToServer("area %x from_area %x length %f", area, from_area, length);
	
	TFTeam botTeam = TF2_GetClientTeam(bot_entidx);

	if ((botTeam == TFTeam_Red  && HasTFAttributes(area, BLUE_SPAWN_ROOM)) ||
		(botTeam == TFTeam_Blue && HasTFAttributes(area, RED_SPAWN_ROOM)))
	{
		if (GameRules_GetRoundState() != RoundState_TeamWin) 
		{
			/* dead end */
			return -1.0;
		}
	}
	
	float multiplier = 1.0;
	float dist;
	
	if (m_iRouteType[bot_entidx] == FASTEST_ROUTE) 
	{
		if (length > 0.0) 
		{
			dist = length;
		} 
		else 
		{
			float center[3];          area.GetCenter(center);
			float fromCenter[3]; from_area.GetCenter(fromCenter);
		
			float subtracted[3]; SubtractVectors(center, fromCenter, subtracted);
		
			dist = GetVectorLength(subtracted);
		}
		
		float delta_z = area.ComputeAdjacentConnectionHeightChange(from_area);
		if(delta_z >= 18.0)
		{
			dist *= 2.0;
		}
		
		return dist + from_area.GetCostSoFar();
	}
	
	if (m_iRouteType[bot_entidx] == DEFAULT_ROUTE) 
	{
		if (!GetEntProp(bot_entidx, Prop_Send, "m_bIsMiniBoss")) 
		{
			/* very similar to CTFBot::TransientlyConsistentRandomValue */
			int seed = RoundToFloor(GetGameTime() * 0.1) + 1;
			seed *= area.GetID();
			seed *= bot_entidx;
			
			/* huge random cost modifier [0, 100] for non-giant bots! */
			multiplier += (Cosine(float(seed)) + 1.0) * 10.0;
		}
	}
	
	if (m_iRouteType[bot_entidx] == SAFEST_ROUTE) 
	{
		if (IsInCombat(area))
		{
			dist *= 4.0 * GetCombatIntensity(area);
		}
		
		if ((botTeam == TFTeam_Red  && HasTFAttributes(area, BLUE_SENTRY)) ||
			(botTeam == TFTeam_Blue && HasTFAttributes(area, RED_SENTRY))) 
		{
			dist *= 5.0;
		}
	}
	
	TFClassType botClass = TF2_GetPlayerClass(bot_entidx);
	
	if(botClass == TFClass_Sniper || botClass == TFClass_Spy)
	{
		int enemy_team = view_as<int>(GetEnemyTeam(bot_entidx));
		
		for (int i = 0; i < g_hSentries.Length; i++)
		{
			int obj = EntRefToEntIndex(g_hSentries.Get(i));
			if(IsValidEntity(obj))
			{
				if(GetTeamNumber(obj) == enemy_team)
				{
					if (area == PF_GetLastKnownArea(obj)) 
					{
						dist *= 10.0;
					}
				}
			}
		}
	
		dist += (dist * 10.0 * area.GetPlayerCount(enemy_team));
	}
	
	float cost = dist * multiplier;
	
/*	if (area.HasAttributes(NAV_MESH_FUNC_COST)) 
	{
		cost *= area->ComputeFuncNavCost(this->m_Actor);
	}*/
	
	return from_area.GetCostSoFar() + cost;
}