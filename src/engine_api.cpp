/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#include "lasersight.h"


CBaseMessageHandler* MessageHandler = NULL;
int message_CurWeapon = 0;


static void pfnMessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *pEdict)
{
	//Not in singleplayer
	if (!gpGlobals->deathmatch) RETURN_META(MRES_IGNORED);
	
	//Clear old
	if(MessageHandler != NULL)
		delete MessageHandler;
	MessageHandler = NULL;
	
	//Get message index	
	if (message_CurWeapon == 0) message_CurWeapon = GET_USER_MSG_ID(PLID,"CurWeapon",NULL);

	// check that valid
	if (message_CurWeapon == 0) RETURN_META(MRES_IGNORED);
	
	//only have one message handler at the time
	if (message_CurWeapon == msg_type)
	{
		MessageHandler = (CBaseMessageHandler*)(new CGetCurrentWeaponID);
	
		//If returns false do clean up
		if (!MessageHandler->MessageBegin(msg_dest,msg_type,pOrigin,pEdict))
		{
			META_RES meta_result = MessageHandler->GetMetaResult();
			
			delete MessageHandler;
			MessageHandler = NULL;
		
			RETURN_META(meta_result);
		}
		
		RETURN_META(MessageHandler->GetMetaResult());
	}

	RETURN_META(MRES_IGNORED);
}


static void pfnWriteInt(int iValue)
{
	if (!gpGlobals->deathmatch || MessageHandler==NULL) RETURN_META(MRES_IGNORED);

	//If returns false do clean up
	if (!MessageHandler->MessageWriteInt(iValue))
	{
		META_RES meta_result = MessageHandler->GetMetaResult();
		
		delete MessageHandler;
		MessageHandler = NULL;
		
		RETURN_META(meta_result);
	}
	
	RETURN_META(MessageHandler->GetMetaResult());
}


static void pfnMessageEnd(void)
{
	if (!gpGlobals->deathmatch || MessageHandler==NULL) RETURN_META(MRES_IGNORED);

	MessageHandler->MessageEnd();
	
	META_RES meta_result = MessageHandler->GetMetaResult();
	
	//Do clean up
	delete MessageHandler;
	MessageHandler = NULL;
	
	RETURN_META(meta_result);
}


//Bot PutInServer fix up
static edict_t * CreateFakeClient_Post(const char *netname)
{
	if (!gpGlobals->deathmatch) RETURN_META_VALUE(MRES_IGNORED,NULL);
	
	edict_t *fake_client=META_RESULT_ORIG_RET(edict_t *);
	
	if(FNullEnt(fake_client))
		RETURN_META_VALUE(MRES_IGNORED,NULL);
		
	//Get player pointer
	CPlayer *pPlayer = CPlayer::GetPlayerFromArray(fake_client);
	
	pPlayer->PutInServer(fake_client);
	
	RETURN_META_VALUE(pPlayer->GetMetaResult(),NULL);
	return NULL;
}


static enginefuncs_t _meta_engfuncs = 
{
	NULL,                   // pfnPrecacheModel()
	NULL,                   // pfnPrecacheSound()
	NULL,                   // pfnSetModel()
	NULL,                   // pfnModelIndex()
	NULL,                   // pfnModelFrames()

	NULL,                   // pfnSetSize()
	NULL,                   // pfnChangeLevel()
	NULL,                   // pfnGetSpawnParms()
	NULL,                   // pfnSaveSpawnParms()

	NULL,                   // pfnVecToYaw()
	NULL,                   // pfnVecToAngles()
	NULL,                   // pfnMoveToOrigin()
	NULL,                   // pfnChangeYaw()
	NULL,                   // pfnChangePitch()

	NULL,                   // pfnFindEntityByString()
	NULL,                   // pfnGetEntityIllum()
	NULL,                   // pfnFindEntityInSphere()
	NULL,                   // pfnFindClientInPVS()
	NULL,                   // pfnEntitiesInPVS()

	NULL,                   // pfnMakeVectors()
	NULL,                   // pfnAngleVectors()

	NULL,                   // pfnCreateEntity()
	NULL,                   // pfnRemoveEntity()
	NULL,                   // pfnCreateNamedEntity()

	NULL,                   // pfnMakeStatic()
	NULL,                   // pfnEntIsOnFloor()
	NULL,                   // pfnDropToFloor()

	NULL,                   // pfnWalkMove()
	NULL,                   // pfnSetOrigin()

	NULL,                   // pfnEmitSound()
	NULL,                   // pfnEmitAmbientSound()

	NULL,                   // pfnTraceLine()
	NULL,                   // pfnTraceToss()
	NULL,                   // pfnTraceMonsterHull()
	NULL,                   // pfnTraceHull()
	NULL,                   // pfnTraceModel()
	NULL,                   // pfnTraceTexture()
	NULL,                   // pfnTraceSphere()
	NULL,                   // pfnGetAimVector()

	NULL,                   // pfnServerCommand()
	NULL,                   // pfnServerExecute()
	NULL,                   // pfnClientCommand()

	NULL,                   // pfnParticleEffect()
	NULL,                   // pfnLightStyle()
	NULL,                   // pfnDecalIndex()
	NULL,                   // pfnPointContents()

	pfnMessageBegin,        // pfnMessageBegin()
	pfnMessageEnd,          // pfnMessageEnd()

	pfnWriteInt,            // pfnWriteByte()
	pfnWriteInt,            // pfnWriteChar()
	pfnWriteInt,            // pfnWriteShort()
	pfnWriteInt,            // pfnWriteLong()
	NULL,                   // pfnWriteAngle()
	NULL,                   // pfnWriteCoord()
	NULL,		        // pfnWriteString()
	NULL,                   // pfnWriteEntity()

	NULL,                   // pfnCVarRegister()
	NULL,                   // pfnCVarGetFloat()
	NULL,                   // pfnCVarGetString()
	NULL,                   // pfnCVarSetFloat()
	NULL,                   // pfnCVarSetString()

	NULL,                   // pfnAlertMessage()
	NULL,                   // pfnEngineFprintf()

	NULL,                   // pfnPvAllocEntPrivateData()
	NULL,                   // pfnPvEntPrivateData()
	NULL,                   // pfnFreeEntPrivateData()

	NULL,                   // pfnSzFromIndex()
	NULL,                   // pfnAllocString()

	NULL,                   // pfnGetVarsOfEnt()
	NULL,                   // pfnPEntityOfEntOffset()
	NULL,                   // pfnEntOffsetOfPEntity()
	NULL,                   // pfnIndexOfEdict()
	NULL,                   // pfnPEntityOfEntIndex()
	NULL,                   // pfnFindEntityByVars()
	NULL,                   // pfnGetModelPtr()

	NULL,                   // pfnRegUserMsg()

	NULL,                   // pfnAnimationAutomove()
	NULL,                   // pfnGetBonePosition()

	NULL,                   // pfnFunctionFromName()
	NULL,                   // pfnNameForFunction()

	NULL,                   // pfnClientPrintf()                    //! JOHN: engine callbacks so game DLL can print messages to individual clients
	NULL,                   // pfnServerPrint()

	NULL,                   // pfnCmd_Args()        //! these 3 added 
	NULL,                   // pfnCmd_Argv()        //! so game DLL can easily 
	NULL,                   // pfnCmd_Argc()        //! access client 'cmd' strings

	NULL,                   // pfnGetAttachment()

	NULL,                   // pfnCRC32_Init()
	NULL,                   // pfnCRC32_ProcessBuffer()
	NULL,                   // pfnCRC32_ProcessByte()
	NULL,                   // pfnCRC32_Final()

	NULL,                   // pfnRandomLong()
	NULL,                   // pfnRandomFloat()

	NULL,                   // pfnSetView()
	NULL,                   // pfnTime()
	NULL,                   // pfnCrosshairAngle()

	NULL,                   // pfnLoadFileForMe()
	NULL,                   // pfnFreeFile()

	NULL,                   // pfnEndSection()                              //! trigger_endsection
	NULL,                   // pfnCompareFileTime()
	NULL,                   // pfnGetGameDir()
	NULL,                   // pfnCvar_RegisterVariable()
	NULL,                   // pfnFadeClientVolume()
	NULL,                   // pfnSetClientMaxspeed()
	NULL,		        // pfnCreateFakeClient()                //! returns NULL if fake client can't be created
	NULL,                   // pfnRunPlayerMove()
	NULL,                   // pfnNumberOfEntities()

	NULL,                   // pfnGetInfoKeyBuffer()                //! passing in NULL gets the serverinfo
	NULL,                   // pfnInfoKeyValue()
	NULL,                   // pfnSetKeyValue()
	NULL,                   // pfnSetClientKeyValue()
	
	NULL,                   // pfnIsMapValid()
	NULL,                   // pfnStaticDecal()
	NULL,                   // pfnPrecacheGeneric()
	NULL,                   // pfnGetPlayerUserId()                 //! returns the server assigned userid for this player.
	NULL,                   // pfnBuildSoundMsg()
	NULL,                   // pfnIsDedicatedServer()               //! is this a dedicated server?
	NULL,                   // pfnCVarGetPointer()
	NULL,                   // pfnGetPlayerWONId()                  //! returns the server assigned WONid for this player.

	//! YWB 8/1/99 TFF Physics additions
	NULL,                   // pfnInfo_RemoveKey()
	NULL,                   // pfnGetPhysicsKeyValue()
	NULL,                   // pfnSetPhysicsKeyValue()
	NULL,                   // pfnGetPhysicsInfoString()
	NULL,                   // pfnPrecacheEvent()
	NULL,                   // pfnPlaybackEvent()

	NULL,                   // pfnSetFatPVS()
	NULL,                   // pfnSetFatPAS()

	NULL,                   // pfnCheckVisibility()

	NULL,                   // pfnDeltaSetField()
	NULL,                   // pfnDeltaUnsetField()
	NULL,                   // pfnDeltaAddEncoder()
	NULL,                   // pfnGetCurrentPlayer()
	NULL,                   // pfnCanSkipPlayer()
	NULL,                   // pfnDeltaFindField()
	NULL,                   // pfnDeltaSetFieldByIndex()
	NULL,                   // pfnDeltaUnsetFieldByIndex()

	NULL,                   // pfnSetGroupMask()

	NULL,                   // pfnCreateInstancedBaseline()         // d'oh, CreateInstancedBaseline in dllapi too
	NULL,                   // pfnCvar_DirectSet()

	NULL,                   // pfnForceUnmodified()

	NULL,                   // pfnGetPlayerStats()

	NULL,                   // pfnAddServerCommand()
};


static enginefuncs_t _meta_engfuncs_Post = 
{
	NULL,                   // pfnPrecacheModel()
	NULL,                   // pfnPrecacheSound()
	NULL,                   // pfnSetModel()
	NULL,                   // pfnModelIndex()
	NULL,                   // pfnModelFrames()

	NULL,                   // pfnSetSize()
	NULL,                   // pfnChangeLevel()
	NULL,                   // pfnGetSpawnParms()
	NULL,                   // pfnSaveSpawnParms()

	NULL,                   // pfnVecToYaw()
	NULL,                   // pfnVecToAngles()
	NULL,                   // pfnMoveToOrigin()
	NULL,                   // pfnChangeYaw()
	NULL,                   // pfnChangePitch()

	NULL,                   // pfnFindEntityByString()
	NULL,                   // pfnGetEntityIllum()
	NULL,                   // pfnFindEntityInSphere()
	NULL,                   // pfnFindClientInPVS()
	NULL,                   // pfnEntitiesInPVS()

	NULL,                   // pfnMakeVectors()
	NULL,                   // pfnAngleVectors()

	NULL,                   // pfnCreateEntity()
	NULL,                   // pfnRemoveEntity()
	NULL,                   // pfnCreateNamedEntity()

	NULL,                   // pfnMakeStatic()
	NULL,                   // pfnEntIsOnFloor()
	NULL,                   // pfnDropToFloor()

	NULL,                   // pfnWalkMove()
	NULL,                   // pfnSetOrigin()

	NULL,                   // pfnEmitSound()
	NULL,                   // pfnEmitAmbientSound()

	NULL,                   // pfnTraceLine()
	NULL,                   // pfnTraceToss()
	NULL,                   // pfnTraceMonsterHull()
	NULL,                   // pfnTraceHull()
	NULL,                   // pfnTraceModel()
	NULL,                   // pfnTraceTexture()
	NULL,                   // pfnTraceSphere()
	NULL,                   // pfnGetAimVector()

	NULL,                   // pfnServerCommand()
	NULL,                   // pfnServerExecute()
	NULL,                   // pfnClientCommand()

	NULL,                   // pfnParticleEffect()
	NULL,                   // pfnLightStyle()
	NULL,                   // pfnDecalIndex()
	NULL,                   // pfnPointContents()

	NULL,                   // pfnMessageBegin()
	NULL,          		// pfnMessageEnd()

	NULL,            	// pfnWriteByte()
	NULL,            	// pfnWriteChar()
	NULL,            	// pfnWriteShort()
	NULL,            	// pfnWriteLong()
	NULL,                   // pfnWriteAngle()
	NULL,                   // pfnWriteCoord()
	NULL,		        // pfnWriteString()
	NULL,                   // pfnWriteEntity()

	NULL,                   // pfnCVarRegister()
	NULL,                   // pfnCVarGetFloat()
	NULL,                   // pfnCVarGetString()
	NULL,                   // pfnCVarSetFloat()
	NULL,                   // pfnCVarSetString()

	NULL,                   // pfnAlertMessage()
	NULL,                   // pfnEngineFprintf()

	NULL,                   // pfnPvAllocEntPrivateData()
	NULL,                   // pfnPvEntPrivateData()
	NULL,                   // pfnFreeEntPrivateData()

	NULL,                   // pfnSzFromIndex()
	NULL,                   // pfnAllocString()

	NULL,                   // pfnGetVarsOfEnt()
	NULL,                   // pfnPEntityOfEntOffset()
	NULL,                   // pfnEntOffsetOfPEntity()
	NULL,                   // pfnIndexOfEdict()
	NULL,                   // pfnPEntityOfEntIndex()
	NULL,                   // pfnFindEntityByVars()
	NULL,                   // pfnGetModelPtr()

	NULL,                   // pfnRegUserMsg()

	NULL,                   // pfnAnimationAutomove()
	NULL,                   // pfnGetBonePosition()

	NULL,                   // pfnFunctionFromName()
	NULL,                   // pfnNameForFunction()

	NULL,                   // pfnClientPrintf()                    //! JOHN: engine callbacks so game DLL can print messages to individual clients
	NULL,                   // pfnServerPrint()

	NULL,                   // pfnCmd_Args()        //! these 3 added 
	NULL,                   // pfnCmd_Argv()        //! so game DLL can easily 
	NULL,                   // pfnCmd_Argc()        //! access client 'cmd' strings

	NULL,                   // pfnGetAttachment()

	NULL,                   // pfnCRC32_Init()
	NULL,                   // pfnCRC32_ProcessBuffer()
	NULL,                   // pfnCRC32_ProcessByte()
	NULL,                   // pfnCRC32_Final()

	NULL,                   // pfnRandomLong()
	NULL,                   // pfnRandomFloat()

	NULL,                   // pfnSetView()
	NULL,                   // pfnTime()
	NULL,                   // pfnCrosshairAngle()

	NULL,                   // pfnLoadFileForMe()
	NULL,                   // pfnFreeFile()

	NULL,                   // pfnEndSection()                              //! trigger_endsection
	NULL,                   // pfnCompareFileTime()
	NULL,                   // pfnGetGameDir()
	NULL,                   // pfnCvar_RegisterVariable()
	NULL,                   // pfnFadeClientVolume()
	NULL,                   // pfnSetClientMaxspeed()
	CreateFakeClient_Post,  // pfnCreateFakeClient()                //! returns NULL if fake client can't be created
	NULL,                   // pfnRunPlayerMove()
	NULL,                   // pfnNumberOfEntities()

	NULL,                   // pfnGetInfoKeyBuffer()                //! passing in NULL gets the serverinfo
	NULL,                   // pfnInfoKeyValue()
	NULL,                   // pfnSetKeyValue()
	NULL,                   // pfnSetClientKeyValue()
	
	NULL,                   // pfnIsMapValid()
	NULL,                   // pfnStaticDecal()
	NULL,                   // pfnPrecacheGeneric()
	NULL,                   // pfnGetPlayerUserId()                 //! returns the server assigned userid for this player.
	NULL,                   // pfnBuildSoundMsg()
	NULL,                   // pfnIsDedicatedServer()               //! is this a dedicated server?
	NULL,                   // pfnCVarGetPointer()
	NULL,                   // pfnGetPlayerWONId()                  //! returns the server assigned WONid for this player.

	//! YWB 8/1/99 TFF Physics additions
	NULL,                   // pfnInfo_RemoveKey()
	NULL,                   // pfnGetPhysicsKeyValue()
	NULL,                   // pfnSetPhysicsKeyValue()
	NULL,                   // pfnGetPhysicsInfoString()
	NULL,                   // pfnPrecacheEvent()
	NULL,                   // pfnPlaybackEvent()

	NULL,                   // pfnSetFatPVS()
	NULL,                   // pfnSetFatPAS()

	NULL,                   // pfnCheckVisibility()

	NULL,                   // pfnDeltaSetField()
	NULL,                   // pfnDeltaUnsetField()
	NULL,                   // pfnDeltaAddEncoder()
	NULL,                   // pfnGetCurrentPlayer()
	NULL,                   // pfnCanSkipPlayer()
	NULL,                   // pfnDeltaFindField()
	NULL,                   // pfnDeltaSetFieldByIndex()
	NULL,                   // pfnDeltaUnsetFieldByIndex()

	NULL,                   // pfnSetGroupMask()

	NULL,                   // pfnCreateInstancedBaseline()         // d'oh, CreateInstancedBaseline in dllapi too
	NULL,                   // pfnCvar_DirectSet()

	NULL,                   // pfnForceUnmodified()

	NULL,                   // pfnGetPlayerStats()

	NULL,                   // pfnAddServerCommand()
};


C_DLLEXPORT int GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion ) 
{
	if(!pengfuncsFromEngine) {
		UTIL_LogPrintf("%s: GetEngineFunctions called with null pengfuncsFromEngine", Plugin_info.logtag);
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		UTIL_LogPrintf("%s: GetEngineFunctions version mismatch; requested=%d ours=%d", Plugin_info.logtag, *interfaceVersion, ENGINE_INTERFACE_VERSION);
		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pengfuncsFromEngine, &_meta_engfuncs, sizeof(enginefuncs_t));
	return TRUE;
}


C_DLLEXPORT int GetEngineFunctions_Post(enginefuncs_t *pengfuncsFromEngine_Post, int *interfaceVersion ) 
{
	if(!pengfuncsFromEngine_Post) {
		UTIL_LogPrintf("%s: GetEngineFunctions_Post called with null pengfuncsFromEngine_Post", Plugin_info.logtag);
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		UTIL_LogPrintf("%s: GetEngineFunctions_Post version mismatch; requested=%d ours=%d", Plugin_info.logtag, *interfaceVersion, ENGINE_INTERFACE_VERSION);
		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pengfuncsFromEngine_Post, &_meta_engfuncs_Post, sizeof(enginefuncs_t));
	return TRUE;
}

