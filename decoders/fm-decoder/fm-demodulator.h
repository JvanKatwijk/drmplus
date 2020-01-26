#
/*
 *    Copyright (C) 2014
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of DRM+ Decoder
 *
 *    DRM+ Decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ Decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__FM_DEMODULATOR__
#define	__FM_DEMODULATOR__

#include	"radio-constants.h"
#include	"sincos.h"
#include	"pllC.h"
#include	"Xtan2.h"

#define	PLL_PILOT_GAIN	3000

class	fm_Demodulator {
public:
	enum fm_demod {
	   FM1DECODER	= 0001,
	   FM2DECODER	= 0002,
	   FM3DECODER	= 0003,
	   FM4DECODER	= 0004,
	   FM5DECODER	= 0005,
	   FM6DECODER	= 0006
	};
private:
	int8_t		selectedDecoder;
	float		max_freq_deviation;
	int32_t		rateIn;
	float		fm_afc;
	float		fm_cvt;
	float		K_FM;
	pllC		*myfm_pll;
	SinCos		*mySinCos;
	int32_t		ArcsineSize;
	float		*Arcsine;
	compAtan	myAtan;
	float		Imin1;
	float		Qmin1;
	float		Imin2;
	float		Qmin2;
public:
		fm_Demodulator	(int32_t	Rate_in,
	                         SinCos		*mySinCos,
	                         float		K_FM);
		~fm_Demodulator	(void);
	void		setDecoder	(int8_t);
const	char		*nameofDecoder	(void);
	float		demodulate	(std::complex<float>);
	float		get_DcComponent	(void);
};
#endif

