#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ Decoder
 *
 *    DRM+ Decoder is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _SINCOS_H
#define _SINCOS_H

#include	"radio-constants.h"

class	SinCos {
public:
	        	SinCos		(std::complex<float> *, int32_t);
			SinCos		(int32_t);
			~SinCos		(void);
	float		getSin		(float);
	float		getCos		(float);
	std::complex<float>	getComplex	(float);
	
private:
	std::complex<float>	*Table;
	int32_t		Rate;
	bool		localTable;
	double		C;
	int32_t		fromPhasetoIndex	(float);
};

#endif

