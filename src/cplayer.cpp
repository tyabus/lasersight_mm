/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */

#include "lasersight.h"


//players array .. index 0 is used for invalid entities
CPlayer players[33];


void CPlayer::PutInServer(edict_t *pThis)
{
	//Clear beam&dot
	RemoveLasersightEntities();
	
	// If index is 0 pThis is invalid :(
	// Don't think proxies as players
	if(m_index==0 || pThis->v.flags & FL_PROXY) 
	{
		m_connected = false;
		m_pEdict    = NULL;
		
		SetMetaResult(MRES_IGNORED);
		return;
	}
	
	//Set defaults
	m_connected = true;
	m_showbeam  = true;
	m_pEdict    = pThis;
	
	//No weapon selected
	m_current_weapon_id = -1;
	
	SetMetaResult(MRES_HANDLED);
	return;
}


void CPlayer::Disconnect()
{
	//Mark disconnected
	m_connected = false;
	m_pEdict = NULL;
	
	//Remove beam&dot
	RemoveLasersightEntities();
	
	//No weapon selected
	m_current_weapon_id = -1;
	
	SetMetaResult(MRES_HANDLED);
	return;
}


void CPlayer::PostThink()
{
	//Default return type for checks at start
	SetMetaResult(MRES_IGNORED);

	//Check	if dead 
	if (!IsAlive())
	{
		//Remove beam
		RemoveLasersightEntities();
	
		return;
	}
	
	//Check if player has weapon selected
	if(m_current_weapon_id < 0 || m_current_weapon_id > 31)
	{
		//Remove beam
		RemoveLasersightEntities();
	
		return;
	}

	//Get pointer
	laserinfo_t &weapon_info = WeaponLaserInfo[m_current_weapon_id];
	
	//Check	if this	weapon should have lasersight
	if (!weapon_info.IsUsed())
	{
		//Remove beam
		RemoveLasersightEntities();
	
		return;
	}

	//Check if mod setup wants to disable beam for this player
	if(!MOD_Handler->ShouldPlayerHaveBeam(this))
	{
		//Remove beam
		RemoveLasersightEntities();
	
		return;
	}

	//Make direction vectors
	UTIL_MakeVectors(m_pEdict->v.v_angle);
	
	TraceResult tr;
	
	//Start	of lasersight
	Vector vecOrigSrc = (m_pEdict->v.origin + m_pEdict->v.view_ofs);//GetGunPos
	
	//End of lasersight (vec_dest)
	Vector wepOffs = 	gpGlobals->v_forward 	*	weapon_info.GetForwardOffset()	+ 
				gpGlobals->v_up 	*	weapon_info.GetUpOffset()	+ 
				gpGlobals->v_right 	*	weapon_info.GetRightOffset();
	Vector vecWeaponOrig = vecOrigSrc + wepOffs;
	Vector vecDest = vecWeaponOrig + gpGlobals->v_forward *	8192;
	
	if (UTIL_PointContents(vecWeaponOrig)==CONTENTS_SOLID)
	{
		tr.vecEndPos = vecWeaponOrig;
	}
	else
	{
		//trace line from start to end
		Unbreakable_Traceline( vecWeaponOrig, vecDest, m_pEdict, &tr);
	}
	
	//Update beam data
	if(lasersight.beam_active)
	{
		m_pBeam.SetColor	(weapon_info.GetColor()		);
		m_pBeam.SetWidth	(weapon_info.GetWidth()		);
		m_pBeam.SetPosition	(tr.vecEndPos, vecWeaponOrig	);
		m_pBeam.SetAttachment	( (weapon_info.UseAttachment() ? m_pEdict : NULL), weapon_info.GetAttachment() );
		
		m_pBeam.Update();
	}
	
	//Update dot data
	if(lasersight.dot_active)
	{
		m_pDot.SetColor		(weapon_info.GetColor()		);
		m_pDot.SetWidth		(weapon_info.GetWidth()		);
		m_pDot.SetPosition	(tr.vecEndPos			);
				
		m_pDot.Update();
	}

	//Got handled properly
	SetMetaResult(MRES_HANDLED);
		
	return;
}


void CPlayer::Command(const char *pcmd, const char *saycmd)
{	
	if (stricmp(pcmd,"say")!=0 && stricmp(pcmd,"say_team")!=0) 
	{
		SetMetaResult(MRES_IGNORED);
		return;
	}
	
	bool hidebeams = ( strnicmp(saycmd,"hidebeam",8) == 0 );
	bool showbeams = ( strnicmp(saycmd,"showbeam",8) == 0 );

	if(hidebeams == showbeams) //Think about it ;)
	{
		SetMetaResult(MRES_HANDLED);
		return;
	}
	
	//Alloc some buffer space
	char* OutPut = (char*)malloc(256);
	
	//Get right output format
	snprintf(OutPut,256,"[%s] %s\n",VLOGTAG,CVAR_GET_STRING(hidebeams?"lasersight_hidingbeams":"lasersight_showingbeams"));
	
	//Send message to client
	SendTextMsg(HUD_PRINTTALK,OutPut);
	
	//Set show beam status
	m_showbeam=showbeams;
	
	//Free buffer
	free(OutPut);
	
	SetMetaResult(MRES_SUPERCEDE);
	return;
}


inline bool CPlayer::IsOtherPlayerEnemy(CPlayer* pOther)
{
	if(!MOD_Handler) return false;
	
	if(MOD_Handler->IsOnSameTeam(this,pOther))
		return false;
	else
		return true;
}


int CPlayer::AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, int hostflags, int player, unsigned char *pSet)
{
	bool IsDot=false;
	bool IsBeam=false;
	CPlayer *pOwner=NULL;
	
	//Set default return type
	SetMetaResult(MRES_IGNORED);
	
	//In GameDLL we trust! (Already stopped .. don't have to do that again)
	//or if ent is player ..
	//or if host is dead ..
	if(META_RESULT_ORIG_RET(int)==0 || player || !IsAlive())
	{
		return 0;
	}
	
	//If not our beam or dot ..return
	if(!GetBeamDotOwner(ent,pOwner,IsBeam,IsDot))
	{
		return 0;
	}
	
	//Ent is our beam .. set result type to override
	SetMetaResult(MRES_OVERRIDE);
	state->eflags &= ~EFLAG_SLERP; //
	
	//Check Allow Set Off Stuff
	if(lasersight.allow_to_set_off && !m_showbeam) 
	{
		return 0;
	}
	
	if (pOwner!=this)
	{
		//Do we allow enemy to see?
		if(!lasersight.enemysees && IsAlive() && IsOtherPlayerEnemy(pOwner))
		{
			return 0; 
		}
		
		//If this isn't	player's own beam return
		return 1; 
	}
	
	//Now check if it's beam or dot	and check cvar values
	if((IsDot && lasersight.show_own_dot) || (IsBeam && lasersight.show_own_beam ))
	{
		if(lasersight.fix_own_beam_pos) //Calculate fixes dots and beams
		{
			//Make engine calculate	direction vectors
			UTIL_MakeVectors(m_pEdict->v.v_angle);
			
			TraceResult tr;
			
			//Start	of lasersight
			Vector vecOrigSrc = (m_pEdict->v.origin + m_pEdict->v.view_ofs);//GetGunPos
			
			//End of lasersight (vec_dest)
			Vector vecDest = vecOrigSrc + gpGlobals->v_forward * 8192;
			
			//Make engine trace line from start to end
			Unbreakable_Traceline( vecOrigSrc, vecDest, m_pEdict, &tr );
			
			memcpy( state->origin, (float*)tr.vecEndPos, 3 * sizeof( float ) );
			
			if(!IsDot)
			{
				Vector mins_maxs_tmp;
				
				//RELINK BEAM START
				//mins	
				mins_maxs_tmp.x = Q_min( tr.vecEndPos.x, ent->v.angles.x );
				mins_maxs_tmp.y = Q_min( tr.vecEndPos.y, ent->v.angles.y );
				mins_maxs_tmp.z = Q_min( tr.vecEndPos.z, ent->v.angles.z );
				mins_maxs_tmp = mins_maxs_tmp - tr.vecEndPos;
				memcpy( state->mins, (float*)mins_maxs_tmp, 3 * sizeof( float ) );
				//maxs
				mins_maxs_tmp.x = Q_max( tr.vecEndPos.x, ent->v.angles.x );
				mins_maxs_tmp.y = Q_max( tr.vecEndPos.y, ent->v.angles.y );
				mins_maxs_tmp.z = Q_max( tr.vecEndPos.z, ent->v.angles.z );
				mins_maxs_tmp = mins_maxs_tmp - tr.vecEndPos;
				memcpy( state->maxs, (float*)mins_maxs_tmp, 3 * sizeof( float ) );
			}
		}
		
		return 1; 
	}
	
	return 0; //This is player's beam .. don't send data.	
}


//Gets owner of beam/dot .. returns true if found
bool CPlayer::GetBeamDotOwner(const edict_t * ent,CPlayer * &pOwner,bool &IsBeam,bool &IsDot)
{
	IsDot=false;
	IsBeam=false;
	pOwner=NULL;
	
	for(int i=0;i<33;i++)
	{
		if(players[i].m_pBeam.GetEdict()==ent)
		{
			pOwner=&players[i];
			IsBeam=true;
			break;
		}
		else if(players[i].m_pDot.GetEdict()==ent)
		{
			pOwner=&players[i];
			IsDot=true;
			break;
		}
	}
	
	return (IsDot || IsBeam);
}


//Sends message to player
static int message_TextMsg = 0;
void CPlayer::SendTextMsg(int msg_dest, const char* msg_name)
{
	if (!m_connected) return;
	
	if (message_TextMsg == 0) message_TextMsg = GET_USER_MSG_ID(PLID,"TextMsg",NULL);
	
	if (message_TextMsg == 0 || m_pEdict->v.flags & FL_FAKECLIENT) return;
	
	MESSAGE_BEGIN( MSG_ONE, message_TextMsg, NULL, m_pEdict );
		WRITE_BYTE( msg_dest );
		WRITE_STRING( msg_name );
	MESSAGE_END();
}


//Very nice traceline function ;)
void Unbreakable_Traceline(const Vector &vecStart, const Vector &vecEnd, edict_t* pIgnore, TraceResult *ptr)
{
	Vector 		vecDir;
	bool 		first_loop;

	ptr->vecEndPos  		= vecStart; 
	first_loop      		= true;
	do {
		if (!first_loop)
		{ 		
			if (ptr->flFraction < 0.001f)
				ptr->vecEndPos = ptr->vecEndPos + vecDir * 4;
		}

		TRACE_LINE( ptr->vecEndPos, vecEnd, 0x101, pIgnore, ptr );

		if (ptr->flFraction>=1.0f) 
			break;

		if ((STRING(ptr->pHit->v.model))[0]!='*') 
			break;

		if (ptr->pHit->v.renderamt==0 && ptr->pHit->v.rendermode==kRenderTransTexture)
		{
			/* This is 100% invisible case */
		}
		else if(ptr->pHit->v.renderamt>250&&ptr->pHit->v.rendermode==kRenderTransTexture)
			break;
		else if (ptr->pHit->v.renderamt==0 || ptr->pHit->v.rendermode==kRenderNormal)
			break;

		pIgnore=ptr->pHit; 

		if (first_loop)
		{
			vecDir=(vecEnd-vecStart).Normalize();
			first_loop=false;
		}
	} while(true);	
}

