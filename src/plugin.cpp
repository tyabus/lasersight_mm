/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    You are allowed to copy/modify this file freely. I don't care :)
 */

#include "lasersight.h"


//Mod handler class
CBaseMODHandler* MOD_Handler = 0;


// Plugin startup.  Register commands and cvars.
static cvar_t lasersight_cvars[] = {
	{"lasersight_on",		"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_fix_own_position",	"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_enemysees",	"0",     FCVAR_EXTDLL, 1, NULL},

	{"lasersight_beam",		"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_beamsize",		"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_show_own_beam",	"0",	 FCVAR_EXTDLL, 0, NULL},

	{"lasersight_dot",		"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_dotsize",		"0.5",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_show_own_dot",	"1",	 FCVAR_EXTDLL, 1, NULL},

	{"lasersight_debug",		"0",	 FCVAR_EXTDLL, 0, NULL},

	{"lasersight_allowsetoff",	"1",	 FCVAR_EXTDLL, 1, NULL},
	{"lasersight_hidingbeams",	"Hiding lasersights (other people still can see your beam)",	FCVAR_EXTDLL|FCVAR_PRINTABLEONLY, 0, NULL},
	{"lasersight_showingbeams",	"Showing lasersights",	 					FCVAR_EXTDLL|FCVAR_PRINTABLEONLY, 0, NULL},

	{"lasersight_version",		VVERSION,FCVAR_EXTDLL|FCVAR_SERVER, 0, NULL},
	
	{NULL,NULL,0,0,NULL},
};


void plugin_init(void) 
{
	LOG_MESSAGE(PLID, "%s v%s, %s", Plugin_info.name, Plugin_info.version, Plugin_info.date);
	LOG_MESSAGE(PLID, "by %s", Plugin_info.author);
	LOG_MESSAGE(PLID, "   %s", Plugin_info.url);
	LOG_MESSAGE(PLID, "compiled: "__DATE__ "," __TIME__" EET");
	
	
	//Register list of cvars
	for(int i = 0; lasersight_cvars[i].name; i++ )
		CVAR_REGISTER( &lasersight_cvars[i] );
	
	//Register command that lists of currently loaded weapon data
	REG_SVR_COMMAND("lasersight_show_ini", &ShowWeaponData);
	REG_SVR_COMMAND("lasersight_reload_ini", &ReloadWeaponData);
	
	
	lasersight.beamsize = 
		lasersight.dotsize = 1.0f;
	
	lasersight.active = 
		lasersight.dot_active = 
		lasersight.beam_active = 
		lasersight.allow_to_set_off = 
		lasersight.show_own_dot = 
		lasersight.fix_own_beam_pos = true;

	
	lasersight.show_own_beam =
		lasersight.enemysees = false;
	
	lasersight.debug = 0;
	
	lasersight.AmmoDetectAsFlag = false;
	lasersight.DefaultAttachment = -1;
	
	lasersight.mod_dir = NULL;
		
	lasersight.m_spriteTexture	= 0;
	lasersight.pSpriteName		= "sprites/laserbeam.spr";
	lasersight.pDotSpriteName	= "sprites/dot.spr";
	
	
	//Get mod_dir
	{
		int pos;
		char game_dir[256];
		char mod_name[256];
		
		// find the directory name of the currently running MOD...
		(*g_engfuncs.pfnGetGameDir)(game_dir);
		
		pos = strlen(game_dir) - 1;
		
		// scan backwards till first directory separator...
		while ((pos) && (game_dir[pos] != '/'))
			pos--;
		
		if (pos) pos++;
		
		STRNCPY(mod_name, &game_dir[pos], 256);
		
		lasersight.mod_dir=strdup(mod_name);
	}
	
	//Select right mod class
	if(!stricmp(lasersight.mod_dir,"cstrike"))
	{
		lasersight.mod_id = 1;
		MOD_Handler = (CBaseMODHandler*)(new CMOD_CounterStrike);
	}
	else if(!stricmp(lasersight.mod_dir,"tfc"))
	{
		lasersight.mod_id = 2;
		MOD_Handler = (CBaseMODHandler*)(new CMOD_TFC);
	}
	else if(!stricmp(lasersight.mod_dir,"dod"))
	{
		lasersight.mod_id = 3;
		MOD_Handler = (CBaseMODHandler*)(new CMOD_DOD);
	}
	else if(!stricmp(lasersight.mod_dir,"gearbox"))
	{
		lasersight.mod_id = 4;
		MOD_Handler = (CBaseMODHandler*)(new CMODDefault);
	}
	else if(!stricmp(lasersight.mod_dir,"ns"))
	{
		lasersight.mod_id = 5;
		MOD_Handler = (CBaseMODHandler*)(new CMOD_NS);
	}
	else
	{
		lasersight.mod_id = UNKNOWN_MOD;
		MOD_Handler = (CBaseMODHandler*)(new CMODDefault);
	}
}
