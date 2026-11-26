#include "baseentity.h"
#include "gmod/scriptedentity.h"

class CSENT_point : public CPointEntity
{
public:
	DECLARE_CLASS( CSENT_point, CPointEntity );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual int UpdateTransmitState( void );
	virtual void Spawn( void );
	virtual void Precache();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual int ObjectCaps(void);
	virtual bool AcceptInput( const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID );
	virtual void OnRestore();
	virtual void UpdateOnRemove( void );
	virtual bool UsesLua();
	virtual int GetLuaEntityType();
	virtual const char *GetLuaScriptName();
	virtual void InitializeScriptedEntity( const char * );

	virtual void ScriptThink();
	virtual void ClientThink();

	CNetworkVarEmbedded( CScriptedEntity, m_ScriptedEntity );
};