#ifndef MDLCACHE_H
#define MDLCACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier1/refcount.h"
#include "vcollide.h"

class CStudioVCollide : public CRefCounted<>
{
public:
	virtual ~CStudioVCollide();

	float m_flScale;
	vcollide_t m_VCollide;
};

#endif
