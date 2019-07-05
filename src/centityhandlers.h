/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef CENTITYHANDLERS_H
#define CENTITYHANDLERS_H

//From effects.h .. probably .. cant remember :D
#define	SF_BEAM_TEMPORARY		0x8000
// Beam	types, encoded as a byte
enum 
{
	BEAM_POINTS = 0,
	BEAM_ENTPOINT,
	BEAM_ENTS,
	BEAM_HOSE,
};


class CBaseEffectEntityHandler
{
public:
	CBaseEffectEntityHandler() 
	{
		width = red = green = blue = brightness = 0;
		pEnt = NULL;
	};

	virtual void Create(void) = 0;
	virtual void Update(void) = 0;
	virtual void Remove(void) = 0;
	
	void SetColor(const int *rgb) 
	{
		red	= rgb[0];
		green	= rgb[1]; 
		blue	= rgb[2];
	};
	
	void SetBrightness(int b)
	{
		brightness = b;
	};
	
	void SetWidth(int w)
	{
		width	= w;
	};
	
	const edict_t * GetEdict()
	{
		return pEnt;
	}
	
	void SetEdictNull()
	{
		pEnt = NULL;
	}
	
protected:
	edict_t	*pEnt;
	int width, red, green, blue, brightness;
};

class CBeamHandler : public CBaseEffectEntityHandler
{
public:
	CBeamHandler()
	{
		startPos = endPos = Vector(0,0,0);
		pAttachment = NULL;
		iAttachment = -1;
		AttachmentIndex = 0;
	}

	void Create()
	{
		if(!FNullEnt(pEnt)) Remove();
		
		pEnt = CREATE_NAMED_ENTITY(MAKE_STRING("beam"));
		
		entvars_t *pev = &pEnt->v;
		
		pev->flags |= FL_CUSTOMENTITY;
		
		pev->animtime =	255;	//scroll rate
		pev->body = 0;		//noise
		
		pev->model = MAKE_STRING( lasersight.pSpriteName );
		pev->modelindex	= lasersight.m_spriteTexture;
		
		pev->skin = 0;
		pev->sequence =	0;
		pev->rendermode	= 0;
		
		SetBrightness(75);
	};
	
	void Update()
	{
		if (FNullEnt(pEnt)) Create();
	
		if(pAttachment && iAttachment>-1)
		{
			UpdateEntityAndPoint();
		}
		else
		{
			UpdateTwoPoints();
		}
	}
	
	void UpdateEntityAndPoint()
	{
		entvars_t *pev = &pEnt->v;

		pev->rendercolor.x = red;
		pev->rendercolor.y = green;
		pev->rendercolor.z = blue;
		
		pev->scale = width * lasersight.beamsize;	 //Width
	
		pev->renderamt = brightness;
		pev->rendermode = (BEAM_ENTPOINT&0x0F); //type

		//Set end point
		pev->origin = startPos;
		
		/*
		//Set start point
		pev->skin = (AttachmentIndex & 0x0FFF) | ((pev->skin&0xF000)<<12);
		pev->aiment = g_engfuncs.pfnPEntityOfEntIndex( AttachmentIndex );
		
		//Set end attachment to zero
		pev->sequence = (pev->sequence & 0x0FFF);
		
		//Set start attachment to iAttachment
		pev->skin = (pev->skin & 0x0FFF) | ((iAttachment&0xF)<<12);
		*/
		//Set start point
		pev->aiment = g_engfuncs.pfnPEntityOfEntIndex( AttachmentIndex );
		
		//Set end attachment to zero
		pev->sequence = (pev->sequence & 0x0FFF);
		
		//Set start attachment to iAttachment
		pev->skin = (AttachmentIndex & 0x0FFF) | ((iAttachment&0xF)<<12);
		
		{//RelinkBeam
			Vector _startPos = pev->origin;
			Vector _endPos = pAttachment->v.origin;

			pev->mins.x = Q_min( _startPos.x, _endPos.x );
			pev->mins.y = Q_min( _startPos.y, _endPos.y );
			pev->mins.z = Q_min( _startPos.z, _endPos.z );
			pev->maxs.x = Q_max( _startPos.x, _endPos.x );
			pev->maxs.y = Q_max( _startPos.y, _endPos.y );
			pev->maxs.z = Q_max( _startPos.z, _endPos.z );
			pev->mins = pev->mins - pev->origin;
			pev->maxs = pev->maxs - pev->origin;

			SET_SIZE( pEnt, pev->mins, pev->maxs );
			SET_ORIGIN(pEnt, pev->origin );
		}//RelinkBeam
	}
	
	void UpdateTwoPoints()
	{
		entvars_t *pev = &pEnt->v;

		pev->rendercolor.x = red;
		pev->rendercolor.y = green;
		pev->rendercolor.z = blue;

		pev->scale = width * lasersight.beamsize;	 //Width
	
		pev->renderamt = brightness;   
		pev->rendermode	= (BEAM_POINTS&0x0F); //BEAM_POINTS
	
		pev->aiment = 0;
	
		pev->origin = startPos;	       //Start point
		pev->angles = endPos;	       //End point
	
		pev->sequence =	(pev->sequence & 0x0FFF); //StartAt
		pev->skin = (pev->skin & 0x0FFF);	  //EndAt
	
		//RELINK BEAM START	
		pev->mins.x = Q_min( startPos.x, endPos.x	);
		pev->mins.y = Q_min( startPos.y, endPos.y	);
		pev->mins.z = Q_min( startPos.z, endPos.z	);
		pev->maxs.x = Q_max( startPos.x, endPos.x	);
		pev->maxs.y = Q_max( startPos.y, endPos.y	);
		pev->maxs.z = Q_max( startPos.z, endPos.z	);
		pev->mins = pev->mins -	pev->origin;
		pev->maxs = pev->maxs -	pev->origin;
	
		SET_SIZE(pEnt, pev->mins, pev->maxs);
		SET_ORIGIN(pEnt, pev->origin );
		//RELINK BEAM END

		pev->spawnflags	|= SF_BEAM_TEMPORARY; //// Flag	these to be destroyed on save/restore or level transition
	};
	
	void Remove()
	{
		if(FNullEnt(pEnt)) 
		{
			pEnt= NULL;
			
			return;
		}
		
		entvars_t *pev = &pEnt->v;
		
		pev->flags |= FL_KILLME;
		pev->targetname	= 0;
		
		pEnt=NULL;
	};
	
	void SetPosition(const Vector &start, const Vector &end) 
	{
		startPos= start; 
		endPos	= end;
	};
	
	void SetAttachment(const edict_t* pent, const int attachment)
	{
		pAttachment = (edict_t*)pent;
		iAttachment = attachment;
		if(pAttachment)
			AttachmentIndex = ENTINDEX(pAttachment);
		else
			AttachmentIndex = 0;
	};
private:
	Vector startPos,endPos;
	edict_t* pAttachment;
	int iAttachment;
	int AttachmentIndex;
};

class CDotHandler : public CBaseEffectEntityHandler
{
public:
	CDotHandler()
	{
		startPos = Vector(0,0,0);
	}

	void Create()
	{
		if(!FNullEnt(pEnt)) Remove();
		
		pEnt = CREATE_NAMED_ENTITY(MAKE_STRING("env_sprite"));
		
		entvars_t *pev = &pEnt->v;

		pev->movetype =	MOVETYPE_NONE;
		pev->solid = SOLID_NOT;

		pev->rendermode	= kRenderGlow;
		pev->renderfx =	kRenderFxNoDissipation;
		pev->renderamt = 255;

		SET_MODEL(pEnt, lasersight.pDotSpriteName);
		
		SetBrightness(75);
	};
	
	void Update()
	{
		if (FNullEnt(pEnt)) Create();

		entvars_t *pev = &pEnt->v;

		pev->rendercolor.x = red;
		pev->rendercolor.y = green;
		pev->rendercolor.z = blue;

		pev->renderamt = (int)(125.0f + ((float)brightness * 130.0f)/255.0f);
		
		pev->scale = (width/10.0f) * lasersight.dotsize;

		pev->origin = startPos;
		
		pev->spawnflags	|= SF_BEAM_TEMPORARY;

		SET_ORIGIN(pEnt, pev->origin);
	};
	
	void Remove()
	{
		if(FNullEnt(pEnt)) 
		{
			pEnt	= NULL;
			
			return;
		}
		
		entvars_t *pev = &pEnt->v;
		
		pev->flags |= FL_KILLME;
		pev->targetname	= 0;
		
		pEnt=NULL;
	};
	
	void SetPosition(const Vector &start) 
	{
		startPos= start; 
	};
private:
	Vector startPos;
};

#endif /*CENTITYHANDLERS*/

