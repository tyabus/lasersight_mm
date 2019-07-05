/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    You are allowed to copy/modify this file freely. I don't care :)
 */

#include "lasersight.h"


// Must	provide	at least one of	these..
static META_FUNCTIONS gMetaFunctionTable = 
{
	NULL,			// pfnGetEntityAPI			HL SDK;		called before game DLL
	NULL,			// pfnGetEntityAPI_Post			META;		called after game DLL
	GetEntityAPI2,		// pfnGetEntityAPI2			HL SDK2;	called before game DLL
	GetEntityAPI2_Post,	// pfnGetEntityAPI2_Post		META;		called after game DLL
	NULL,			// pfnGetNewDLLFunctions		HL SDK2;	called before game DLL
	NULL,			// pfnGetNewDLLFunctions_Post		META;		called after game DLL
	GetEngineFunctions,	// pfnGetEngineFunctions		META;		called before HL engine
	GetEngineFunctions_Post,// pfnGetEngineFunctions_Post		META;		called after HL	engine
};


void UTIL_LogPrintf( char *fmt, ... )
{
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf( string, 1024, fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	ALERT( at_logged, "%s", string );
}


// Description of plugin.
// (V* info from vers_plugin.h)
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION, // ifvers
	VNAME,			// name
	VVERSION,		// version
	VDATE,			// date
	VAUTHOR,		// author
	VURL,			// url
	VLOGTAG,		// logtag
	PT_CHANGELEVEL,		// loadable
	PT_STARTUP,		// unloadable
};


// Global variables from metamod.  These variable names are referenced by
// various macros.
meta_globals_t *gpMetaGlobals;		// metamod globals
gamedll_funcs_t *gpGamedllFuncs;	// gameDLL function tables
mutil_funcs_t *gpMetaUtilFuncs;		// metamod utility functions


// Metamod requesting info about this plugin
//  ifvers			(given) interface_version metamod is using
//  pPlugInfo		(requested) struct with info about plugin
//  pMetaUtilFuncs	(given) table of utility functions provided by metamod
// Metamod requesting info about this plugin
//  ifvers		(given) interface_version metamod is using
//  pPlugInfo		(requested) struct with info about plugin
//  pMetaUtilFuncs	(given) table of utility functions provided by metamod
#ifndef __BORLANDC__
	C_DLLEXPORT int Meta_Query
#else
	int _Meta_Query
#endif
(char *ifvers, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs) 
{
	if ((int) CVAR_GET_FLOAT("developer") != 0)
		UTIL_LogPrintf("[%s] dev: called: Meta_Query; version=%s, ours=%s\n", 
				Plugin_info.logtag, ifvers, Plugin_info.ifvers);

	// Check for valid pMetaUtilFuncs before we continue.
	if(!pMetaUtilFuncs) {
		UTIL_LogPrintf("[%s] ERROR: Meta_Query called with null pMetaUtilFuncs\n", Plugin_info.logtag);
		return(FALSE);
	}
	gpMetaUtilFuncs=pMetaUtilFuncs;

	// Give metamod our plugin_info struct.
	*pPlugInfo=&Plugin_info;

	// Check for interface version compatibility.
	if(!FStrEq(ifvers, Plugin_info.ifvers)) 
	{
		int mmajor=0, mminor=0, pmajor=0, pminor=0;
		// If plugin has later interface version, it's incompatible (update
		// metamod).
		sscanf(ifvers, "%d:%d", &mmajor, &mminor);
		sscanf(META_INTERFACE_VERSION, "%d:%d", &pmajor, &pminor);
		if(pmajor > mmajor || (pmajor==mmajor && (pminor > mminor))) {
			LOG_MESSAGE(PLID, "WARNING: meta-interface version mismatch; requested=%s ours=%s",Plugin_info.logtag, ifvers);
			LOG_ERROR(PLID, "metamod version is too old for this plugin; update metamod");
			return(FALSE);
		}
		// If plugin has older major interface version, it's incompatible
		// (update plugin).
		else if(pmajor < mmajor) {
			LOG_MESSAGE(PLID, "WARNING: meta-interface version mismatch; requested=%s ours=%s",Plugin_info.logtag, ifvers);
			LOG_ERROR(PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
			return(FALSE);
		}
		// Minor interface is older, but this is guaranteed to be backwards
		// compatible, so we warn, but we still accept it.
		else if(pmajor==mmajor && pminor < mminor)
		{
			//LOG_MESSAGE(PLID, "WARNING: meta-interface version mismatch; requested=%s ours=%s",Plugin_info.logtag, ifvers);
			//LOG_MESSAGE(PLID, "WARNING: metamod version is newer than expected; consider finding a newer version of this plugin");
			
			/*	Don't show version warnings if its compatible!		*/
			Plugin_info.ifvers=strdup(ifvers);
		}
		else
		{
			LOG_MESSAGE(PLID, "WARNING: meta-interface version mismatch; requested=%s ours=%s",Plugin_info.logtag, ifvers);
			LOG_ERROR(PLID, "unexpected version comparison; metavers=%s, mmajor=%d, mminor=%d; plugvers=%s, pmajor=%d, pminor=%d", ifvers, mmajor, mminor, META_INTERFACE_VERSION, pmajor, pminor);
		}
	}
	return(TRUE);
}


// Metamod attaching plugin to the server.
//  now				(given) current phase, ie during map, during changelevel, or at startup
//  pFunctionTable	(requested) table of function tables this plugin catches
//  pMGlobals		(given) global vars from metamod
//  pGamedllFuncs	(given) copy of function tables from game dll
#ifndef __BORLANDC__
	C_DLLEXPORT int Meta_Attach
#else
	int _Meta_Attach
#endif
(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, 
		meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
	if(now > Plugin_info.loadable) {
		LOG_ERROR(PLID, "Can't load plugin right now");
		return(FALSE);
	}
	if(!pMGlobals) {
		LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
		return(FALSE);
	}
	gpMetaGlobals=pMGlobals;
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
		return(FALSE);
	}
	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
	gpGamedllFuncs=pGamedllFuncs;
	// Let's go.
	plugin_init();
	return(TRUE);
}


// Metamod detaching plugin from the server.
// now		(given) current phase, ie during map, etc
// reason	(given) why detaching (refresh, console unload, forced unload, etc)
#ifndef __BORLANDC__
	C_DLLEXPORT int Meta_Detach
#else
	int _Meta_Detach
#endif
(PLUG_LOADTIME now, PL_UNLOAD_REASON reason) {
	if(now > Plugin_info.unloadable && reason != PNL_CMD_FORCED) {
		LOG_ERROR(PLID, "Can't unload plugin right now");
		return(FALSE);
	}
	
	delete MOD_Handler;
	
	return(TRUE);
}


#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved){return TRUE;}
#endif


//! Holds engine functionality callbacks
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;


// Receive engine function table from engine.
// This appears to be the _first_ DLL routine called by the engine, so we
// do some setup operations here.
#ifdef __BORLANDC__
	void _GiveFnptrsToDll
#elif defined(__MINGW32__)
	C_DLLEXPORT void DLLEXPORT WINAPI GiveFnptrsToDll
#else
	C_DLLEXPORT void GiveFnptrsToDll
#endif
( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
	// NOTE!  Have to call logging function _after_ copying into g_engfuncs, so
	// that g_engfuncs.pfnAlertMessage() can be resolved properly, heh. :)
	UTIL_LogPrintf("[%s] dev: called: GiveFnptrsToDll\n", Plugin_info.logtag);
}

