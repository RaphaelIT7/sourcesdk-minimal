//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Satchel Charge
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef	SATCHEL_H
#define	SATCHEL_H

#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"
#include "hl2mp/weapon_slam.h"

class CSprite;

class CSoundPatch;

class CSatchelCharge : public CBaseGrenade
{
public:
	DECLARE_CLASS( CSatchelCharge, CBaseGrenade );

	void			Spawn( void );
	void			Precache( void );
	void			BounceSound( void );
	void			UpdateSlideSound( void );
	void			KillSlideSound(void);
	void			SatchelTouch( CBaseEntity *pOther );
	void			SatchelThink( void );
	void			SatchelUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	float			m_flNextBounceSoundTime;
	bool			m_bInAir;
	Vector			m_vLastPosition;

public:
	CHandle<CWeapon_SLAM>	m_pMyWeaponSLAM;	// Who shot me..
	bool			m_bIsAttached;
	CHandle<CSprite>	m_hGlowSprite;
	void			Deactivate( void );

	CSatchelCharge();
	~CSatchelCharge();

	DECLARE_DATADESC();

private:
	void InitSlideSound(void);
};

#endif	//SATCHEL_H
