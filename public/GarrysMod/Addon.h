#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <list>

class CSteamAPIContext;
class IAddonDownloadNotification;
struct SteamUGCDetails_t;

namespace IAddonSystem
{
	struct Information
	{
		std::string title;
		std::string file;
		std::string tags;
		std::string placeholder1;
		uint64_t time_updated;
		uint64_t wsid;
		uint64_t creator;
		uint64_t hcontent_file;
		uint64_t size;
		uint64_t hcontent_preview;
		uint32_t timeadded;
	};

	struct UGCInfo
	{
		std::string title;
		std::string file;
		std::string placeholder1;
		uint64_t wsid;
		uint64_t creator;
		uint32_t pubdate;
	};

	struct AddonSource; // unknown
}

namespace Addon
{
	namespace Job
	{
		class Base;
	}

	class FileSystem
	{
		public:
			virtual void Clear( ) = 0;
			virtual void Refresh( ) = 0;
			virtual int MountFile( const std::string& gmaPath, std::vector<std::string>* files, uint64_t, uint64_t, IAddonSystem::AddonSource) = 0;
			virtual bool ShouldMount( uint64_t ) = 0;
			virtual void SetShouldMount( uint64_t, bool ) = 0;
			virtual bool Save( ) = 0;
			virtual const std::list<IAddonSystem::Information> &GetList( ) const = 0;
			virtual const std::list<IAddonSystem::UGCInfo> &GetUGCList( ) const = 0;
			virtual void ScanForSubscriptions( CSteamAPIContext *, const char *, bool ) = 0;
			virtual void Think( ) = 0;
			virtual void SetDownloadNotify( IAddonDownloadNotification * ) = 0;
			virtual int Notify( ) = 0;
			virtual bool IsSubscribed( uint64_t ) = 0;
			virtual const IAddonSystem::Information *FindFileOwner( const std::string & ) = 0;
			virtual void AddAddon( const IAddonSystem::Information & ) = 0;
			virtual void ClearUnusedGMAs( ) = 0;
			virtual const std::string& GetAddonFilepath( uint64_t, bool ) = 0;
			virtual void UnmountAddon( uint64_t, const char* ) = 0;
			virtual void UnmountServerAddons( ) = 0;
			virtual void IsAddonValidPreInstall( SteamUGCDetails_t ) = 0;
			virtual void Shutdown( ) = 0;
			virtual void AddJob( Addon::Job::Base * ) = 0;
			virtual const std::list<SteamUGCDetails_t> &GetSubList( ) const = 0;
			virtual void MountFloatingAddons( ) = 0;
			virtual void AddAddonFromSteamDetails( const SteamUGCDetails_t & ) = 0;
			virtual void OnAddonSubscribed( const SteamUGCDetails_t & ) = 0;
			virtual void AddUnloadedSubscription( uint64_t ) = 0;
			virtual bool HasChanges( ) = 0;
			virtual void MarkChanged( ) = 0;
			virtual void OnAddonDownloaded( const IAddonSystem::Information & ) = 0;
			virtual void OnAddonDownloadFailed( const IAddonSystem::Information & ) = 0;
			virtual void Load( ) = 0;
	};

}