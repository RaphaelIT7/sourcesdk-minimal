#pragma once

abstract_class IGarrysMod
{
public:
	virtual void MenuThink() = 0;
	virtual void RunConsoleCommand( const char* cmd ) = 0;
	virtual void StartVideoScale( int, int ) = 0;
	virtual void EndVideoScale( int, int ) = 0;
};