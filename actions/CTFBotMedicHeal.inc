/* reverse engineering by sigsegv
 * based on TF2 version 20151007a
 * server/tf/bot/behavior/medic/tf_bot_medic_heal.cpp
 * used in MvM: TODO
 */


/*
ConVar tf_bot_medic_stop_follow_range("tf_bot_medic_stop_follow_range", "75", FCVAR_CHEAT);
ConVar tf_bot_medic_start_follow_range("tf_bot_medic_start_follow_range", "250", FCVAR_CHEAT);
ConVar tf_bot_medic_max_heal_range("tf_bot_medic_max_heal_range", "600", FCVAR_CHEAT);
ConVar tf_bot_medic_debug("tf_bot_medic_debug", "0", FCVAR_CHEAT);
ConVar tf_bot_medic_max_call_response_range("tf_bot_medic_max_call_response_range", "1000", FCVAR_CHEAT);
ConVar tf_bot_medic_cover_test_resolution("tf_bot_medic_cover_test_resolution", "8", FCVAR_CHEAT);
*/

float m_vecPatientPosition[MAXPLAYERS + 1][3];
int m_hPatient[MAXPLAYERS + 1];

bool CTFBotMedicHeal_OnStart(int actor)
{
	m_vecPatientPosition[actor] = NULL_VECTOR;
	m_hPatient[actor] = -1;
	// dword @ 0x4868 = 0
	// CountdownTimer @ 0x485c .Invalidate()
	
	return true;
}

bool CTFBotMedicHeal_Update(int actor)
{
	m_hPatient[actor] = this->SelectPatient(actor, m_hPatient[actor]);
	
	/* try very hard to avoid healing medics; handle healbeam loops safely */
	if (TF2_IsMvM() && m_hPatient[actor] != -1 && TF2_GetPlayerClass(m_hPatient[actor]) == TFClass_Medic) 
	{
		CUtlVector<CBaseEntity *> patients;
		patients.AddToTail(m_hPatient[actor]);
		
		CBaseEntity *patient;
		while (ToTFPlayer(patient = m_hPatient[actor]->MedicGetHealTarget()) != nullptr) 
		{
			if (!patients.IsEmpty()) 
			{
				bool loop = false;
				FOR_EACH_VEC(patients, i) 
				{
					if (patient == patients[i]) 
					{
						loop = true;
						break;
					}
				}
				
				if (loop) break;
			}
			
			patients.AddToTail(patient);
			
			m_hPatient[actor] = ToTFPlayer(patient);
		}
	}
	
	// LABEL_23
	
	if (m_hPatient[actor] != nullptr) 
	{
		if (m_hPatient[actor]->GetAbsOrigin() - this->m_vecPatientPosition).LengthSqr > Square(200.0f)) 
		{
			this->m_vecPatientPosition = m_hPatient[actor]->GetAbsOrigin();
			
			// CountdownTimer @ 0x485c .Start(3.0f)
		}
		
		if (m_hPatient[actor]->m_Shared.InCond(TF_COND_SELECTED_TO_TELEPORT)) 
		{
			CUtlVector<CBaseObject *> objs;
			TheNavMesh->CollectBuiltObjects(&objs, actor->GetTeamNumber());
			
			CObjectTeleporter *best_tele = nullptr;
			float best_dsqr              = FLT_MAX;
			
			FOR_EACH_VEC(objs, i) 
			{
				if (objs[i]->GetType() != OBJ_TELEPORTER) 
				{
					continue;
				}
				
				auto tele = static_cast<CObjectTeleporter *>(objs[i]);
				
				if (tele->m_iTeleportType != 1 || !tele->IsReady()) 
				{
					continue;
				}
				
				float dsqr = (m_hPatient[actor]->GetAbsOrigin() - tele->GetAbsOrigin()).LengthSqr();
				if (dsqr < best_dsqr) 
				{
					best_tele = tele;
					best_dsqr = dsqr;
				}
			}
			
			if (best_tele != nullptr) 
			{
				// TODO: enum name for (CTFBotUseTeleporter::UseHowType)1
				return ActionResult<CTFBot>::SuspendFor(new CTFBotUseTeleporter(best_tele, 1), "Following my patient through a teleporter");
			}
		}
		
		// LABEL_89 / LABEL_90
		
		const CKnownEntity *threat = actor->GetVisionInterface()->GetPrimaryKnownThreat(false);
		
		// TODO: names for these
		bool bool1 = true;
		bool bool2 = false;
		
		CWeaponMedigun *medigun = nullptr;
		
		CTFWeaponBase *weapon = actor->m_Shared.GetActiveTFWeapon();
		if (weapon != nullptr) 
		{
			medigun = dynamic_cast<CWeaponMedigun *>(weapon);
			if (medigun != nullptr) 
			{
				if (medigun->GetMedigunType() == 3) 
				{
					// TODO: bunch of branch spaghetti for cycling resist type
					// based on bot attributes
					
					// 100000 VACCINATORBULLETS
					// 200000 VACCINATORBLAST
					// 400000 VACCINATORFIRE
				}
				
				if (!medigun->IsReleasingCharge()) 
				{
					// TODO
				}
				
				actor->GetBodyInterface()->AimHeadTowards(m_hPatient[actor],
					IBody::LookAtPriorityType::CRITICAL, 1.0f, nullptr,
					"Aiming at my patient");
				
				if (medigun->GetHealTarget() != nullptr &&
					medigun->GetHealTarget() != m_hPatient[actor]) 
				{
					actor->PressFireButton();
					
					bool2 = (medigun->GetHealTarget() != nullptr);
					bool1 = false;
				} 
				else 
				{
					if (/* CountdownTimer @ 0x4834 IsElapsed */) {
						// TODO
						
						// RandomFloat etc
					} 
					else 
					{
						actor->PressFireButton();
						
						bool2 = false;
						bool1 = true;
					}
				}
				
				if (!this->IsReadyToDeployUber(medigun)) 
				{
					// TODO
					
					if (this->IsReadyToDeployUber(medigun) ||
						actor->m_Shared.InCond(TF_COND_INVULNERABLE) ||
						bool2 || (!bool3 && !out_of_range && !bool4)) 
					{
						CBaseCombatWeapon *secondary = actor->Weapon_GetSlot(1);
						if (secondary != nullptr) 
						{
							actor->Weapon_Switch(secondary);
						}
					} 
					else 
					{
						// TODO
						// aiming at enemy
					}
					
					// TODO
				}
				
				// TODO
			}
		}
		
		if (medigun != nullptr && this->IsReadyToDeployUber(medigun)) 
		{
			if (medigun->GetMedigunType() == 3) 
			{
				// TODO
			} 
			else 
			{
				// TODO
			}
		}
		
		// L_medigun_is_nullptr
		
		// TODO
		// is visible recently
		// path stuff
		// etc
		
		return ActionResult<CTFBot>::Continue();
	}
	
	return ActionResult<CTFBot>::SuspendFor(new CTFBotMedicRetreat(), "Retreating to find another patient to heal");
	
	return true;
}

ActionResult<CTFBot> CTFBotMedicHeal::OnResume(CTFBot *actor, Action<CTFBot> *action)
{
	this->m_ChasePath.Invalidate();
	
	return ActionResult<CTFBot>::Continue();
}
/*
bool CTFBotMedicHeal_OnActorEmoted(CTFBot *actor, CBaseCombatCharacter *who, int concept)
{
	if (!who->IsPlayer()) {
		return EventDesiredResult<CTBot>::Continue();
	}
	
	CTFPlayer *player = ToTFPlayer(who);
	if (player == nullptr) {
		return EventDesiredResult<CTBot>::Continue();
	}
	
	if (concept == MP_CONCEPT_PLAYER_GO ||
		concept == MP_CONCEPT_PLAYER_ACTIVATECHARGE) {
		CTFPlayer *patient = m_hPatient[actor]();
		if (patient != nullptr && player != nullptr &&
			ENTINDEX(player) == ENTINDEX(patient)) {
			CWeaponMedigun *medigun = dynamic_cast<CWeaponMedigun *>(
				actor->m_Shared.GetActiveTFWeapon());
			
			if (this->IsReadyToDeployUber(medigun)) {
				actor->PressAltFireButton();
			}
		}
	}
	
	return EventDesiredResult<CTFBot>::Continue();
}


QueryResponse CTFBotMedicHeal::ShouldRetreat(const INextBot *nextbot) const
{
	CTFBot *actor = static_cast<CTFBot *>(nextbot->GetEntity());
	
	return (actor->m_Shared.IsControlStunned() ||
		actor->m_Shared.IsLoserStateStunned());
}

QueryResponse CTFBotMedicHeal_ShouldAttack(const INextBot *nextbot, const CKnownEntity *threat) const
{
	CTFBot *actor = static_cast<CTFBot *>(nextbot->GetEntity());
	
	return actor->IsCombatWeapon();
}
*/

void CTFBotMedicHeal_ComputeFollowPosition(CTFBot *actor)
{
	VPROF_BUDGET("CTFBotMedicHeal::ComputeFollowPosition", "NextBot");
	
	this->m_vecFollowPosition = actor->GetAbsOrigin();
	
	// TODO
}

bool CTFBotMedicHeal_IsGoodUberTarget(CTFPlayer *player) const
{
	if (player->IsPlayerClass(TF_CLASS_MEDIC) ||
		player->IsPlayerClass(TF_CLASS_SNIPER) ||
		player->IsPlayerClass(TF_CLASS_ENGINEER) ||
		player->IsPlayerClass(TF_CLASS_SCOUT) ||
		player->IsPlayerClass(TF_CLASS_SPY)) {
		return false;
	}
	
	/* BUG: this function always returns false! */
	return false;
}

bool CTFBotMedicHeal_IsReadyToDeployUber(const CWeaponMedigun *medigun) const
{
	// TODO
}

bool CTFBotMedicHeal_IsStable(CTFPlayer *player) const
{
	// TODO
}

bool CTFBotMedicHeal_IsVisibleToEnemy(CTFBot *actor, const Vector& v1) const
{
	// TODO
}

int CTFBotMedicHeal_SelectPatient(CTFBot *actor, CTFPlayer *old_patient)
{
	// TODO
}


bool CKnownCollector_Inspect(const CKnownEntity& known)
{
	this->m_KnownEntities.AddToTail(&known);
	return true;
}


bool CFindMostInjuredNeighbor_Inspect(const CKnownEntity& known)
{
	if (!known.GetEntity()->IsPlayer()) {
		return true;
	}
	
	CTFPlayer *player = ToTFPlayer(known.GetEntity());
	if (this->m_pMedic->IsRangeGreaterThan(player, this->m_flRangeLimit) ||
		!this->m_pMedic->IsLineOfFireClear(player->EyePosition())) {
		return true;
	}
	
	if (this->m_pMedic->IsSelf(player) || !player->IsAlive() ||
		player->InSameTeam(this->m_pMedic)) {
		return true;
	}
	
	float max_health;
	if (this->m_bUseNonBuffedMaxHealth) 
	{
		max_health = (float)player->GetMaxHealth();
	} 
	else 
	{
		max_health = (float)player->GetMaxBuffedHealth(false, false);
	}
	
	float health_ratio = (float)player->GetHealth() / max_health;
	
	if (this->m_bIsOnFire) 
	{
		if (!player->m_Shared.InCond(TF_COND_BURNING)) 
		{
			return true;
		}
	} 
	else 
	{
		if (player->m_Shared.InCond(TF_COND_BURNING)) 
		{
			this->m_pMostInjured = player;
			this->m_flHealthRatio = health_ratio;
			this->m_bIsOnFire = true;
			return true;
		}
	}
	
	if (health_ratio < this->m_flHealthRatio) 
	{
		this->m_pMostInjured = player;
		this->m_flHealthRatio = health_ratio;
	}
	
	return true;
}


bool CSelectPrimaryPatient_Inspect(const CKnownEntity& known)
{
	/* BUG: medics will refuse to heal friendly spies because they magically
	 * know whether players are actually friendly instead of believing the
	 * spy's disguise */
	if (known.GetEntity() == nullptr ||
		!known.GetEntity()->IsPlayer() ||
		!known.GetEntity()->IsAlive() ||
		!this->m_pMedic->IsFriend(known.GetEntity())) {
		return true;
	}
	
	CTFPlayer *player = dynamic_cast<CTFPlayer *>(known.GetEntity());
	if (player == nullptr || this->m_pMedic->IsSelf(player)) 
	{
		return true;
	}
	
	if (!player->HasTheFlag() && !this->m_pMedic->m_Squad == nullptr)
	{
		if (player->IsPlayerClass(TF_CLASS_MEDIC) ||
			player->IsPlayerClass(TF_CLASS_SNIPER) ||
			player->IsPlayerClass(TF_CLASS_ENGINEER) ||
			player->IsPlayerClass(TF_CLASS_SPY))) {
			return true;
		}
	}
	
	CTFPlayer *patient = this->m_pPatient;
	
	if (TFGameRules()->IsInTraining()) 
	{
		if (patient != nullptr && !patient->IsBot()) 
		{
			this->m_pPatient = patient;
			return true;
		} 
		else 
		{
			this->m_pPatient = player;
			return true;
		}
	}
	
	if (ENTINDEX(this->m_pMedic->m_Squad->GetLeader()) == ENTINDEX(patient)) 
	{
		this->m_pPatient = patient;
		return true;
	}
	
	if (ENTINDEX(this->m_pMedic->m_Squad->GetLeader()) == ENTINDEX(player)) 
	{
		this->m_pPatient = player;
		return true;
	}
	
	int num_healers = patient->m_Shared.m_nNumHealers;
	for (int i = 0; i < num_healers; ++i) 
	{
		CTFPlayer *healer = ToTFPlayer(patient->m_Shared.GetHealerByIndex(i));
		if (healer != nullptr) 
		{
			if (!this->m_pMedic->IsSelf(healer)) 
			{
				this->m_pPatient = player;
				return true;
			}
		}
	}
	
	for (int i = 0; i < player->m_Shared.m_nNumHealers; ++i) 
	{
		CTFBotPathCost cost_func(this->m_pMedic, FASTEST_ROUTE);
		
	}
	
	
	// TODO
	
	return true;
}

UNKNOWN CSelectPrimaryPatient::SelectPreferred(CTFPlayer *player1, CTFPlayer *player2)
{
	static int preferredClass[] = {
		TF_CLASS_HEAVYWEAPONS,
		TF_CLASS_SOLDIER,
		TF_CLASS_PYRO,
		TF_CLASS_DEMOMAN,
		TF_CLASS_UNDEFINED,
	}
	
	// TODO
}