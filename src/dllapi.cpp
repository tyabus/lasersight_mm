/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */

#include "lasersight.h"


lasersight_t	lasersight;


static void ClientPutInServer( edict_t *pEdict )
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	
	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(pEdict);
	
	//call player class function
	pPlayer->PutInServer(pEdict);
	
	RETURN_META(pPlayer->GetMetaResult());
}


static void ClientDisconnect( edict_t *pEdict )
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	
	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(pEdict);
	
	if(!pPlayer->m_connected)
	{
		//Not valid!
		RETURN_META(MRES_IGNORED);
	}

	//call player class function
	pPlayer->Disconnect();
	
	RETURN_META(pPlayer->GetMetaResult());
}

static void PlayerPostThink( edict_t *pEdict )
{	
	if (!gpGlobals->deathmatch || !lasersight.active) RETURN_META(MRES_IGNORED);
	
	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(pEdict);
	
	if(!pPlayer->m_connected)
	{
		//Metamod bot fix
		if(pEdict && pEdict->v.flags & FL_FAKECLIENT)
		{
			//call player class function
			pPlayer->PutInServer(pEdict);
		}
		
		//Not valid!
		RETURN_META(MRES_IGNORED);
	}
	
	//call player class function
	pPlayer->PostThink();
	
	RETURN_META(pPlayer->GetMetaResult());
}


static void ClientCommand( edict_t *pThis )
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	if (!lasersight.allow_to_set_off) RETURN_META(MRES_IGNORED);

	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(pThis);
	
	if(!pPlayer->m_connected)
	{
		//Not valid!
		RETURN_META(MRES_IGNORED);
	}
	
	//call player class function
	pPlayer->Command(CMD_ARGV(0),CMD_ARGV(1));
	
	RETURN_META(pPlayer->GetMetaResult());
}


static int AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, edict_t *pThis, int hostflags, int player, unsigned char *pSet)
{
	if (!gpGlobals->deathmatch || !lasersight.active) RETURN_META_VALUE(MRES_IGNORED,1);
	
	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(pThis);
	
	if(!pPlayer->m_connected)
	{
		//Not valid!
		RETURN_META_VALUE(MRES_IGNORED,0);
	}
	
	//call player class function
	register int ret_val = pPlayer->AddToFullPack_Post(state,e,ent,hostflags,player,pSet);
	
	RETURN_META_VALUE(pPlayer->GetMetaResult(),ret_val);
	return ret_val;
}


static float next_checktime=0;


static void StartFrame(void)
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);

	if (next_checktime<gpGlobals->time)
	{
		//Some ppl might not know that they have to remove old laserbeam.. so just disable it here
		cvar_t *tmp;
		tmp = CVAR_GET_POINTER("laserbeam_on");
		if(tmp) CVAR_SET_STRING("laserbeam_on","0");
		tmp = CVAR_GET_POINTER("laserbeam_beam");
		if(tmp) CVAR_SET_STRING("laserbeam_beam","0");
		tmp = CVAR_GET_POINTER("laserbeam_dot");
		if(tmp) CVAR_SET_STRING("laserbeam_dot","0");
		
		//Get float values
		lasersight.beamsize		= CVAR_GET_FLOAT("lasersight_beamsize");
		lasersight.dotsize		= CVAR_GET_FLOAT("lasersight_dotsize");
		
		//Get int values
		lasersight.debug		= (int)CVAR_GET_FLOAT("lasersight_debug");
		
		//Get bool values
		lasersight.active		= (int)CVAR_GET_FLOAT("lasersight_on")			> 0;
		lasersight.beam_active		= (int)CVAR_GET_FLOAT("lasersight_beam")		> 0;
		lasersight.dot_active		= (int)CVAR_GET_FLOAT("lasersight_dot")			> 0;
		lasersight.allow_to_set_off	= (int)CVAR_GET_FLOAT("lasersight_allowsetoff")		> 0;
		lasersight.show_own_beam	= (int)CVAR_GET_FLOAT("lasersight_show_own_beam")	> 0;
		lasersight.show_own_dot		= (int)CVAR_GET_FLOAT("lasersight_show_own_dot")	> 0;
		lasersight.fix_own_beam_pos	= (int)CVAR_GET_FLOAT("lasersight_fix_own_position")	> 0;
		lasersight.enemysees		= (int)CVAR_GET_FLOAT("lasersight_enemysees")		> 0;
		
		if(!lasersight.beam_active && !lasersight.dot_active) lasersight.active = false;

		//
		//	Remove beams that are not needed
		//
		int i;
		bool remove_beam= (!lasersight.active || !lasersight.beam_active);
		bool remove_dot	= (!lasersight.active || !lasersight.dot_active);
			
		for (i=0;i<33;i++) 
		{
			if (remove_beam) players[i].m_pBeam.Remove();
			if (remove_dot) players[i].m_pDot.Remove();
		}
		
		if (remove_beam	&& remove_dot)
			RETURN_META(MRES_HANDLED);

		for (i=0;i<33;i++)
		{
			//
			//	Clear disconnected players!
			//
			if (!players[i].m_connected) 
			{
				if (!remove_beam) players[i].m_pBeam.Remove();
				if (!remove_dot) players[i].m_pDot.Remove();
			}
		}
		
		next_checktime=gpGlobals->time+0.25f;
	}
	
	RETURN_META(MRES_HANDLED);
}


static int DispatchSpawn( edict_t *pent )
{	
	if (!gpGlobals->deathmatch) RETURN_META_VALUE(MRES_IGNORED, 0);
	
	if (!stricmp(STRING(pent->v.classname), "worldspawn"))
	{
		//precache beam and dot sprites
		lasersight.m_spriteTexture = PRECACHE_MODEL((char*)lasersight.pSpriteName);
		PRECACHE_MODEL((char*)lasersight.pDotSpriteName);
		
		//Clear players
		for (int pid=0;pid<33;pid++) 
		{
			players[pid].m_current_weapon_id	= -1;
			players[pid].m_index			= pid;
			players[pid].m_showbeam			= true;
			players[pid].m_connected		= false;
			players[pid].m_pEdict			= NULL;
			
			players[pid].m_pBeam.SetEdictNull();
			players[pid].m_pDot.SetEdictNull();
		}
 
		next_checktime=gpGlobals->time + (-0.01f);
		
		//Load weapon data
		LoadINIData();
	}
	
	RETURN_META_VALUE(MRES_HANDLED,	0);
	return 0;
}


static edict_t *pent_info_tfdetect = NULL;
static bool is_defaultctf = false;

static void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	
	if (lasersight.mod_id == 2)//TFC
   	{
      		if (pentKeyvalue == pent_info_tfdetect)
      		{
      			if (!stricmp(pkvd->szKeyName, "team1_allies"))       // BLUE allies
				lasersight.team_allies[0] = atoi(pkvd->szValue);
			else if (!stricmp(pkvd->szKeyName, "team2_allies"))  // RED allies
				lasersight.team_allies[1] = atoi(pkvd->szValue);
			else if (!stricmp(pkvd->szKeyName, "team3_allies"))  // YELLOW allies
				lasersight.team_allies[2] = atoi(pkvd->szValue);
			else if (!stricmp(pkvd->szKeyName, "team4_allies"))  // GREEN allies
				lasersight.team_allies[3] = atoi(pkvd->szValue);
      		}
      		else if (pent_info_tfdetect == NULL 
      				&& !strcmp(pkvd->szKeyName, "classname")
      				&& !strcmp(pkvd->szValue, "info_tfdetect") )
         	{
            		pent_info_tfdetect = pentKeyvalue;
         	}
      	}
	else if (lasersight.mod_id == 4 //OPFOR
			&& !is_defaultctf 
			&& !stricmp(pkvd->szKeyName, "defaultctf") 
			&& !stricmp(pkvd->szValue, "1") )
	{
		is_defaultctf = true;

		delete MOD_Handler;
		MOD_Handler = (CBaseMODHandler*)(new CMOD_OP4CTF);
	}

	RETURN_META(MRES_HANDLED);
}


static void SrvDeactive(void)
{
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	
	//Reset MOD Handler
	delete MOD_Handler;
	switch(lasersight.mod_id)
	{
		case 1:
			MOD_Handler = (CBaseMODHandler*)(new CMOD_CounterStrike); 
		break;
		
		case 2:
			MOD_Handler = (CBaseMODHandler*)(new CMOD_TFC);
		break;
		
		case 3:
			MOD_Handler = (CBaseMODHandler*)(new CMOD_DOD);
		break;
	
		case 4:
			MOD_Handler = (CBaseMODHandler*)(new CMODDefault);
		break;
		
		case 5:
			MOD_Handler = (CBaseMODHandler*)(new CMOD_NS);
		break;
		
		default:
			MOD_Handler = (CBaseMODHandler*)(new CMODDefault);
		break;
	}

	is_defaultctf      = false;
	pent_info_tfdetect = NULL;
	
	RETURN_META(MRES_HANDLED);
}


static DLL_FUNCTIONS gFunctionTable = 
{
	NULL,						//! pfnGameInit()
	DispatchSpawn,					//! pfnSpawn()
	NULL,						//! pfnThink()
	NULL,						//! pfnUse()
	NULL,						//! pfnTouch()
	NULL,						//! pfnBlocked()
	DispatchKeyValue,				//! pfnKeyValue()
	NULL,						//! pfnSave()
	NULL,						//! pfnRestore()
	NULL,						//! pfnSetAbsBox()

	NULL,						//! pfnSaveWriteFields()
	NULL,						//! pfnSaveReadFields()

	NULL,						//! pfnSaveGlobalState()
	NULL,						//! pfnRestoreGlobalState()
	NULL,						//! pfnResetGlobalState()

	NULL	,					//! pfnClientConnect()
	ClientDisconnect,				//! pfnClientDisconnect()
	NULL,						//! pfnClientKill()
	ClientPutInServer,				//! pfnClientPutInServer()
	ClientCommand,					//! pfnClientCommand()
	NULL,						//! pfnClientUserInfoChanged()
	NULL,						//! pfnServerActivate()
	SrvDeactive,					//! pfnServerDeactivate()
	
	NULL,						//! pfnPlayerPreThink()
	PlayerPostThink,				//! pfnPlayerPostThink()

	StartFrame,					//! pfnStartFrame()
	NULL,						//! pfnParmsNewLevel()
	NULL,						//! pfnParmsChangeLevel()

	NULL,						//! pfnGetGameDescription()
	NULL,						//! pfnPlayerCustomization()

	NULL,						//! pfnSpectatorConnect()
	NULL,						//! pfnSpectatorDisconnect()
	NULL,						//! pfnSpectatorThink()
	
	NULL,						//! pfnSys_Error()

	NULL,						//! pfnPM_Move()
	NULL,						//! pfnPM_Init()
	NULL,						//! pfnPM_FindTextureType()
	
	NULL,						//! pfnSetupVisibility()
	NULL,						//! pfnUpdateClientData()
	NULL,//AddToFullPack,					//! pfnAddToFullPack()
	NULL,						//! pfnCreateBaseline()
	NULL,						//! pfnRegisterEncoders()
	NULL,						//! pfnGetWeaponData()
	NULL,						//! pfnCmdStart()
	NULL,						//! pfnCmdEnd()
	NULL,						//! pfnConnectionlessPacket()
	NULL,						//! pfnGetHullBounds()
	NULL,						//! pfnCreateInstancedBaselines()
	NULL,						//! pfnInconsistentFile()
	NULL,						//! pfnAllowLagCompensation()
};

static DLL_FUNCTIONS gFunctionTable_Post = 
{
	NULL,						//! pfnGameInit()
	NULL,					//! pfnSpawn()
	NULL,						//! pfnThink()
	NULL,						//! pfnUse()
	NULL,						//! pfnTouch()
	NULL,						//! pfnBlocked()
	NULL,						//! pfnKeyValue()
	NULL,						//! pfnSave()
	NULL,						//! pfnRestore()
	NULL,						//! pfnSetAbsBox()

	NULL,						//! pfnSaveWriteFields()
	NULL,						//! pfnSaveReadFields()

	NULL,						//! pfnSaveGlobalState()
	NULL,						//! pfnRestoreGlobalState()
	NULL,						//! pfnResetGlobalState()

	NULL	,					//! pfnClientConnect()
	NULL,				//! pfnClientDisconnect()
	NULL,						//! pfnClientKill()
	NULL,				//! pfnClientPutInServer()
	NULL,					//! pfnClientCommand()
	NULL,						//! pfnClientUserInfoChanged()
	NULL,						//! pfnServerActivate()
	NULL,//SrvDeactive,					//! pfnServerDeactivate()
	
	NULL,						//! pfnPlayerPreThink()
	NULL,				//! pfnPlayerPostThink()

	NULL,					//! pfnStartFrame()
	NULL,						//! pfnParmsNewLevel()
	NULL,						//! pfnParmsChangeLevel()

	NULL,						//! pfnGetGameDescription()
	NULL,						//! pfnPlayerCustomization()

	NULL,						//! pfnSpectatorConnect()
	NULL,						//! pfnSpectatorDisconnect()
	NULL,						//! pfnSpectatorThink()
	
	NULL,						//! pfnSys_Error()

	NULL,						//! pfnPM_Move()
	NULL,						//! pfnPM_Init()
	NULL,						//! pfnPM_FindTextureType()
	
	NULL,						//! pfnSetupVisibility()
	NULL,						//! pfnUpdateClientData()
	AddToFullPack_Post,					//! pfnAddToFullPack()
	NULL,						//! pfnCreateBaseline()
	NULL,						//! pfnRegisterEncoders()
	NULL,						//! pfnGetWeaponData()
	NULL,						//! pfnCmdStart()
	NULL,						//! pfnCmdEnd()
	NULL,						//! pfnConnectionlessPacket()
	NULL,						//! pfnGetHullBounds()
	NULL,						//! pfnCreateInstancedBaselines()
	NULL,						//! pfnInconsistentFile()
	NULL,						//! pfnAllowLagCompensation()
};

C_DLLEXPORT int	GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion )
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI2; version=%d", *interfaceVersion);
	if(!pFunctionTable) {
		LOG_ERROR(PLID,	"GetEntityAPI2 called with null	pFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != INTERFACE_VERSION)	{
		LOG_ERROR(PLID,	"GetEntityAPI2 version mismatch; requested=%d ours=%d",	*interfaceVersion, INTERFACE_VERSION);
		//! Tell engine	what version we	had, so	it can figure out who is out of	date.
		*interfaceVersion = INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pFunctionTable, &gFunctionTable,	sizeof(DLL_FUNCTIONS));

	return(TRUE);
}

C_DLLEXPORT int	GetEntityAPI2_Post( DLL_FUNCTIONS *pFunctionTable_Post, int *interfaceVersion )
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI2_Post; version=%d", *interfaceVersion);
	if(!pFunctionTable_Post) {
		LOG_ERROR(PLID,	"GetEntityAPI2_Post called with null	pFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != INTERFACE_VERSION)	{
		LOG_ERROR(PLID,	"GetEntityAPI2_Post version mismatch; requested=%d ours=%d",	*interfaceVersion, INTERFACE_VERSION);
		//! Tell engine	what version we	had, so	it can figure out who is out of	date.
		*interfaceVersion = INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pFunctionTable_Post, &gFunctionTable_Post, sizeof(DLL_FUNCTIONS));

	return(TRUE);
}

