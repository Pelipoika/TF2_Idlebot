
bool CTFBotSpyLurk_OnStart(int actor)
{
	if (!IsStealthed(actor)) {
		BotAim(actor).PressAltFireButton();
	}
	
	DisguiseAsMemberOfEnemyTeam(actor);
	
	m_ctPatience[actor] = GetGameTime() + GetRandomFloat(3.0, 5.0);
	
	return true;
}

bool CTFBotSpyLurk_Update(int actor)
{
	const CKnownEntity *threat = actor->GetVisionInterface()->GetPrimaryKnownThreat(false);
	if (threat != nullptr && threat->GetEntity() != nullptr) {
		auto obj = dynamic_cast<CBaseObject *>(threat->GetEntity());
		if (obj != nullptr && !obj->HasSapper() && actor->IsEnemy(obj)) {
			return ActionResult<CTFBot>::SuspendFor(new CTFBotSpySap(obj),
				"Sapping an enemy object");
		}
	}
	
	//if (actor->m_hTargetSentry != nullptr && !actor->m_hTargetSentry->HasSapper()) {
		//return ActionResult<CTFBot>::SuspendFor(new CTFBotSpySap(actor->m_hTargetSentry),
			//"Sapping a Sentry");
	//}
	
	if (m_ctPatience[actor] < GetGameTime()) {
		//return ActionResult<CTFBot>::Done("Lost patience with my hiding spot");
		ChangeAction(actor, ACTION_IDLE);
	}
	
	if (PF_GetLastKnownArea(actor) != NavArea_Null && threat != nullptr && threat->GetTimeSinceLastKnown() < 3.0) {
		CTFPlayer *victim = ToTFPlayer(threat->GetEntity());
		if (victim != nullptr && !victim->IsLookingTowards(actor, 0.9f)) {
			return ActionResult<CTFBot>::ChangeTo(new CTFBotSpyAttack(victim),
				"Going after a backstab victim");
		}
	}
	
	return ActionResult<CTFBot>::Continue();
}