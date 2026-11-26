#pragma once

#include "nextbot/nextbot.h"
#include "gmod/scriptedentity.h"
#include "physics_bone_follower.h"

class CLuaLocomotion;
class CLuaNextBotBody;
class CLuaNextBotVision;

class CLuaNextBot : public NextBotCombatCharacter
{
public:
	DECLARE_CLASS( CLuaNextBot, NextBotCombatCharacter );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	class CLuaNextBotIntention;

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iHealth );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iMaxHealth );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_lifeState ); // RaphaelIT7: GMod uses 3 bits for networking

	CNetworkVarEmbedded( CScriptedEntity, m_ScriptedEntity );

	CLuaNextBotIntention *m_pIntention;
	CLuaLocomotion *m_pLocomotor;
	CLuaNextBotBody *m_pBody;
	CLuaNextBotVision *m_pVision;
	Vector m_vecUnused1;
	Vector m_vecUnused2;
	int m_iMoveXPoseParameter;
	int m_iMoveYPoseParameter;
	CBoneFollowerManager m_BoneFollowerManager;
	int m_UseType;
};
