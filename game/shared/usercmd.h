//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( USERCMD_H )
#define USERCMD_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/vector.h"
#include "utlvector.h"
#include "imovehelper.h"
#include "checksum_crc.h"


class bf_read;
class bf_write;

class CEntityGroundContact
{
public:
	int					entindex;
	float				minheight;
	float				maxheight;
};

/*
	Credits:
		I've got these values from Source.NET!
		https://github.com/marchc1/Source.NET/blob/00e4ba4edfef9b18cc774bdf1b22db2d87216734/Game.Shared/UserCmd.cs

	RaphaelIT7: I've yoinked it from Source.NET since they did great work for it <3
*/
#define MAX_BUTTON_PRESSED 5
#define MAX_MOTION_SENSOR_POSITIONS 20
class CUserCmd
{
public:
	CUserCmd()
	{
		Reset();
	}

	void Reset()
	{
		command_number = 0;
		tick_count = 0;
		viewangles.Init();
		forwardmove = 0.0f;
		sidemove = 0.0f;
		upmove = 0.0f;
		buttons = 0;
		impulse = 0;
		weaponselect = 0;
		weaponsubtype = 0;
		random_seed = 0;
#ifdef GAME_DLL
		server_random_seed = 0;
#endif
		mousedx = 0;
		mousedy = 0;

		for ( int i=0; i<MAX_BUTTON_PRESSED; ++i )
			pressedButtons[i] = 0;

		scrollSpeed = 0;
		worldClicking = false;
		isTyping = false;
		worldClickDirection.Zero();
		for ( int i=0; i<MAX_MOTION_SENSOR_POSITIONS; ++i )
			motionSensorPosition[i].Zero();

		forced = false;

		hasbeenpredicted = false;
	}

	CUserCmd& operator =( const CUserCmd& src )
	{
		if ( this == &src )
			return *this;

		command_number		= src.command_number;
		tick_count			= src.tick_count;
		viewangles			= src.viewangles;
		forwardmove			= src.forwardmove;
		sidemove			= src.sidemove;
		upmove				= src.upmove;
		buttons				= src.buttons;
		impulse				= src.impulse;
		weaponselect		= src.weaponselect;
		weaponsubtype		= src.weaponsubtype;
		random_seed			= src.random_seed;
#ifdef GAME_DLL
		server_random_seed = src.server_random_seed;
#endif
		mousedx				= src.mousedx;
		mousedy				= src.mousedy;

		for ( int i=0; i<MAX_BUTTON_PRESSED; ++i )
			pressedButtons[i] = src.pressedButtons[ i ];

		scrollSpeed = src.scrollSpeed;
		worldClicking = src.worldClicking;
		isTyping = src.isTyping;
		worldClickDirection = src.worldClickDirection;

		for ( int i=0; i<MAX_MOTION_SENSOR_POSITIONS; ++i )
			motionSensorPosition[i] = src.motionSensorPosition[i];

		forced = src.forced;

		hasbeenpredicted	= src.hasbeenpredicted;

		return *this;
	}

	CUserCmd( const CUserCmd& src )
	{
		*this = src;
	}

	CRC32_t GetChecksum( void ) const
	{
		CRC32_t crc;

		CRC32_Init( &crc );
		CRC32_ProcessBuffer( &crc, &command_number, sizeof( command_number ) );
		CRC32_ProcessBuffer( &crc, &tick_count, sizeof( tick_count ) );
		CRC32_ProcessBuffer( &crc, &viewangles, sizeof( viewangles ) );    
		CRC32_ProcessBuffer( &crc, &forwardmove, sizeof( forwardmove ) );   
		CRC32_ProcessBuffer( &crc, &sidemove, sizeof( sidemove ) );      
		CRC32_ProcessBuffer( &crc, &upmove, sizeof( upmove ) );         
		CRC32_ProcessBuffer( &crc, &buttons, sizeof( buttons ) );		
		CRC32_ProcessBuffer( &crc, &impulse, sizeof( impulse ) );        
		CRC32_ProcessBuffer( &crc, &weaponselect, sizeof( weaponselect ) );	
		CRC32_ProcessBuffer( &crc, &weaponsubtype, sizeof( weaponsubtype ) );
		CRC32_ProcessBuffer( &crc, &random_seed, sizeof( random_seed ) );
		CRC32_ProcessBuffer( &crc, &mousedx, sizeof( mousedx ) );
		CRC32_ProcessBuffer( &crc, &mousedy, sizeof( mousedy ) );
		CRC32_Final( &crc );

		return crc;
	}

	// Allow command, but negate gameplay-affecting values
	void MakeInert( void )
	{
		viewangles = vec3_angle;
		forwardmove = 0.f;
		sidemove = 0.f;
		upmove = 0.f;
		buttons = 0;
		impulse = 0;
	}

	// For matching server and client commands for debugging
	int		command_number;
	
	// the tick the client created this command
	int		tick_count;
	
	// Player instantaneous view angles.
	QAngle	viewangles;     
	// Intended velocities
	//	forward velocity.
	float	forwardmove;   
	//  sideways velocity.
	float	sidemove;      
	//  upward velocity.
	float	upmove;         
	// Attack button states
	int		buttons;
	// Current weapon id
	int		weaponselect;
	int		weaponsubtype;

	int		random_seed;	// For shared random functions
#ifdef GAME_DLL
	int		server_random_seed; // Only the server populates this seed
#endif

	short	mousedx;		// mouse accum in x from create move
	short	mousedy;		// mouse accum in y from create move

	// Client only, tracks whether we've predicted this command at least once
	bool	hasbeenpredicted;

	// Impulse command issued.
	byte    impulse;

	bool isTyping;
	bool forced;
	unsigned char pressedButtons[MAX_BUTTON_PRESSED];
	char scrollSpeed;
	bool worldClicking;
	Vector worldClickDirection;
	Vector motionSensorPosition[MAX_MOTION_SENSOR_POSITIONS];
};

void ReadUsercmd( bf_read *buf, CUserCmd *move, CUserCmd *from );
void WriteUsercmd( bf_write *buf, const CUserCmd *to, const CUserCmd *from );

#endif // USERCMD_H
