/* reverse engineering by sigsegv
 * based on TF2 version 20151007a
 * server/tf/bot/behavior/sniper/tf_bot_sniper_lurk.cpp
 * used in MvM: TODO
 */

//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/sniper/tf_bot_sniper_lurk.h
//https://github.com/sigsegv-mvm/mvm-reversed/blob/master/server/tf/bot/behavior/sniper/tf_bot_sniper_lurk.cpp

// How long a Sniper bot will wait without seeing an enemy before picking a new spot
const float tf_bot_sniper_patience_duration = 10.0;
// How long a Sniper bot will keep toward at a target it just lost sight of
const float tf_bot_sniper_target_linger_duration = 2.0;

// If set, Snipers will stop on their way to their preferred lurking spot to snipe at opportunistic targets
const bool tf_bot_sniper_allow_opportunistic = true;
// If set, Snipers in MvM mode target the victim that has the highest DPS
const bool tf_mvm_bot_sniper_target_by_dps   = true;

float m_ctPatience[MAXPLAYERS + 1];            
float m_vecHome[MAXPLAYERS + 1][3]; 

bool m_bHasHome[MAXPLAYERS + 1];                     
bool m_bNearHome[MAXPLAYERS + 1];                    
bool m_bOpportunistic[MAXPLAYERS + 1];               

ArrayList m_Hints[MAXPLAYERS + 1];
int m_hHint[MAXPLAYERS + 1]; //EHandle / EntRef

bool CTFBotSniperLurk_OnStart(int actor)
{
	FakeClientCommand(actor, "tournament_player_readystate 1");

	if(m_Hints[actor] != null)
		delete m_Hints[actor];
		
	m_Hints[actor] = new ArrayList();

	m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.9, 1.1) * tf_bot_sniper_patience_duration);
	
	m_vecHome[actor] = GetAbsOrigin(actor);
	m_bHasHome[actor] = false;
	m_bNearHome[actor] = false;
	m_bOpportunistic[actor] = tf_bot_sniper_allow_opportunistic;
	
	int ent = -1;
	while((ent = FindEntityByClassname(ent, "func_tfbot_hint")) != -1)
	{
		int iHintType = GetEntProp(ent, Prop_Data, "m_hint");
		if (iHintType == 0) 
		{
			m_Hints[actor].Push(EntIndexToEntRef(ent));
			
			if (actor == GetOwnerEntity(ent))
			{
				SetOwnerEntity(ent, INVALID_ENT_REFERENCE);
			}
		}
	}
	
	m_hHint[actor] = INVALID_ENT_REFERENCE;
	
	return true;
}

bool CTFBotSniperLurk_Update(int actor)
{
//	actor->AccumulateSniperSpots();
	
	PF_SetGoalVector(actor, m_vecHome[actor]);
	g_bPath[actor] = true;
	
	if (!m_bHasHome[actor]) {
		CTFBotSniperLurk_FindNewHome(actor);
	}
	
	//Don't constantly switch target.
	if (m_hAimTarget[actor] != -1 && IsLineOfFireClear(GetEyePosition(actor), GetEyePosition(m_hAimTarget[actor])) && !IsInvulnerable(m_hAimTarget[actor])) 
		m_hAimTarget[actor] = m_hAimTarget[actor];
	else
		m_hAimTarget[actor] = Entity_GetClosestClient(actor);
	
	if (m_hAimTarget[actor] != -1 && IsLineOfFireClear(GetEyePosition(actor), GetEyePosition(m_hAimTarget[actor])))
	{
		BotAim(actor).AimHeadTowardsEntity(m_hAimTarget[actor], CRITICAL, 0.3, "Looking at visible threat");
	
		if (m_bOpportunistic[actor]) 
		{
			EquipWeaponSlot(actor, TFWeaponSlot_Primary);
			
			m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.9, 1.1) * tf_bot_sniper_patience_duration);
			
			if (!m_bHasHome[actor]) 
			{
				m_vecHome[actor] = GetAbsOrigin(actor);
			}
		} 
		else
		{
			if(!m_bNearHome[actor])
				EquipWeaponSlot(actor, TFWeaponSlot_Secondary);
			else
				EquipWeaponSlot(actor, TFWeaponSlot_Primary);
		}		
	}
	
	float subtracted[3];
	SubtractVectors(GetAbsOrigin(actor), m_vecHome[actor], subtracted);
	float dst_from_home = GetVectorLength(subtracted);
	
	m_bNearHome[actor] = (dst_from_home < 25.0);
	
	if(m_bNearHome[actor])
	{
		g_bPath[actor] = false;
	}
	
	if (dst_from_home < 25.0) 
	{
		m_bOpportunistic[actor] = tf_bot_sniper_allow_opportunistic;
		
		if (m_ctPatience[actor] < GetGameTime())
		{
			if (CTFBotSniperLurk_FindNewHome(actor)) 
			{
				FakeClientCommand(actor, "voicemenu 2 5");
				
				m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.9, 1.1) * tf_bot_sniper_patience_duration);
			} 
			else 
			{
				m_ctPatience[actor] = GetGameTime() + 1.0;
			}
		}
	} 
	else 
	{
		m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.9, 1.1) * tf_bot_sniper_patience_duration);
	}
	
	return true;
}

void CTFBotSniperLurk_OnEnd(int actor)
{
	//if (TF2_IsPlayerInCondition(actor, TFCond_Zoomed)) 
	//{
	//	BotAim(actor).PressAltFireButton();
	//}
	
	if (EntRefToEntIndex(m_hHint[actor]) != INVALID_ENT_REFERENCE) 
	{
		SetOwnerEntity(EntRefToEntIndex(m_hHint[actor]), INVALID_ENT_REFERENCE);
		
		PrintToServer("%3.2f: %N: Releasing hint.\n", GetGameTime(), actor);
	}
}
/*
const CKnownEntity CTFBotSniperLurk_SelectMoreDangerousThreat(const INextBot *nextbot, const CBaseCombatCharacter *them, const CKnownEntity *threat1, const CKnownEntity *threat2) const
{
	if (TFGameRules()->IsMannVsMachineMode() && tf_mvm_bot_sniper_target_by_dps.GetBool()) {
		CTFBot *actor = ToTFBot(nextbot->GetEntity());
		
		if (!threat1->IsVisibleRecently() && threat2->IsVisibleRecently()) {
			return threat2;
		}
		if (threat1->IsVisibleRecently() && !threat2->IsVisibleRecently()) {
			return threat1;
		}
		
		CTFPlayer *player1 = ToTFPlayer(threat1->GetEntity());
		CTFPlayer *player2 = ToTFPlayer(threat2->GetEntity());
		if (player1 == nullptr || player2 == nullptr) {
			return nullptr;
		}
		
		float dsqr1 = actor->GetRangeSquaredTo(player1);
		float dsqr2 = actor->GetRangeSquaredTo(player2);
		
		if ((actor->m_nRestrict & CTFBot::WeaponRestriction::MELEEONLY) == 0) {
			if (dsqr1 >= Square(500.0f) && dsqr2 < Square(500.0f)) {
				return threat2;
			}
			if (dsqr1 < Square(500.0f) && dsqr2 >= Square(500.0f)) {
				return threat1;
			}
			
			// TODO: CTFPlayer+0x2930
			// (prefer threat with higher value, presumably this is a DPS figure)
		}
		
		if (dsqr2 > dsqr1) {
			return threat1;
		}
		return threat2;
	}
	
	return nullptr;
}

*/
int CTFBotSniperLurk_FindHint(int actor)
{
	if(m_Hints[actor] == null || m_Hints[actor].Length <= 0)
	{
		return INVALID_ENT_REFERENCE;
	}

	int iBomb = TF2_FindBombNearestToHatch();
	if(iBomb <= 0)
	{
		//Explore random hints while bomb is not in play.
		return EntRefToEntIndex(m_Hints[actor].Get(GetRandomInt(0, m_Hints[actor].Length - 1)));
	}
	
	ArrayList aPossibleHints = new ArrayList();
	
	int iBestHint = INVALID_ENT_REFERENCE;
	float flBestDistance = 999999999.0;
	
	for (int i = 0; i < m_Hints[actor].Length; i++)
	{
		int iHint = EntRefToEntIndex(m_Hints[actor].Get(i));
		if(iHint == INVALID_ENT_REFERENCE || !!GetEntProp(iHint, Prop_Data, "m_isDisabled") == true)
			continue;
		
		if(!IsLineOfFireClear(WorldSpaceCenter(iBomb), WorldSpaceCenter(iHint)))
			continue;
		
		aPossibleHints.Push(iHint);
		
		//For fallback.
		float flDistance = GetVectorDistance(WorldSpaceCenter(iHint), WorldSpaceCenter(iBomb));
		if(flDistance <= flBestDistance)
		{
			flBestDistance = flDistance;
			iBestHint = iHint;
		}
	}
	
	if(aPossibleHints.Length > 0)
	{
		iBestHint = aPossibleHints.Get(GetRandomInt(0, aPossibleHints.Length - 1));
	}
	
	delete aPossibleHints;
	return iBestHint;
}

bool CTFBotSniperLurk_FindNewHome(int actor)
{
	m_bHasHome[actor] = false;
	if(m_Hints[actor] == null || m_Hints[actor].Length <= 0)
		return false;
	
	int iHint = CTFBotSniperLurk_FindHint(actor);
	if(iHint == INVALID_ENT_REFERENCE)
		return false;

	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(iHint), true);
	if(area == NavArea_Null)
		return false;
	
	area.GetRandomPoint(m_vecHome[actor]);
	m_bHasHome[actor] = true;
	
	return true;
}