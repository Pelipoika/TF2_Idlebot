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

int m_hHint[MAXPLAYERS + 1]; //EHandle / EntRef

bool CTFBotSniperLurk_OnStart(int actor)
{
	FakeClientCommand(actor, "tournament_player_readystate 1");

	m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.9, 1.1) * tf_bot_sniper_patience_duration);
	
	m_vecHome[actor] = GetAbsOrigin(actor);
	m_bHasHome[actor] = false;
	m_bNearHome[actor] = false;
	m_bOpportunistic[actor] = tf_bot_sniper_allow_opportunistic;
	
	int ent = -1;
	while((ent = FindEntityByClassname(ent, "func_tfbot_hint")) != -1)
	{
		if (actor == GetOwnerEntity(ent))
		{
			SetOwnerEntity(ent, INVALID_ENT_REFERENCE);
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
			
			//Lurk a little less time.
			m_ctPatience[actor] = GetGameTime() + (GetRandomFloat(0.5, 1.0) * tf_bot_sniper_patience_duration);
			
			//Saw an enemy so stay where we were when we spotted them.
			m_vecHome[actor] = GetAbsOrigin(actor);
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

/*
	First search for hints visble to bomb and return a random one.
	If none found AND bomb is valid, use closest hint to bomb.
	If bomb IS NOT valid and there are hints return a random hint.
*/
int CTFBotSniperLurk_FindHint()
{
	int iBomb = TF2_FindBombNearestToHatch();
	
	//Every hint available.
	ArrayList aPossibleHints = new ArrayList();
	
	//Every hint with LOS to the Bomb.
	ArrayList aVisibleHints = new ArrayList();
	
	//The closest hint to the bomb that also has LOS.
	int iBestHint = INVALID_ENT_REFERENCE;
	
	float flBestDistance = 999999999.0;
	
	int iHint = -1;
	while((iHint = FindEntityByClassname(iHint, "func_tfbot_hint")) != -1)
	{
		//Sniper hints only.
		if (GetEntProp(iHint, Prop_Data, "m_hint") != 0) 
			continue;
		
		//Ignore disabilities.
		if(GetEntProp(iHint, Prop_Data, "m_isDisabled") == 1)
			continue;
		
		aPossibleHints.Push(iHint);
		
		//Bomb not valid so let's just collect all hints.
		if(iBomb <= 0)
			continue;
		
		//For fallback.
		float flDistance = GetVectorDistance(WorldSpaceCenter(iHint), WorldSpaceCenter(iBomb));
		if(flDistance <= flBestDistance)
		{
			flBestDistance = flDistance;
			iBestHint = iHint;
		}
		
		if(!IsLineOfFireClear(WorldSpaceCenter(iBomb), WorldSpaceCenter(iHint)))
			continue;
		
		aVisibleHints.Push(iHint);
	}
	
	if(aVisibleHints.Length > 0)
	{
		iBestHint = aVisibleHints.Get(GetRandomInt(0, aVisibleHints.Length - 1));
	}
	else
	{
		if(iBomb <= 0)
		{
			//Explore random hints while bomb is not in play.
			return aPossibleHints.Get(GetRandomInt(0, aPossibleHints.Length - 1));
		}
	}
	
	delete aPossibleHints;
	delete aVisibleHints;
	
	return iBestHint;
}

bool CTFBotSniperLurk_FindNewHome(int actor)
{
	m_bHasHome[actor] = false;
	
	int iHint = CTFBotSniperLurk_FindHint();
	if(iHint == INVALID_ENT_REFERENCE)
		return false;

	NavArea area = TheNavMesh.GetNearestNavArea_Vec(WorldSpaceCenter(iHint), true);
	if(area == NavArea_Null)
		return false;
	
	area.GetRandomPoint(m_vecHome[actor]);
	m_bHasHome[actor] = true;
	
	return true;
}