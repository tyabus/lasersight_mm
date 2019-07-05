/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#include "lasersight.h"
#include "libini.h" //v1.1.9-1
#include <stdio.h>


laserinfo_t WeaponLaserInfo[32];
static ini_fd_t	inidata;


void ShowWeaponData(void)
{
	LOG_CONSOLE(PLID, "Currently loaded weapon data:");
	LOG_CONSOLE(PLID, "%8s |" "%8s |" "%8s |" "%8s |" "%8s |" "%8s |" "%8s |" "%8s |" "%8s" "\n", 
		"id", "cR", "cG", "cB", "oR", "oU", "oF", "wi", "at");
	
	for(unsigned int i = 0; i < 32; i++) {
		if(!WeaponLaserInfo[i].IsUsed())
			continue;
		
		LOG_CONSOLE(PLID, "%8u  " "%8i  " "%8i  " "%8i  " "%8i  " "%8i  " "%8i  " "%8i  " "%8i" "\n", 
			i, 
			WeaponLaserInfo[i].GetColor()[0],
			WeaponLaserInfo[i].GetColor()[1],
			WeaponLaserInfo[i].GetColor()[2],
			WeaponLaserInfo[i].GetRightOffset(),
			WeaponLaserInfo[i].GetUpOffset(),
			WeaponLaserInfo[i].GetForwardOffset(),
			WeaponLaserInfo[i].GetWidth(),
			WeaponLaserInfo[i].GetAttachment()
		);
	}
	
	LOG_CONSOLE(PLID, "--EOL--");
}


void ReloadWeaponData(void)
{
	LOG_CONSOLE(PLID, "Reloading lasersight_mm.ini ...");
	LoadINIData();
}


static int INIReadInt(char *input, int default_value)
{
	double value = 0.0;
	
	// [2.10] changed from ini_readInt to ini_readDouble to read negative values correctly
	if (!ini_locateKey(inidata,(input)) && !ini_readDouble(inidata,&value) && (value)>0)
	{
		return (int)value;
	}
	
	return default_value;
}


void LoadINIData(void)
{
	char filename[1024];
	int i,value;

	//Clear table
	for(i=0;i<32;i++) WeaponLaserInfo[i].Reset();
	
	//Get filename
	STRNCPY(filename,lasersight.mod_dir,1024);
	STRNCAT(filename,
#ifdef __linux__
	"/addons/lasersight/" 
#else
	"\\addons\\lasersight\\"
#endif
	"lasersight_mm.ini",1024);

	ALERT(at_console,"[%s] Loading lasersight config from '%s' ... ", "Lasersight",filename);

	//Try load/parse ini to memory	
	inidata=ini_open(filename,"r",";");

	if (!inidata)
	{
		ALERT(at_console,"Failed!\n");
		
		//Get filename
		STRNCPY(filename,lasersight.mod_dir,1024);
		STRNCAT(filename,
#ifdef __linux__
		"/" 
#else
		"\\"
#endif
		"lasersight_mm.ini",1024);
		
		ALERT(at_console,"[%s] Loading lasersight config from '%s' ... ", "Lasersight",filename);
		
		//Try load/parse ini to memory	
		inidata=ini_open(filename,"r",";");

		if (!inidata)
		{	
			ALERT(at_console,"Failed!\n");
		
			return;
		}
	}
	ALERT(at_console,"Successed!\n");
	
	//Set [head]
	char head_info[256];
	if (
		!stricmp(lasersight.mod_dir, "valve")
			||
		!stricmp(lasersight.mod_dir, "ag")
			||
		!stricmp(lasersight.mod_dir, "oz")
			||
		!stricmp(lasersight.mod_dir, "phineas")
			||
		!stricmp(lasersight.mod_dir, "gearbox")
			||
		!stricmp(lasersight.mod_dir, "jumbot")
	   )
	{
		STRNCPY(head_info,"standard(valve,gearbox,etc)",256);
	}
	else
	{
		STRNCPY(head_info,lasersight.mod_dir,256);
	}
	
	//Find [head]
	if (!ini_locateHeading(inidata,head_info))
	{
		value=0;
		lasersight.AmmoDetectAsFlag  	= (!ini_locateKey(inidata,"AmmoIdIsFlag") && !ini_readBool(inidata,&value) && value!=0);
		lasersight.DefaultAttachment 	= INIReadInt("DefaultAttachment",	-1);
		lasersight.DefaultOffsetRight 	= INIReadInt("DefaultOffsetRight",	0);
		lasersight.DefaultOffsetUp 	= INIReadInt("DefaultOffsetUp",		0);
		lasersight.DefaultOffsetForward = INIReadInt("DefaultOffsetForward",	14);
		lasersight.DefaultFlipFlag 	= INIReadInt("DefaultFlipFlag",		0);
		lasersight.DefaultWidth		= INIReadInt("DefaultWidth",		0);
		
		//Get DefaultColor
		ini_listDelims(inidata,":,");
		if (!ini_locateKey(inidata,"DefaultColor"))
		{
			int color[3] = {0,0,0};
			for (int ii=0;ii<3;ii++)
			{
				if (!ini_listIndex(inidata,ii))
				{
					value=0;
					if (!ini_readInt(inidata,&value) && value>0) color[ii]=value;
				}
			}
			lasersight.DefaultColor[0]=color[0];
			lasersight.DefaultColor[1]=color[1];
			lasersight.DefaultColor[2]=color[2];
		}
		else
		{
			lasersight.DefaultColor[0]=lasersight.DefaultColor[1]=lasersight.DefaultColor[2]=0;
		}
		ini_listDelims(inidata,"");
	
		//Read data
		for (i=0;i<32;i++) 
		{
			char num[5];
			char offs_u[32];
			char offs_r[32];
			char rgb[32];
			char width[32];
			char flip[32];
			char forward[32];
			char attachment[32];
			
			laserinfo_t &weapon_info = WeaponLaserInfo[i];
			
			snprintf(num,5,"%d",i);
			
			STRNCPY(offs_u,num,32);
			STRNCAT(offs_u,"_offset_up",32);

			STRNCPY(offs_r,num,32);
			STRNCAT(offs_r,"_offset_right",32);
			
			STRNCPY(forward,num,32);
			STRNCAT(forward,"_offset_forward",32);
			
			STRNCPY(width,num,32);
			STRNCAT(width,"_width",32);
			
			STRNCPY(rgb,num,32);
			STRNCAT(rgb,"_rgb",32);
			
			STRNCPY(flip,num,32);
			STRNCAT(flip,"_flipflag",32);
			
			STRNCPY(attachment,num,32);
			STRNCAT(attachment,"_attachment",32);
			
			weapon_info.SetRight	(INIReadInt(offs_r, lasersight.DefaultOffsetRight));
			weapon_info.SetUp	(INIReadInt(offs_u, lasersight.DefaultOffsetUp));
			weapon_info.SetForward	(INIReadInt(forward,lasersight.DefaultOffsetForward));
			weapon_info.SetWidth	(INIReadInt(width,  lasersight.DefaultWidth));
			weapon_info.SetOffsetFlip(INIReadInt(flip,  lasersight.DefaultFlipFlag));
			weapon_info.SetAttachment(INIReadInt(attachment,lasersight.DefaultAttachment));
			
			//start - Get color
			ini_listDelims(inidata,":,");
			if (!ini_locateKey(inidata,rgb))
			{
				int color[3];
				memcpy(color,lasersight.DefaultColor,sizeof(color));
				for (int ii=0;ii<3;ii++)
				{
					if (!ini_listIndex(inidata,ii))
					{
						value=0;
						if (!ini_readInt(inidata,&value) && value>0) color[ii]=value;
					}
				}
				weapon_info.SetColor(color);
			}
			else
			{
				weapon_info.SetColor(lasersight.DefaultColor);
			}
			ini_listDelims(inidata,"");
			//end - Get color
			
			weapon_info.SetUsed(weapon_info.GetWidth()>0);
		}
	}
	else
	{
		lasersight.AmmoDetectAsFlag = false;
	}
	
	ini_close(inidata);
}

