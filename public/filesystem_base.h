#ifndef FILESYSTEM_BASE_H
#define FILESYSTEM_BASE_H

#if defined( _WIN32 )
#pragma once
#endif

#include "tier1/refcount.h"
#include "tier1/utlsymbol.h"
#include "tier1/utlstring.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utldict.h"
#include "tier0/tslist.h"
#include "tier1/byteswap.h"
#include "tier1/UtlSortVector.h"
#include "filesystem.h"
#include "tier1/tier1.h"
#include "tier1/utlintrusivelist.h"
#include "threadsaferefcountedobject.h"
#include "vpklib/packedstore.h"

class IThreadPool;
class CPackFile;
class CBaseFileSystem;
class CPackedStore;
class CFileInfo;
struct FIND_DATA;

enum FileMode_t
{
	FM_BINARY,
	FM_TEXT
};

enum FileType_t
{
	FT_NORMAL,
	FT_PACK_BINARY,
	FT_PACK_TEXT
};

struct MapPakSearchFile;

class CPackFileHandle
{
public:
	virtual int Read( void *pBuffer, int nDestSize, int nBytes ) = 0;
	virtual int Seek( int64_t nOffset, int nWhence ) = 0;
	virtual int Tell( ) = 0;
	virtual unsigned int Size( ) = 0;

	int64_t m_nBase;
	int64_t m_nFilePointer;
	CPackFile *m_pOwner;
	uint32_t m_nLength;
	uint32_t m_nIndex;
};

namespace Addon
{
	class FileHandle;
}

class CFileHandle
{
public:
	virtual ~CFileHandle( ) = 0;

#if !defined( _RETAIL )
	char *m_pszTrueFileName;
#endif

	CPackFileHandle *m_pPackFileHandle;
	CPackedStoreFileHandle m_VPKHandle;
	Addon::FileHandle *m_pAddonFileHandle;
	int64_t m_nLength;
	FileType_t m_type;
	FILE *m_pFile;
	CBaseFileSystem *m_fs;
	uint32_t m_nMagic;
};

class CPackFile : public CRefCounted<CRefCountServiceMT>
{
public:
	virtual ~CPackFile( ) = 0;
	virtual CFileHandle *OpenFile( const char *pFileName, const char *pOptions = "rb" ) = 0;
	virtual bool ContainsFile( const char *pFileName ) = 0;
	virtual bool Prepare( int64_t fileLen = -1, int64_t nFileOfs = 0 ) = 0;
	virtual bool FindFile( const char *pFilename, int32_t &nIndex, int64_t &nPosition, int32_t &nOriginalSize, int32_t &nCompressedSize, uint16_t &nCompressionMethod ) = 0;
	virtual void FindFirstHelper( CUtlLinkedList<MapPakSearchFile> *, const char * ) = 0;
	virtual int32_t ReadFromPack( int32_t nIndex, void *buffer, int32_t nDestBytes, int32_t nBytes, int64_t nOffset ) = 0;
	virtual bool IndexToFilename( int32_t nIndex, char *buffer, int32_t nBufferSize ) = 0;
	virtual void SetupPreloadData( ) = 0;
	virtual void DiscardPreloadData( ) = 0;
	virtual int64_t GetPackFileBaseOffset( ) = 0;

	CThreadFastMutex m_mutex; // 8b
	CUtlSymbol m_Path; // 4b
	int64_t m_nBaseOffset; // 8b
	CUtlString m_ZipName; // 4b
	bool m_bIsMapPath; // 4b
	long m_lPackFileTime; // 4b
	int32_t m_refCount; // 4b
	int32_t m_nOpenFiles; // 4b
	FILE *m_hPackFileHandleFS; // 4b
	CPackedStoreFileHandle m_hPackFileHandleVPK;
	bool m_bIsExcluded;
	int32_t m_PackFileID;
	int64_t m_FileLength; // 8b
	CBaseFileSystem *m_fs;  // 4b
};

class CZipPackFile : public CPackFile
{
public:
	class CPackFileEntry
	{
	public:
		uint32_t m_nPosition;
		uint32_t m_nOriginalSize;
		uint32_t m_nCompressedSize;
		uint32_t m_HashName;
		uint16_t m_nPreloadIdx;
		uint16_t pad;
		uint16_t m_nCompressionMethod;
#if !defined( _RETAIL )
		FileNameHandle_t m_hDebugFilename;
#endif
	};

	class CPackFileLessFunc
	{
	public:
		bool Less( CPackFileEntry const &src1, CPackFileEntry const &src2, void *pCtx );
	};

	CUtlSortVector<CPackFileEntry, CPackFileLessFunc> m_PackFiles; // 28b
	int64_t m_nPreloadSectionOffset; // 8b
	uint32_t m_nPreloadSectionSize; // 4b
	struct ZIP_PreloadHeader *m_pPreloadHeader; // 4b
	uint16_t *m_pPreloadRemapTable; // 4b
	struct ZIP_PreloadDirectoryEntry *m_pPreloadDirectory; // 4b
	void *m_pPreloadData; // 4b
	CByteswap m_swap; // 4b
	uint8_t *m_pMemoryImage;
};

class CPathIDFileList
{
public:
	CUtlSymbol m_PathID;
	CUtlDict<CFileInfo *, int32_t> m_Files;
	CUtlLinkedList<CFileInfo *, int32_t> m_UnverifiedCRCFiles;
};

class CFileInfo
{
public:
	uint16_t m_Flags;
	CRC32_t m_CRC;
	CPathIDFileList	*m_pPathIDFileList;
	int32_t m_PathIDFileListDictIndex;
	int32_t m_iNeedsVerificationListIndex;
};

class CFileTracker
{
public:
	CUtlLinkedList<CFileInfo *> m_NeedsVerificationList;
	CUtlDict<CPathIDFileList *, int32_t> m_PathIDs;
	CBaseFileSystem *m_pFileSystem;
	CThreadMutex m_Mutex;
};

class CWhitelistSpecs
{
public:
	IFileList *m_pWantCRCList;
	IFileList *m_pAllowFromDiskList;
};
typedef CThreadSafeRefCountedObject<CWhitelistSpecs *> CWhitelistHolder;

class CBaseFileSystem : public CTier1AppSystem<IFileSystem>
{
public:
	virtual void *QueryInterface( const char *pInterfaceName ) = 0;
	virtual InitReturnVal_t Init( ) = 0;
	virtual void Shutdown( ) = 0;

	// file handling
	virtual FileHandle_t Open( const char *pFileName, const char *pOptions, const char *pathID = nullptr ) = 0;
	virtual FileHandle_t OpenEx( const char *pFileName, const char *pOptions, unsigned flags = 0, const char *pathID = nullptr, char **ppszResolvedFilename = nullptr ) = 0;
	virtual void Close( FileHandle_t file ) = 0;
	virtual void Seek( FileHandle_t file, long long pos, FileSystemSeek_t seekType ) = 0;
	virtual unsigned int Tell( FileHandle_t file ) = 0;
	virtual unsigned int Size( FileHandle_t file ) = 0;
	virtual unsigned int Size( const char *pFileName, const char *pPathID = nullptr ) = 0;

	virtual void SetBufferSize( FileHandle_t file, unsigned nBytes ) = 0;
	virtual bool IsOk( FileHandle_t file ) = 0;
	virtual void Flush( FileHandle_t file ) = 0;
	virtual bool Precache( const char *pFileName, const char *pPathID = nullptr ) = 0;
	virtual bool EndOfFile( FileHandle_t file ) = 0;

	virtual int Read( void *pOutput, int size, FileHandle_t file ) = 0;
	virtual int ReadEx( void *pOutput, int sizeDest, int size, FileHandle_t file ) = 0;
	virtual int Write( void const *pInput, int size, FileHandle_t file ) = 0;
	virtual char *ReadLine( char *pOutput, int maxChars, FileHandle_t file ) = 0;
	virtual int FPrintf( FileHandle_t file, PRINTF_FORMAT_STRING const char *pFormat, ... ) = 0;

	// Reads/writes files to utlbuffers
	virtual bool ReadFile( const char *pFileName, const char *pPath, CUtlBuffer &buf, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = nullptr ) = 0;
	virtual bool WriteFile( const char *pFileName, const char *pPath, CUtlBuffer &buf ) = 0;
	virtual bool UnzipFile( const char *pFileName, const char *pPath, const char *pDestination ) = 0;
	virtual int ReadFileEx( const char *pFileName, const char *pPath, void **ppBuf, bool bNullTerminate = false, bool bOptimalAlloc = false, int nMaxBytes = 0, int nStartingByte = 0, FSAllocFunc_t pfnAlloc = nullptr ) = 0;
	virtual bool ReadToBuffer( FileHandle_t hFile, CUtlBuffer &buf, int nMaxBytes = 0, FSAllocFunc_t pfnAlloc = nullptr ) = 0;

	// Optimal buffer
	virtual bool GetOptimalIOConstraints( FileHandle_t hFile, unsigned *pOffsetAlign, unsigned *pSizeAlign, unsigned *pBufferAlign ) = 0;
	virtual void *AllocOptimalReadBuffer( FileHandle_t hFile, unsigned nSize = 0, unsigned nOffset = 0 ) = 0;
	virtual void FreeOptimalReadBuffer( void * ) = 0;

	// Gets the current working directory
	virtual bool GetCurrentDirectory( char *pDirectory, int maxlen ) = 0;

	// this isn't implementable on STEAM as is.
	virtual void CreateDirHierarchy( const char *path, const char *pathID = nullptr ) = 0;

	// returns true if the file is a directory
	virtual bool IsDirectory( const char *pFileName, const char *pathID = nullptr ) = 0;

	// path info
	virtual const char *GetLocalPath( const char *pFileName, OUT_Z_CAP( maxLenInChars ) char *pDest, int maxLenInChars ) = 0;
	virtual bool FullPathToRelativePath( const char *pFullpath, OUT_Z_CAP( maxLenInChars ) char *pDest, int maxLenInChars ) = 0;

	// removes a file from disk
	virtual void RemoveFile( char const *pRelativePath, const char *pathID = nullptr ) = 0;

	// Remove all search paths (including write path?)
	virtual void RemoveAllSearchPaths( ) = 0;

	// Purpose: Removes all search paths for a given pathID, such as all "GAME" paths.
	virtual void RemoveSearchPaths( const char *pathID ) = 0;

	// STUFF FROM IFileSystem
	// Add paths in priority order (mod dir, game dir, ....)
	// Can also add pak files (errr, NOT YET!)
	virtual void AddSearchPath( const char *pPath, const char *pathID, SearchPathAdd_t addType ) = 0;
	virtual bool RemoveSearchPath( const char *pPath, const char *pathID ) = 0;
	virtual void PrintSearchPaths( ) = 0;

	virtual void MarkPathIDByRequestOnly( const char *pPathID, bool bRequestOnly ) = 0;

	virtual bool FileExists( const char *pFileName, const char *pPathID = NULL ) = 0;
	virtual long GetFileTime( const char *pFileName, const char *pPathID = NULL ) = 0;
	virtual bool IsFileWritable( char const *pFileName, const char *pPathID = NULL ) = 0;
	virtual bool SetFileWritable( char const *pFileName, bool writable, const char *pPathID = 0 ) = 0;
	virtual void FixUpPath( const char *, char *, int ) = 0;
	virtual void FileTimeToString( char *pString, int maxChars, long fileTime ) = 0;

	virtual const char *FindFirst( const char *pWildCard, FileFindHandle_t *pHandle ) = 0;
	virtual const char *FindFirstEx( const char *pWildCard, const char *pPathID, FileFindHandle_t *pHandle ) = 0;
	virtual const char *FindNext( FileFindHandle_t handle ) = 0;
	virtual bool FindIsDirectory( FileFindHandle_t handle ) = 0;
	virtual void FindClose( FileFindHandle_t handle ) = 0;

	virtual void PrintOpenedFiles( ) = 0;
	virtual void SetWarningFunc( void ( *pfnWarning )( const char *fmt, ... ) ) = 0;
	virtual void SetWarningLevel( FileWarningLevel_t level ) = 0;
	virtual void AddLoggingFunc( FileSystemLoggingFunc_t logFunc ) = 0;
	virtual void RemoveLoggingFunc( FileSystemLoggingFunc_t logFunc ) = 0;
	virtual bool RenameFile( char const *pOldPath, char const *pNewPath, const char *pathID ) = 0;

	virtual void GetLocalCopy( const char *pFileName ) = 0;

	virtual FileNameHandle_t FindOrAddFileName( char const *pFileName ) = 0;
	virtual FileNameHandle_t FindFileName( char const *pFileName ) = 0;
	virtual bool String( const FileNameHandle_t &handle, char *buf, int buflen ) = 0;
	virtual int GetPathIndex( const FileNameHandle_t &handle ) = 0;
	virtual long GetPathTime( const char *pFileName, const char *pPathID ) = 0;

	virtual void EnableWhitelistFileTracking( bool bEnable, bool bCacheAllVPKHashes, bool bRecalculateAndCheckHashes ) = 0;
	virtual void RegisterFileWhitelist( IPureServerWhitelist *pWhiteList, IFileList **pFilesToReload ) = 0;
	virtual	void MarkAllCRCsUnverified( ) = 0;
	virtual void CacheFileCRCs( const char *pPathname, ECacheCRCType eType, IFileList *pFilter ) = 0;
	virtual int GetWhitelistSpewFlags( ) = 0;
	virtual void SetWhitelistSpewFlags( int flags ) = 0;
	virtual void InstallDirtyDiskReportFunc( FSDirtyDiskReportFunc_t func ) = 0;

	// Returns the file system statistics retreived by the implementation.  Returns NULL if not supported.
	virtual const FileSystemStatistics *GetFilesystemStatistics( ) = 0;

	// Load dlls
	virtual CSysModule *LoadModule( const char *pFileName, const char *pPathID, bool bValidatedDllOnly ) = 0;
	virtual void UnloadModule( CSysModule *pModule ) = 0;

	//--------------------------------------------------------
	// asynchronous file loading
	//--------------------------------------------------------
	virtual FSAsyncStatus_t AsyncReadMultiple( const FileAsyncRequest_t *pRequests, int nRequests, FSAsyncControl_t *pControls ) = 0;
	virtual FSAsyncStatus_t AsyncReadMultipleCreditAlloc( const FileAsyncRequest_t *pRequests, int nRequests, const char *pszFile, int line, FSAsyncControl_t *phControls = NULL ) = 0;
	virtual FSAsyncStatus_t AsyncFinish( FSAsyncControl_t hControl, bool wait ) = 0;
	virtual FSAsyncStatus_t AsyncGetResult( FSAsyncControl_t hControl, void **ppData, int *pSize ) = 0;
	virtual FSAsyncStatus_t AsyncAbort( FSAsyncControl_t hControl ) = 0;
	virtual FSAsyncStatus_t AsyncStatus( FSAsyncControl_t hControl ) = 0;
	virtual FSAsyncStatus_t AsyncSetPriority( FSAsyncControl_t hControl, int newPriority ) = 0;
	virtual FSAsyncStatus_t AsyncFlush( ) { return FSASYNC_ERR_FAILURE; }
	virtual FSAsyncStatus_t AsyncAppend( const char *pFileName, const void *pSrc, int nSrcBytes, bool bFreeMemory, FSAsyncControl_t *pControl ) = 0;
	virtual FSAsyncStatus_t AsyncWrite( const char *pFileName, const void *pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t *pControl ) = 0;
	virtual FSAsyncStatus_t AsyncWriteFile( const char *pFileName, const CUtlBuffer *pSrc, int nSrcBytes, bool bFreeMemory, bool bAppend, FSAsyncControl_t *pControl ) = 0;
	virtual FSAsyncStatus_t AsyncAppendFile( const char *pDestFileName, const char *pSrcFileName, FSAsyncControl_t *pControl ) = 0;
	virtual void AsyncFinishAll( int iToPriority = INT_MIN ) = 0;
	virtual void AsyncFinishAllWrites( ) = 0;
	virtual bool AsyncSuspend( ) = 0;
	virtual bool AsyncResume( ) = 0;

	virtual void AsyncAddRef( FSAsyncControl_t hControl ) = 0;
	virtual void AsyncRelease( FSAsyncControl_t hControl ) = 0;
	virtual FSAsyncStatus_t AsyncBeginRead( const char *pszFile, FSAsyncFile_t *phFile ) = 0;
	virtual FSAsyncStatus_t AsyncEndRead( FSAsyncFile_t hFile ) = 0;

	//--------------------------------------------------------
	// pack files
	//--------------------------------------------------------
	virtual bool AddPackFile( const char *pFileName, const char *pathID ) = 0;

	// converts a partial path into a full path
	// can be filtered to restrict path types and can provide info about resolved path
	virtual const char *RelativePathToFullPath( const char *pFileName, const char *pPathID, char *pFullPath, int fullPathBufferSize, PathTypeFilter_t pathFilter = FILTER_NONE, PathTypeQuery_t *pPathType = NULL ) = 0;

	// Returns the search path, each path is separated by ;s. Returns the length of the string returned
	virtual int GetSearchPath( const char *pathID, bool bGetPackFiles, char *pPath, int nMaxLen ) = 0;

	virtual bool GetFileTypeForFullPath( char const *pFullPath, wchar_t *buf, size_t bufSizeInBytes ) = 0;

	virtual void BeginMapAccess( ) = 0;
	virtual void EndMapAccess( ) = 0;
	virtual bool FullPathToRelativePathEx( const char *pFullpath, const char *pPathId, char *pRelative, int maxlen ) = 0;

	virtual void SetupPreloadData( ) = 0;
	virtual void DiscardPreloadData( ) = 0;

	virtual void LoadCompiledKeyValues( KeyValuesPreloadType_t type, char const *archiveFile ) = 0;

	// If the "PreloadedData" hasn't been purged, then this'll try and instance the KeyValues using the fast path of compiled keyvalues loaded during startup.
	// Otherwise, it'll just fall through to the regular KeyValues loading routines
	virtual KeyValues *LoadKeyValues( KeyValuesPreloadType_t type, char const *filename, char const *pPathID = 0 ) = 0;
	virtual bool LoadKeyValues( KeyValues &head, KeyValuesPreloadType_t type, char const *filename, char const *pPathID = 0 ) = 0;
	virtual bool ExtractRootKeyName( KeyValuesPreloadType_t type, char *outbuf, size_t bufsize, char const *filename, char const *pPathID = 0 ) = 0;

	virtual DVDMode_t GetDVDMode( ) = 0;

	class CPathIDInfo
	{
	public:
		bool m_bByRequestOnly;
		CUtlSymbol m_PathID;
		const char *m_pDebugPathID;
	};

	class CSearchPath
	{
	public:
		int32_t m_storeId;
		CPathIDInfo *m_pPathIDInfo;
		uint32_t _flag0;
		CPathPriorityGroup_t m_GroupID;
		CUtlSymbol m_Path;
		const char *m_pDebugPath;
		CPackFile *m_pPackFile;
		CPackedStore *m_pPackFile2;
	};

	struct FindData_t
	{ };

	struct CompiledKeyValuesPreloaders_t
	{
		FileNameHandle_t m_CacheFile;
		class CCompiledKeyValuesReader *m_pReader;
	};

	class COpenedFile
	{
	public:
		FILE *m_pFile;
		char *m_pName;
	};

	CWhitelistHolder m_FileWhitelist;
	CUtlVector<FileSystemLoggingFunc_t> m_LogFuncs;
	CThreadMutex m_SearchPathsMutex;
	CUtlLinkedList<CSearchPath> m_SearchPaths;
	CUtlVector<CPathIDInfo *> m_PathIDInfos;
	CUtlLinkedList<FindData_t> m_FindData;
	int32_t m_iMapLoad;
	CUtlVector<CPackFile *> m_ZipFiles;
	FILE *m_pLogFile;
	bool m_bOutputDebugString;
	IThreadPool *m_pThreadPool;
	CThreadFastMutex m_AsyncCallbackMutex;
	FileSystemStatistics m_Stats;
	CThreadMutex m_OpenedFilesMutex;
	CUtlVector<COpenedFile> m_OpenedFiles;
	FileWarningLevel_t m_fwLevel;
	CUtlFilenameSymbolTable m_FileNames;
	CFileTracker m_FileTracker;
	int32_t m_WhitelistFileTrackingEnabled;
	FSDirtyDiskReportFunc_t m_DirtyDiskReportFunc;
	CompiledKeyValuesPreloaders_t m_PreloadData[IFileSystem::NUM_PRELOAD_TYPES];
};

#endif
