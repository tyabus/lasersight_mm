/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef CPLAYER_H
#define CPLAYER_H

class CPlayer
{
public:
	bool		m_connected;	//Client connected to this slot
	
	edict_t	*	m_pEdict;
	int		m_index;
	
	CBeamHandler	m_pBeam;
	CDotHandler	m_pDot;
	bool		m_showbeam;
	
	int		m_current_weapon_id;
	
	//Client connected/disconnected
	void PutInServer(edict_t *pThis);
	void Disconnect();
	
	//Player think
	void PostThink();
	
	//Client command
	void Command(const char *pcmd, const char *saycmd);
	
	//For blocking/changing stuff for one client only
	int AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, int hostflags, int player, unsigned char *pSet);
	
	bool IsValid()
	{
		return(m_connected && !FNullEnt(m_pEdict));
	}
	
	bool IsAlive()
	{
		//Not connected -- Not Alive
		if(!m_connected || FNullEnt(m_pEdict)) return false;
		
		return ((m_pEdict->v.deadflag == DEAD_NO) && (m_pEdict->v.health > 0) && !(m_pEdict->v.flags & FL_NOTARGET));
	};
	
	//Set return type
	void SetMetaResult(META_RES meta_resu)
	{
		m_message_mres=meta_resu;
	}

	//Get default return type
	META_RES GetMetaResult(void)
	{
		return m_message_mres;
	}
	
	//Gets owner of beam/dot .. returns true if found
	static bool GetBeamDotOwner(const edict_t * ent,CPlayer * &pOwner,bool &IsBeam,bool &IsDot);
	
	//Gets index to players array
	static CPlayer *GetPlayerFromArray(int index);
	static CPlayer *GetPlayerFromArray(edict_t *pEnt)
	{
		return GetPlayerFromArray(FNullEnt(pEnt)?0:ENTINDEX(pEnt));
	}
	
	//Sends message to player
	void SendTextMsg(int msg_dest, const char* msg_name);
	
	//Remove beam
	void RemoveLasersightEntities(void)
	{
		m_pBeam.Remove();
		m_pDot.Remove();
	}

private:
	META_RES m_message_mres;
	
	bool IsOtherPlayerEnemy(CPlayer* pOther);
};


extern CPlayer players[33];


//Gets pointer to players array with index
inline CPlayer * CPlayer::GetPlayerFromArray(int index)
{
	if(index<1 || index>32) return &players[0];
	return &players[index];
}


#endif /*CPLAYER_H*/
