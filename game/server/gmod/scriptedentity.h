#pragma once

#include "baseentity.h"

class CScriptedEntity
{
public:
	typedef CScriptedEntity ThisClass;
	DECLARE_SIMPLE_DATADESC();
	DECLARE_EMBEDDED_NETWORKVAR()

	bool m_bLuaStarted;
	int m_iLuaEntityType;
	CBaseEntity* m_pEntity;
	CNetworkString( m_strScriptName, 64 );
	string_t m_sScriptName;
	char m_szModuleName[64];
	char m_szSelfName[64];
};
