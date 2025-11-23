//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.
//!
//! @file   PIHSVFilter.h
//! @brief  HSVフィルタ
//!
#pragma once

typedef	unsigned char	BYTE;
typedef	unsigned long	UINT32;
typedef	long			INT32;

//!
//! @brief  HSVフィルタ
//!
class	PIHSVFilter
{
	//	ブレンド
	static	int Blend8( const int dst, const int src, const int mask )
	{
		return ((dst-src)*mask/255)+src;
	}
public:
	//!	@param		const INT32	nHFilter		--- [in]	 0,(360*65536-1)
	//!	@param		const INT32	nSFilter		--- [in]	 -65536,65536
	//!	@param		const INT32	nVFilter		--- [in]	 -65536,65536
	inline	static	void	SetHSV8( BYTE& rR, BYTE& rG, BYTE& rB, const INT32 cnHFilter, const INT32 cnSFilter, const INT32 cnVFilter )
	{
		UINT32	uH,uS,uV;
		RGBtoHSV(uH,uS,uV,rR,rG,rB);

		HSVFilter(uH,uS,uV,cnHFilter,cnSFilter,cnVFilter);
		
		UINT32	uR,uG,uB;
		HSVtoRGB(uR,uG,uB,uH,uS,uV);

		rR	= (BYTE)uR;
		rG	= (BYTE)uG;
		rB	= (BYTE)uB;
	}
	inline	static	void	SetHSV8Mask( BYTE& rR, BYTE& rG, BYTE& rB,const BYTE mask,  const INT32 cnHFilter, const INT32 cnSFilter, const INT32 cnVFilter )
	{
		UINT32	uH,uS,uV;
		RGBtoHSV(uH,uS,uV,rR,rG,rB);

		HSVFilter(uH,uS,uV,cnHFilter,cnSFilter,cnVFilter);
		
		UINT32	uR,uG,uB;
		HSVtoRGB(uR,uG,uB,uH,uS,uV);

		rR	= (BYTE)Blend8(uR,rR,mask);
		rG	= (BYTE)Blend8(uG,rG,mask);
		rB	= (BYTE)Blend8(uB,rB,mask);
	}

private:
	inline	static	UINT32	GetP1( const UINT32 cuV, const UINT32 cuS, const UINT32 cuF )
	{
		return (cuV*(65536-cuS))>>16;
	}

	inline	static	UINT32	GetP2( const UINT32 cuV, const UINT32 cuS, const UINT32 cuF )
	{
		return (cuV*(65536-((cuS*cuF)>>15)))>>16;
	}

	inline	static	UINT32	GetP3( const UINT32 cuV, const UINT32 cuS, const UINT32 cuF )
	{
		return (cuV*(65536-((cuS*(32768-cuF))>>15)))>>16;
	}

	inline	static	INT32	GetH( const UINT32 cuColor1, const UINT32 cuColor2, const UINT32 cuD )
	{
		if( cuColor1 >= cuColor2 )
		{
			return ((cuColor1-cuColor2)<<15)/cuD;
		}
		else
		{
			return -(INT32)((((cuColor2-cuColor1)<<15)/cuD));
		}
	}

	//	h(0〜6*32768)
	//	s(0〜65536)
	//	v(0〜255)	RGBが16bitの場合はv(0〜65535)
	inline	static	void	RGBtoHSV( UINT32& ruH, UINT32& ruS, UINT32& ruV, const UINT32 cuR, const UINT32 cuG, const UINT32 cuB )
	{
		UINT32	uMax,uMin;
		if( cuR >= cuG )
		{
			if( cuR >= cuB )
			{
				uMax	= cuR;
				if( cuG >= cuB ){ uMin=cuB; }else{ uMin=cuG; }
			}
			else
			{
				uMax	= cuB;
				uMin	= cuG;
			}
		}
		else if( cuG >= cuB )
		{
			uMax	= cuG;
			if( cuR >= cuB ){ uMin=cuB; }else{ uMin=cuR; }
		}
		else
		{
			uMax	= cuB;
			uMin	= cuR;
		}

		UINT32	uD	= uMax-uMin;
		UINT32	uS;
		if( uMax == 0 )
		{
			uS	= 0;
		}
		else
		{
			uS	= (uD<<16)/uMax;
		}

		INT32	nH;
		if( uS == 0 )
		{
			nH	= 0;
		}
		else
		{
			if( uMax == cuR )
			{
				nH	= GetH(cuG,cuB,uD);
				if( nH < 0 ){ nH+=6*32768; }
			}
			else if( uMax == cuG )
			{
				nH	= GetH(cuB,cuR,uD);
				nH	+= 2*32768;
			}
			else
			{
				nH	= GetH(cuR,cuG,uD);
				nH	+= 4*32768;
			}
		}
		ruH	= nH;
		ruS	= uS;
		ruV	= uMax;
	}

	//	h(0〜6*32768)
	//	s(0〜65536)
	//	v(0〜255)	RGBが16bitの場合はv(0〜65535)
	inline	static	void	HSVtoRGB( UINT32& ruR, UINT32& ruG, UINT32& ruB, const UINT32 cuH, const UINT32 cuS, const UINT32 cuV )
	{
		const UINT32	cuF	= (cuH&32767);
		switch( (cuH>>15) )
		{
			case 0:
				ruR	= cuV;
				ruG	= GetP3(cuV,cuS,cuF);
				ruB	= GetP1(cuV,cuS,cuF);
				break;

			case 1:
				ruR	= GetP2(cuV,cuS,cuF);
				ruG	= cuV;
				ruB	= GetP1(cuV,cuS,cuF);
				break;

			case 2:
				ruR	= GetP1(cuV,cuS,cuF);
				ruG	= cuV;
				ruB	= GetP3(cuV,cuS,cuF);
				break;

			case 3:
				ruR	= GetP1(cuV,cuS,cuF);
				ruG	= GetP2(cuV,cuS,cuF);
				ruB	= cuV;
				break;

			case 4:
				ruR	= GetP3(cuV,cuS,cuF);
				ruG	= GetP1(cuV,cuS,cuF);
				ruB	= cuV;
				break;

			case 5:
				ruR	= cuV;
				ruG	= GetP1(cuV,cuS,cuF);
				ruB	= GetP2(cuV,cuS,cuF);
				break;
		}
	}

	//!------------------------------------------------------------------------------
	//!
	//!	@brief		HSVFilter
	//!
	//!	@brief		HSVフィルタ
	//!
	//!	@param		INT32&		rnH				--- [in/out] 0,(360*65536-1)
	//!	@param		INT32&		rnS				--- [in/out] 0,65535
	//!	@param		INT32&		rnV				--- [in/out] 0,65535
	//!	@param		const INT32	nHFilter		--- [in]	 0,(360*65536-1)
	//!	@param		const INT32	nSFilter		--- [in]	 -65536,65536
	//!	@param		const INT32	nVFilter		--- [in]	 -65536,65536
	//!
	//!	@retval		なし
	//!
	//!	@note		なし
	//!
	//!------------------------------------------------------------------------------
	inline	static	void HSVFilter( UINT32& ruH, UINT32& ruS, UINT32& ruV, const INT32 cnHFilter, const INT32 cnSFilter, const INT32 cnVFilter )
	{
		static	const INT32	cnHSVHMax		= 6*32768;
		static	const INT32	cnHSVSMax		= 65536;
		static	const INT32	cnHSVVMax		= 255;
		
		INT32	nH	= ruH;
		INT32	nS	= ruS;
		INT32	nV	= ruV;

		if( cnHFilter != 0 )
		{
			nH	+= cnHFilter;
			if( nH >= cnHSVHMax ){ nH-=cnHSVHMax; }
		}

		if( cnVFilter != 0 )
		{
			if( cnVFilter > 0 )
			{
				nV	+= (INT32)((((UINT32)(cnHSVVMax-nV))*((UINT32)cnVFilter))>>15);
				if( nV > cnHSVVMax ){ nV=cnHSVVMax; }

				nS	-= (INT32)((((UINT32)nS)*((UINT32)cnVFilter))>>15);
				if( nS < 0 ){ nS=0; }
			}
			else
			{
				nV	-= (INT32)((((UINT32)nV)*((UINT32)(0-cnVFilter)))>>15);
				if( nV < 0 ){ nV=0; }
			}
		}

		if( cnSFilter != 0 && nS > 0 && nV > 0 )
		{
			if( cnSFilter > 0 )
			{
				INT32	nSaturation	= (((UINT32)(cnHSVSMax - nS)) * ((UINT32)cnSFilter)) >> 15;
				INT32	nValue		= (((UINT32)nV) * ((UINT32)nSaturation)) >> 16;
				INT32	nV2			= nV;

				nV	+= nValue;
				if( nV > cnHSVVMax ) 
				{
					nV	=  cnHSVVMax;
					nS	+= ((UINT32)nSaturation)*((UINT32)(nV-nV2))/((UINT32)nValue);
				}
				else
				{
					nS += nSaturation;
				}
				if( nS > cnHSVSMax ){ nS = cnHSVSMax; }
			}
			else
			{
				INT32	nS2	= nS;
				nS -= (INT32)((((UINT32)nS)*((UINT32)(0-cnSFilter)))>>15);
				if( nS < 0 ){ nS=0; }

				nV -= (INT32)((((UINT32)(nS2-nS))*((UINT32)nV))>>17);
				if( nV < 0 ){ nV=0; }
			}
		}

		ruH	= nH;
		ruS	= nS;
		ruV	= nV;
	}

};

