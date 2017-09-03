bool CTFBotSeekAndDestroy_CTFBotSeekAndDestroy(float duration)
{
	if (duration > 0.0) {
		this->m_ctActionDuration.Start(duration);
	}
}

bool CTFBotSeekAndDestroy_OnStart(int actor, Action<CTFBot> *action)
{
	this->m_PathFollower.SetMinLookAheadDistance(actor->GetDesiredPathLookAheadRange());
	
	this->RecomputeSeekPath(actor);
	
	CTeamControlPoint *point = actor->GetMyControlPoint();
	if (point != nullptr) {
		this->m_bPointLocked = point->IsLocked();
	} else {
		this->m_bPointLocked = false;
	}
	
	// start the countdown timer back to the beginning
	if (this->m_ctActionDuration.HasStarted()) {
		this->m_ctActionDuration.Reset();
	}
	
	return true;
}

bool CTFBotSeekAndDestroy_Update(int actor, float dt)
{
	if (this->m_ctActionDuration.HasStarted() && this->m_ctActionDuration.IsElapsed()) {
		return bool::Done("Behavior duration elapsed");
	}
	
	if (actor->IsCapturingPoint()) {
		return bool::Done("Keep capturing point I happened to stumble upon");
	}
	
	if (this->m_bPointLocked) {
		CTeamControlPoint *point = actor->GetMyControlPoint();
		if (point != nullptr && !point->IsLocked()) {
			return bool::Done("The point just unlocked");
		}
	}
	
	// BUG: this condition always resolves as true if there's no round time
	if (TFGameRules()->State_Get() != GR_STATE_TEAM_WIN &&
		actor->GetTimeLeftToCapture() < tf_bot_offense_must_push_time.GetFloat()) {
		return bool::Done("Time to push for the objective");
	}
	
	const CKnownEntity *threat = actor->GetVisionInterface()->GetPrimaryKnownThreat(false);
	if (threat != nullptr) {
		/* BUG: doesn't check if we're on the winning team; basically we just
		 * assume that we would only be in SeekAndDestroy in the first place if
		 * TacticalMonitor put us here */
		if (TFGameRules()->State_Get() == GR_STATE_TEAM_WIN) {
			return bool::SuspendFor(new CTFBotAttack(),
				"Chasing down the losers");
		}
		
		if (actor->IsRangeLessThan(threat->GetLastKnownPosition(), 1000.0f)) {
			return bool::SuspendFor(new CTFBotAttack(),
				"Going after an enemy");
		}
	}
	
	this->m_PathFollower.Update(actor);
	
	if (!this->m_PathFollower.IsValid() && this->m_ctRecomputePath.IsElapsed()) {
		this->m_ctRecomputePath.Start(1.0f);
		this->RecomputeSeekPath(actor);
	}
	
	return bool::Continue();
}

CTFNavArea *CTFBotSeekAndDestroy_ChooseGoalArea(int actor)
{
	CUtlVector<CTFNavArea *> areas;
	TheNavMesh->CollectSpawnRoomThresholdAreas(&areas, GetEnemyTeam(actor));
	
	CTeamControlPoint *point = actor->GetMyControlPoint();
	if (point != nullptr && !point->IsLocked()) {
		int index = point->GetPointIndex();
		if (index < 8) {
			// TODO: some CTFNavMesh stuff
			// (may add one more area to vector)
		}
	}
	
	if (!areas.IsEmpty()) {
		return areas.Random();
	} else {
		return nullptr;
	}
}

void CTFBotSeekAndDestroy_RecomputeSeekPath(int actor)
{
	if ((this->m_GoalArea = this->ChooseGoalArea(actor)) == nullptr) {
		this->m_PathFollower.Invalidate();
		return;
	}
	
	CTFBotPathCost path_cost(actor, SAFEST_ROUTE);
	this->m_PathFollower.Compute(actor, this->m_GoalArea->GetCenter(), path_cost, 0.0, true);
}


// TODO: need CTeamControlPoint *CTFBot::GetMyControlPoint