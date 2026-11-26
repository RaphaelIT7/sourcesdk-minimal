#ifndef PURE_SERVER_H
#define PURE_SERVER_H
#ifdef _WIN32
#pragma once
#endif

#include "filesystem.h"
#include "utldict.h"
#include "utlvector.h"
#include "utlbuffer.h"

class KeyValues;

class CPureServerWhitelist : public IPureServerWhitelist
{
public:
	static CPureServerWhitelist *Create( IFileSystem *pFileSystem );

	virtual void AddRef();
	virtual void Release();
	virtual EPureServerFileClass GetFileClass( const char *pszFilename );
	virtual int GetTrustedKeyCount() const;
	virtual const byte *GetTrustedKey( int iKeyIndex, int *nKeySize ) const;

	void Init( IFileSystem *pFileSystem );
	void Term();

	// Load up whitelist from config files
	void Load( int iPureMode );

	// Encode for networking.
	void Encode( CUtlBuffer &buf );
	void Decode( CUtlBuffer &buf );

	// Calls IFileSystem::CacheFileCRCs for all files and directories marked check_crc.
	void CacheFileCRCs();

	// Used by sv_pure command when you're connected to a server.
	void PrintWhitelistContents();

	// Compare for equivalence
	bool operator==( const CPureServerWhitelist &other ) const;

private:
	class CCommand
	{
	public:
		CCommand();
		~CCommand();

		EPureServerFileClass m_eFileClass;
		unsigned short m_LoadOrder;	// What order this thing was specified in the whitelist file?  Used to resolve rule conflicts.
	};

	CPureServerWhitelist();
	~CPureServerWhitelist();

	void AddHardcodedFileCommands();
	void AddFileCommand( const char *pszPattern, EPureServerFileClass eFileClass, unsigned short nLoadOrder );
	// Load up any entries in the KV file.
	void LoadCommandsFromKeyValues( KeyValues *pKeyValues );

	// Load list of trusted keys from specified file
	void LoadTrustedKeysFromKeyValues( KeyValues *pKeyValues );
	void InternalCacheFileCRCs( CUtlDict<CCommand *, int> &theDict, ECacheCRCType eType );
	CCommand *GetBestEntry( const char *pszFilename );
	CCommand *CheckEntry( CUtlDict<CCommand *, int> &theDict, const char *pszEntryName, CCommand *pBestEntry );
	CCommand *FindCommandByLoadOrder( CUtlDict<CCommand *, int> &theDict, int nLoadOrder );
	void PrintCommand( const char *pszFormat, const char *pszPattern, int nIndex, CCommand *pCommand );
	void EncodeCommandList( CUtlDict<CCommand *, int> &theDict, CUtlBuffer &buf );
	void DecodeCommandList( CUtlDict<CCommand *, int> &theDict, CUtlBuffer &buf, unsigned int nCount );
	void UpdateCommandStats( CUtlDict<CCommand *, int> &theDict, int *pHighest, int *pCount );
	bool CommandDictDifferent( const CUtlDict<CCommand *, int> &a, const CUtlDict<CCommand *, int> &b ) const;

	unsigned short m_LoadCounter;	// Incremented as we load things so their m_LoadOrder increases.
	CInterlockedInt m_RefCount;

	// Commands are applied to files in order.
	CUtlDict<CCommand *, int> m_FileCommands;				// file commands
	CUtlDict<CCommand *, int> m_NonRecursiveDirCommands;	// *.* commands
	CUtlDict<CCommand *, int> m_RecursiveDirCommands;		// ... commands
	IFileSystem *m_pFileSystem;

	// List of trusted keys
	CUtlVector< CUtlVector<byte> > m_vecTrustedKeys;
};

#endif
