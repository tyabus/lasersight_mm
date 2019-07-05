/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef Lasersight_H
#define Lasersight_H
 
 
#include <extdll.h>
#include <sdk_util.h>
#include <dllapi.h>
#include <meta_api.h>
#include <support_meta.h>
#include <entity_state.h>
#include "vers_plugin.h"


//cvar value storage
class lasersight_t
{
public:
	lasersight_t() 
	{
		memset(this,0,sizeof(lasersight_t));
	};
	
	float beamsize;		// Width modifier for beam
	float dotsize;		// Size modifier for dot
	
	bool active;		// Plugin active
	bool dot_active;	// Dots active
	bool beam_active;	// Beams active

	bool enemysees;		// Does enemies see players' beams
	bool allow_to_set_off;	// Allows player to use hidebeam/showbeam commands
	bool show_own_beam;	// Show player's own beam
	bool show_own_dot;	// Show player's own dot
	bool fix_own_beam_pos;	// Fix position of own beam to screencenter
	
	int debug;		// Debug level
	
	//These are not cvars:
	bool 		AmmoDetectAsFlag;// ini setting
	int		DefaultAttachment;// ini setting
	int		DefaultOffsetForward;
	int		DefaultOffsetRight;
	int		DefaultOffsetUp;
	int		DefaultFlipFlag;
	int		DefaultColor[3];
	int		DefaultWidth;
	const char *	mod_dir;
	
	//Model stuff
	int		m_spriteTexture;
	const char *	pSpriteName;
	const char *	pDotSpriteName;
	
	int mod_id;
	
	//For TFC team allies
	int team_allies[4];
};


extern lasersight_t lasersight;


#include "claserinfo.h"
#include "centityhandlers.h"
#include "cplayer.h"


inline char *STRNCAT(char *dst, const char *src, int size)
{
	return strncat(dst,src,size-1); //why -1? .. see support_meta.h notes about strncat
}


void LoadINIData(void);
void plugin_init(void);


#define UTIL_PointContents(vec)				POINT_CONTENTS(vec)
#define	UTIL_MakeVectors(vecAngles)			MAKE_VECTORS(vecAngles)


void Unbreakable_Traceline(const Vector &vecStart, const Vector &vecEnd, edict_t* pIgnore, TraceResult *ptr);


void ShowWeaponData(void);
void ReloadWeaponData(void);


#include "cmessagehandlers.h"
#include "cmodhandlers.h"


extern CBaseMODHandler* MOD_Handler;


#endif /*Lasersight_H*/


