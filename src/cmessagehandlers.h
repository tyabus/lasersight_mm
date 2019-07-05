/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef CMESSAGEHANDLERS_H
#define CMESSAGEHANDLERS_H


//This is the great base message handler class!
//See CGetCurrentWeaponID class too.
class CBaseMessageHandler
{
protected:
	//MessageBegin
	int message_dest;
	int message_type;
	const float *message_pOrigin;
	edict_t *message_pEdict;
	
	//extra
	int message_counter;
	CPlayer* pPlayer;
	
	//current values
	void *current_value;
	union {
		int curvalue_int;
		const char *curvalue_string;
		float curvalue_float;
		void *curvalue_pvoid;
	};

private:
	//Return type
	META_RES message_mres;
	
public:
	CBaseMessageHandler ()
	{
		message_dest = 0;
		message_type = 0;
		message_pOrigin = NULL;
		message_pEdict  = NULL;
		
		current_value	= NULL;
		curvalue_int	= 0;
		curvalue_float	= 0;	
		curvalue_string	= NULL;
		
		message_counter = 0;
	}
	
	bool MessageBegin(int msg_dest, int msg_type, const float *pOrigin, edict_t *pEdict)
	{
		//Set default return type
		SetMetaResult(MRES_HANDLED);
		
		//Init
		message_dest = msg_dest;
		message_type = msg_type;
		message_pOrigin = pOrigin;
		message_pEdict  = pEdict;
		
		message_counter = 0;
		
		pPlayer = CPlayer::GetPlayerFromArray(message_pEdict);
		
		//Call user defined code
		return HandleMessageBegin();
	}

	bool MessageWriteFloat(float value) 
	{
		//Set default return type
		SetMetaResult(MRES_HANDLED);
		
		//Set current value
		curvalue_float = value;
		current_value = (void *)&curvalue_float;
		
		//Call user defined code
		bool ret_val = HandleMessageWrite();
		
		message_counter++; 
		
		//clear current value
		current_value = NULL;
		curvalue_int = 0;
		
		return ret_val;
	}

	bool MessageWriteInt(int value) 
	{
		//Set default return type
		SetMetaResult(MRES_HANDLED);
		
		//Set current value
		curvalue_int = value;
		current_value = (void *)&curvalue_int;
		
		//Call user defined code
		bool ret_val = HandleMessageWrite();
		
		message_counter++; 
		
		//clear current value
		current_value = NULL;
		curvalue_int = 0;
		
		return ret_val;
	}
	
	bool MessageWriteString(const char* value) 
	{
		//Set default return type
		SetMetaResult(MRES_HANDLED);
		
		//Set current value
		curvalue_string = strdup(value);
		current_value = (void *)curvalue_string;
		
		//Call user defined code
		bool ret_val = HandleMessageWrite();
		
		message_counter++; 

		//free buffer
		free((void *)curvalue_string);
		
		//clear current value
		current_value = NULL;
		curvalue_int = 0;
		
		return ret_val;
	}

	void MessageEnd(void) 
	{ 
		//Set default return type
		SetMetaResult(MRES_HANDLED);
		
		//Call user defined code
		HandleMessageEnd(); 
		return;
	}
	
	//Set return type
	void SetMetaResult(META_RES meta_resu)
	{
		message_mres=meta_resu;
	}

	//Get default return type
	META_RES GetMetaResult(void)
	{
		return message_mres;
	}
	
protected:
	virtual bool HandleMessageBegin() = 0;
	virtual bool HandleMessageWrite() = 0;
	virtual void HandleMessageEnd() = 0;
};


//Handles CurWeapon msg to get new weapon id.
class CGetCurrentWeaponID : public CBaseMessageHandler
{
protected:
	bool HandleMessageBegin();
	bool HandleMessageWrite();
	void HandleMessageEnd() {};
	
private:
	int iState;
	int iId;
};


#endif /*CMESSAGEHANDLERS_H*/
