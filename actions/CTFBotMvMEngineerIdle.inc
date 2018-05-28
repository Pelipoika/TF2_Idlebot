
float m_ctSentrySafe[MAXPLAYERS + 1];
float m_ctSentryCooldown[MAXPLAYERS + 1];

float m_ctDispenserSafe[MAXPLAYERS + 1]; 
float m_ctDispenserCooldown[MAXPLAYERS + 1];

float m_ctFindNestHint[MAXPLAYERS + 1]; 
float m_ctAdvanceNestSpot[MAXPLAYERS + 1]; 

float m_ctRecomputePathMvMEngiIdle[MAXPLAYERS + 1];

NavArea m_aNestArea[MAXPLAYERS + 1] = {NavArea_Null, ...};


bool g_bGoingToGrabBuilding[MAXPLAYERS + 1];
int m_hBuildingToGrab[MAXPLAYERS + 1];


stock bool CTFBotMvMEngineerIdle_IsPossible(int actor)
{
	return TF2_GetPlayerClass(actor) == TFClass_Engineer;
}

bool CTFBotMvMEngineerIdle_OnStart(int actor)
{
	m_hBuildingToGrab[actor] = INVALID_ENT_REFERENCE;
	g_bGoingToGrabBuilding[actor] = false;
	
	
	
	m_ctRecomputePathMvMEngiIdle[actor] = -1.0;
	
	m_ctSentrySafe[actor] = -1.0;
	m_ctSentryCooldown[actor] = -1.0;
	
	m_ctDispenserSafe[actor] = -1.0;
	m_ctDispenserCooldown[actor] = -1.0;

	m_ctFindNestHint[actor] = -1.0;
	m_ctAdvanceNestSpot[actor] = -1.0;
	
	g_bPath[actor] = true;
	
	return true;
}

bool CTFBotMvMEngineerIdle_Update(int actor)
{
	int sentry    = TF2_GetObject(actor, TFObject_Sentry);
	int dispenser = TF2_GetObject(actor, TFObject_Dispenser);

	bool bShouldAdvance = CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(actor);
	if(bShouldAdvance && !g_bGoingToGrabBuilding[actor])
	{
		//TF2_DetonateObjectsOfType(actor, TFObject_Sentry);
		//TF2_DetonateObjectsOfType(actor, TFObject_Dispenser);
		
		PrintToServer("ADVANCE");
		
		//RIGHT NOW
		CTFBotMvMEngineerIdle_OnStart(actor);
		
		m_aNestArea[actor] = PickBuildArea(actor);
		
		if (sentry != INVALID_ENT_REFERENCE && m_aNestArea[actor] != NavArea_Null)
		{
			g_bGoingToGrabBuilding[actor] = true;
			
			m_hBuildingToGrab[actor] = EntIndexToEntRef(sentry);
			
			PF_SetGoalEntity(actor, sentry);
		}
	}
	
	if(g_bGoingToGrabBuilding[actor])
	{
		int building = EntRefToEntIndex(m_hBuildingToGrab[actor]);
		if(building == INVALID_ENT_REFERENCE)
		{
			g_bGoingToGrabBuilding[actor] = false;
			m_hBuildingToGrab[actor] = INVALID_ENT_REFERENCE;
			
			PrintToServer("g_bGoingToGrabBuilding : building %i | m_aNestArea %x", building, m_aNestArea[actor]);
			
			TF2_DetonateObjectsOfType(actor, TFObject_Sentry);
			TF2_DetonateObjectsOfType(actor, TFObject_Dispenser);
			
			g_bPath[actor] = false;
			return false;
		}
		
		g_bUpdateLookingAroundForEnemies[actor] = false;
		
		if(!GetEntProp(actor, Prop_Send, "m_bCarryingObject"))
		{
			float flDistanceToBuilding = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(building));
			
			if(flDistanceToBuilding < 90.0)
			{
				EquipWeaponSlot(actor, TFWeaponSlot_Melee);
			
				BotAim(actor).AimHeadTowards(WorldSpaceCenter(building), CRITICAL, 1.0, "Grab building");
				BotAim(actor).PressAltFireButton();
				
				//PrintToServer("Grab");
			}
		}
		else
		{
			if(m_aNestArea[actor] != NavArea_Null)
			{
				float center[3]; m_aNestArea[actor].GetCenter(center);
				PF_SetGoalVector(actor, center);
				
				float flDistanceToGoal = GetVectorDistance(GetAbsOrigin(actor), center);
				
				if(flDistanceToGoal < 200.0)
				{	
					//Crouch when closer than 200 hu
					if(!IsStuck(actor))
					{
						g_iAdditionalButtons[actor] |= IN_DUCK;
					}
					
					if(flDistanceToGoal < 70.0)
					{
						//Try placing building when closer than 70 hu
						int objBeingBuilt = GetEntPropEnt(actor, Prop_Send, "m_hCarriedObject");
						
						if(!IsValidEntity(objBeingBuilt))
							return false;
						
						bool m_bPlacementOK = !!GetEntData(objBeingBuilt, FindSendPropInfo("CObjectSentrygun", "m_iKills") - 4);
						
						BotAim(actor).PressFireButton();
						
						if(!m_bPlacementOK && BotAim(actor).IsHeadAimingOnTarget() && BotAim(actor).GetHeadSteadyDuration() > 0.6 )
						{
							//That spot was no good.
							//Time to pick a new spot.
							m_aNestArea[actor] = PickBuildArea(actor);
						}
						else
						{
							g_bGoingToGrabBuilding[actor] = false;
							m_hBuildingToGrab[actor] = INVALID_ENT_REFERENCE;
							
							g_bPath[actor] = false;
						}
					}
				}
				
				//PrintToServer("Travel");
			}
		}
		
		g_bPath[actor] = true;
		
		return false;
	}
	
	if ((m_aNestArea[actor] == NavArea_Null || bShouldAdvance) || sentry == INVALID_ENT_REFERENCE)
	{
		//HasStarted && !IsElapsed
		if (m_ctFindNestHint[actor] > 0.0 && m_ctFindNestHint[actor] > GetGameTime()) 
		{
			return false;
		}
		
		//Start
		m_ctFindNestHint[actor] = GetGameTime() + (GetRandomFloat(1.0, 2.0));
		
		m_aNestArea[actor] = PickBuildArea(actor);
	}
	
	if(bShouldAdvance)
		return true;
	
	if (m_aNestArea[actor] != NavArea_Null)
	{
		if (sentry != INVALID_ENT_REFERENCE)
		{
			if(GetEntProp(sentry, Prop_Send, "m_iHealth") >= GetEntProp(sentry, Prop_Send, "m_iMaxHealth") 
			&& !GetEntProp(sentry, Prop_Send, "m_bBuilding")
			&& GetEntProp(sentry, Prop_Send, "m_iUpgradeLevel") >= 3
			&& GetEntProp(sentry, Prop_Send, "m_iAmmoShells") > 0)
			{
				m_ctSentrySafe[actor] = GetGameTime() + 3.0;
			}
			
			m_ctSentryCooldown[actor] = GetGameTime() + 3.0;	
		}
		else 
		{
			/* do not have a sentry; retreat for a few seconds if we had a
			 * sentry before this; then build a new sentry */
			if (m_ctSentryCooldown[actor] < GetGameTime()) 
			{
				m_ctSentryCooldown[actor] = GetGameTime() + 3.0;

				ChangeAction(actor, ACTION_MVM_ENGINEER_BUILD_SENTRYGUN, "No sentry - building a new one");
				
				return true;
			}
		}
		
		//Don't build a dispenser if we don't have a sentry...
		if(sentry != INVALID_ENT_REFERENCE)
		{
			if (dispenser != INVALID_ENT_REFERENCE)
			{
				//sentry is not safe.
				if(m_ctSentrySafe[actor] < GetGameTime())
				{
					m_ctDispenserCooldown[actor] = GetGameTime() + 3.0;
				}
				
				//m_ctDispenserCooldown[actor] = GetGameTime() + 3.0;	
			}
			else 
			{
				/* do not have a dispenser; retreat for a few seconds if we had a
				 * dispenser before this; then build a new dispenser */
				if (m_ctDispenserCooldown[actor] < GetGameTime() && m_ctSentrySafe[actor] > GetGameTime())
				{
					m_ctDispenserCooldown[actor] = GetGameTime() + 3.0;
	
					ChangeAction(actor, ACTION_MVM_ENGINEER_BUILD_DISPENSER, "Sentry safe, No dispenser - building one");
					
					return true;
				}
			}
		}
	}
	
	if (dispenser != INVALID_ENT_REFERENCE && m_ctSentrySafe[actor] > GetGameTime())
	{
		if(GetEntProp(dispenser, Prop_Send, "m_iUpgradeLevel") < 3 || GetEntProp(dispenser, Prop_Send, "m_iHealth") < GetEntProp(dispenser, Prop_Send, "m_iMaxHealth"))
		{
			float dist = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(dispenser));
			
			if (m_ctRecomputePathMvMEngiIdle[actor] < GetGameTime()) 
			{
				m_ctRecomputePathMvMEngiIdle[actor] = GetGameTime() + GetRandomFloat(1.0, 2.0);
				
				float dir[3];
				SubtractVectors(GetAbsAngles(dispenser), GetAbsOrigin(actor), dir);
				NormalizeVector(dir, dir);
				
				float goal[3]; goal = GetAbsOrigin(dispenser);
				goal[0] -= (50.0 * dir[0]);
				goal[1] -= (50.0 * dir[1]);
				goal[2] -= (50.0 * dir[2]);
				
				if (PF_IsPathToVectorPossible(actor, goal, _))
				{
					PF_SetGoalVector(actor, goal);
				}
				else
				{
					PF_SetGoalEntity(actor, sentry);
				}
				
				m_iRouteType[actor] = FASTEST_ROUTE;
				g_bPath[actor] = true;
			}
			
			if (dist < 90.0) 
			{
				if(!IsStuck(actor))
				{
					g_iAdditionalButtons[actor] |= IN_DUCK;
				}
				
				EquipWeaponSlot(actor, TFWeaponSlot_Melee);
				
				g_bUpdateLookingAroundForEnemies[actor] = false;
				
				BotAim(actor).AimHeadTowards(WorldSpaceCenter(dispenser), CRITICAL, 1.0, "Work on my Dispenser");
				BotAim(actor).PressFireButton();
			}
			
			return false;
		}
	}
	
	if (sentry != INVALID_ENT_REFERENCE) 
	{
		float dist = GetVectorDistance(GetAbsOrigin(actor), GetAbsOrigin(sentry));
		
		if (m_ctRecomputePathMvMEngiIdle[actor] < GetGameTime()) 
		{
			m_ctRecomputePathMvMEngiIdle[actor] = GetGameTime() + GetRandomFloat(1.0, 2.0);
			
			float dir[3];
			GetAngleVectors(GetTurretAngles(sentry), dir, NULL_VECTOR, NULL_VECTOR);
			
			float goal[3]; goal = GetAbsOrigin(sentry);
			goal[0] -= (50.0 * dir[0]);
			goal[1] -= (50.0 * dir[1]);
			goal[2] -= (50.0 * dir[2]);
			
			if (PF_IsPathToVectorPossible(actor, goal))
			{
				PF_SetGoalVector(actor, goal);
			}
			else
			{
				PF_SetGoalEntity(actor, sentry);
			}
			
			m_iRouteType[actor] = SAFEST_ROUTE;
			g_bPath[actor] = true;
		}
		
		if (dist < 90.0) 
		{
			if(!IsStuck(actor))
			{
				g_iAdditionalButtons[actor] |= IN_DUCK;
			}
			
			EquipWeaponSlot(actor, TFWeaponSlot_Melee);
			
			g_bUpdateLookingAroundForEnemies[actor] = false;
			
			BotAim(actor).AimHeadTowards(WorldSpaceCenter(sentry), CRITICAL, 1.0, "Work on my Sentry");
			BotAim(actor).PressFireButton();
		}
	}
	
	return true;
}

stock bool IsBaseObject(int entity)
{
	return HasEntProp(entity, Prop_Send, "m_bPlasmaDisable");
}

stock void CTFBotMvMEngineerIdle_OnEnd(int actor)
{
	EquipWeaponSlot(actor, TFWeaponSlot_Primary);
	//Something here so we don't get a compile warning
}

stock bool CTFBotMvMEngineerIdle_ShouldAdvanceNestSpot(int actor)
{
	if (m_aNestArea[actor] == NavArea_Null)
		return false;
	
	if (m_ctAdvanceNestSpot[actor] <= 0.0)
	{
		m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
		return false;
	}
	
	int obj = TF2_GetObject(actor, TFObject_Sentry);
	if (obj != INVALID_ENT_REFERENCE && GetEntProp(obj, Prop_Send, "m_iHealth") < GetEntProp(obj, Prop_Send, "m_iMaxHealth"))
	{
		m_ctAdvanceNestSpot[actor] = GetGameTime() + 5.0;
		return false;
	}
	
	//IsElapsed
	if (GetGameTime() > m_ctAdvanceNestSpot[actor])
	{
		m_ctAdvanceNestSpot[actor] = -1.0;
	}
	
	JSONObject bombinfo = new JSONObject();
	if (!GetBombInfo(bombinfo)) 
	{
		delete bombinfo;
		return false;
	}
	
	float m_flBombTargetDistance = GetBombTargetDistance(m_aNestArea[actor]);
	
	//No point in advancing now.
	if (m_flBombTargetDistance <= 1000.0)
	{
		delete bombinfo;
		return false;
	}
	
	bool bigger = (m_flBombTargetDistance > bombinfo.GetFloat("hatch_dist_back"));
	
	//PrintToServer("m_flBombTargetDistance %f > bombinfo.hatch_dist_back %f = %s", m_flBombTargetDistance, bombinfo.GetFloat("hatch_dist_back"), bigger ? "Yes" : "No");

	delete bombinfo;
	return bigger;
}