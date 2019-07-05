/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef CMODHANDLERS_H
#define CMODHANDLERS_H


#define UNKNOWN_MOD 10101


class CBaseMODHandler
{
public:
	virtual bool IsOnSameTeam(CPlayer* P1,CPlayer* P2) = 0;
	virtual bool ShouldPlayerHaveBeam(CPlayer* pPlayer) = 0;
};


class CMODDefault : public CBaseMODHandler
{
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		return true;
	}
	
	bool ShouldPlayerHaveBeam(CPlayer* pPlayer) { return true; }
};


inline const char *GetModel(edict_t *pEntity) 
{
	if(pEntity)
		return( g_engfuncs.pfnInfoKeyValue( (*g_engfuncs.pfnGetInfoKeyBuffer)( pEntity ), "model" ) );
	return "";
}
inline const char *GetModel(CPlayer *pPlayer) 
{
	if(pPlayer && pPlayer->IsValid())
		return( GetModel(pPlayer->m_pEdict) );
	return "";
}


class CMOD_CounterStrike : public CBaseMODHandler
{
private:
	int GetTeamID(edict_t* pEntity)
	{
		const char *model_name = GetModel(pEntity);
		
		if (!model_name || !model_name[0] || !model_name[1])
		{
			return 0;
		}
		else if (model_name[0]=='g')
		{
			if(model_name[1]=='u')
			{
				return 2; //Red Team
			}
			else
			{
				return 1; //Blue Team
			}
		}
		else if (model_name[0]=='a' || model_name[0]=='t' || model_name[0]=='l')
		{
			return 2; //Red Team
		}
		else
		{
			return 1; //Blue Team
		}
	}
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		if (!P1 || !P1->IsValid() || !P2 || !P2->IsValid()) 
			return false;
		else
		{
			int team1 = GetTeamID(P1->m_pEdict);
			int team2 = GetTeamID(P2->m_pEdict);
			
			//No model.. spectator
			if (team1==0 || team2==0) 
				return true;
			else
				return (team1 == team2);
		}
	}

	bool ShouldPlayerHaveBeam(CPlayer* pPlayer) { return true; }
};


class CMOD_TFC : public CBaseMODHandler
{
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		if (!P1 || !P1->IsValid() || !P2 || !P2->IsValid()) 
			return false;
		else
			return (
					P1->m_pEdict->v.team==P2->m_pEdict->v.team 
						|| 
					(lasersight.team_allies[(P1->m_pEdict->v.team-1)] & (1<<(P2->m_pEdict->v.team-1)))
			);
	}

	bool ShouldPlayerHaveBeam(CPlayer* pPlayer) { return true; }
};


class CMOD_DOD : public CBaseMODHandler
{
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		if (!P1 || !P1->IsValid() || !P2 || !P2->IsValid()) 
			return false;
		else
			return ( P1->m_pEdict->v.team == P2->m_pEdict->v.team );
	}
	
	bool ShouldPlayerHaveBeam(CPlayer* pPlayer) { return true; }
};


class CMOD_NS : public CBaseMODHandler
{
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		if (!P1 || !P1->IsValid() || !P2 || !P2->IsValid()) 
			return false;
		else
			return ( P1->m_pEdict->v.team == P2->m_pEdict->v.team );
	}
	
	bool ShouldPlayerHaveBeam(CPlayer* pPlayer)
	{
		if(!pPlayer || !pPlayer->IsValid()) 
		{
			return false;
		}
		
		//Check if player is in the ReadyRoom
		if(pPlayer->m_pEdict->v.team == 0)
		{
			return false;
		}
		
		//Check if player is Commander
		const char* model = GetModel(pPlayer);
		if(!stricmp(model,"commander"))
		{
			return false;
		}
		
		return true;
	}
};


class CMOD_OP4CTF : public CBaseMODHandler
{
private:
	int GetTeamID(edict_t* pEntity)
	{
		const char *model_name = GetModel(pEntity);
		
		if (!model_name || !model_name[0])
			return 0;
		else if (model_name[0]=='c' || model_name[0]=='o')
   			return 1; //"Yellow"-team
   		else
   			return 2; //"Green"-team
	}
public:
	bool IsOnSameTeam(CPlayer* P1,CPlayer* P2)
	{
		if (!P1 || !P1->IsValid() || !P2 || !P2->IsValid()) 
			return false;
		else
		{
			int team1 = GetTeamID(P1->m_pEdict);
			int team2 = GetTeamID(P2->m_pEdict);
			
			//No model.. spectator
			if (team1==0 || team2==0) 
				return true;
			else
				return (team1 == team2);
		}
	}
	
	bool ShouldPlayerHaveBeam(CPlayer* pPlayer) { return true; }
};


#endif /*CMODHANDLERS_H*/
