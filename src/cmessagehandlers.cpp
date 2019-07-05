/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */


#include "lasersight.h"


bool CGetCurrentWeaponID::HandleMessageBegin() 
{
	if (lasersight.debug>1)
		ALERT(at_console,"[[CurWeapon]] [player-id:%d]\n",pPlayer->m_index);
	
	//Init
	iState = 0;
	iId = 0;
	
	//Player has to be valid
	return pPlayer->m_connected; 
}


bool CGetCurrentWeaponID::HandleMessageWrite() 
{ 
	switch (message_counter) 
	{
	case 0:
		//If state is 1 player has selected new weapon
		//If state is 0 player has picked new weapon up but not selected it
		iState = curvalue_int;
		
		//If state is 1 continue
		return (iState==1);
	case 1:
		// DMC fix
		if (lasersight.AmmoDetectAsFlag)
		{
			// DMC weapon ids are flag style .. convert theim to normal ones
			int flagId = curvalue_int;
				
			iId=0;
				
			while(flagId>1)
			{
				flagId/=2;
				iId++;
			}
		}
		else 
		{
			iId = curvalue_int;
		}

		if (iId <= 31 && iId >= 0)
		{
			pPlayer->m_current_weapon_id = iId;
		}
			
		if (lasersight.debug>0) 
			ALERT(at_console,"  <[Current weapon id(playerid:%d)] Id:%d>\n",pPlayer->m_index,pPlayer->m_current_weapon_id);
		
		//returning false removes handler
		return false;
	default:
		//returning false removes handler
		return false;
	}
}

