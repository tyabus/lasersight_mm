/*
 * Copyright © 2001-2002 Jussi Kivilinna <hullu@unitedadmins.com>
 *
 *    This file is part of Lasersight Metamod Plugin.
 *
 *    I think this file should be is GPL because Metamod is GPL .. but I allow you to
 *    use any part that isn't from Metamod or Valve's HLSDK for whatever you want :)
 */
 
#ifndef Claserinfo_H
#define Claserinfo_H


//weapon lasersight info (colors/offsets/etc)
class laserinfo_t
{
private:
	bool iUse;		//Are we going to create beam for this one.
	int iRGB[3];		//Beam color
	int iOffset_Right;	//Lasersight start place offset (left-right axis)
	int iOffset_Up;		//          - | | -            (down-up axis)
	int iOffset_Forward;	//
	int iWidth;		//
	int iAttachment;	//	
public:
	void Reset()
	{
		iUse		= false;
		iRGB[0] =
			iRGB[1] =
			iRGB[2]	= 0;
		iOffset_Right	= 0;
		iOffset_Up	= 0;
		iOffset_Forward = 14;//default value
		iWidth		= 0;
		iAttachment	= -1;//don't use by default
	};

	laserinfo_t() 
	{
		Reset();
	}
	
	const bool &IsUsed() 
	{ 
		return iUse; 
	};
	
	const int* GetColor() 
	{ 
		return &iRGB[0]; 
	};
	
	const int &GetWidth() 
	{ 
		return iWidth; 
	};
	
	const int &GetUpOffset()
	{ 
		return iOffset_Up; 
	};

	const int &GetRightOffset() 
	{ 
		return iOffset_Right; 
	};
	
	const int &GetForwardOffset() 
	{ 
		return iOffset_Forward; 
	};
	
	const int &GetAttachment()
	{
		return iAttachment;
	}
	
	bool UseAttachment()
	{
		return (iAttachment>-1);
	}
	
	void SetColor(const int &r,const int &g,const int &b)
	{
		iRGB[0]=r; 
		iRGB[1]=g; 
		iRGB[2]=b; 
	};
	
	void SetColor(const int* rgb)
	{
		SetColor(rgb[0],rgb[1],rgb[2]);
	};
	
	void SetUsed(const bool &use) 
	{ 
		iUse = use; 
	};
	
	void SetOffsets(const int &right,const int &up) 
	{ 
		iOffset_Right = right;
		iOffset_Up = up;
	};
	
	void SetRight(const int &right) 
	{ 
		iOffset_Right = right;
	};
	
	void SetUp(const int &up) 
	{ 
		iOffset_Up = up;
	};
	
	void SetForward(const int &forward)
	{
		iOffset_Forward = forward;
	};
	
	void SetOffsetFlip(const int &fl)
	{
		if (fl==1 || fl==3)
		{
			iOffset_Up = -iOffset_Up;
		}
		if (fl==2 || fl==3)
		{
			iOffset_Right = -iOffset_Right;
		}
	}
	
	void SetWidth(const int &Width) 
	{ 
		iWidth = Width; 
	};
	
	void SetAttachment(const int &at)
	{
		iAttachment = at;
	}
};

extern laserinfo_t WeaponLaserInfo[32];


#endif /*Claserinfo_H*/

