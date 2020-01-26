#
/*
 *    Copyright (C) 2020
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
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"sincos.h"
//
//	As it turns out, when using DAB sticks, this simple function is the
//	real CPU burner, with a usage of up to 22 %

	SinCos::SinCos (std::complex<float> *Table, int32_t Rate) {
           this	-> Table	= Table;
	   this	-> localTable	= false;
	   this	-> Rate		= Rate;
	   this	->	C	= Rate / (2 * M_PI);
}

	SinCos::SinCos (int32_t Rate) {
int32_t	i;
	   this	-> Rate		= Rate;
	   this	-> localTable	= true;
	   this	-> Table	= new std::complex<float> [Rate];
	   for (i = 0; i < Rate; i ++) 
	      Table [i] = std::complex<float> (cos (2 * M_PI * i / Rate),
	                                       sin (2 * M_PI * i / Rate));
	   this	->	C	= Rate / (2 * M_PI);
}

	SinCos::~SinCos (void) {
	   if (localTable)
	      delete [] Table;
}
//	Heavy code: executed millions of times
//	we get all kinds of very strange values here, so
//	testing over the whole domain is needed
int32_t	SinCos::fromPhasetoIndex (float Phase) {	
	if (Phase >= 0)
	   return (int32_t (Phase * C)) % Rate;
	else
	   return Rate - (int32_t (Phase * C)) % Rate;
	if (0 <= Phase && Phase < 2 * M_PI)
	   return Phase / (2 * M_PI) * Rate;

	if (Phase >= 2 * M_PI)
//	   return fmod (Phase, 2 * M_PI) / (2 * M_PI) * Rate;
	   return (int32_t (Phase / (2 * M_PI) * Rate)) % Rate;

	if (Phase >= -2 * M_PI) {
	   Phase = -Phase;
	   return Rate - Phase / (2 * M_PI) * Rate;
	}

	Phase = -Phase;
	return Rate - fmod (Phase, 2 * M_PI) / (2 * M_PI) * Rate;
}

float	SinCos::getSin (float Phase) {
	if (Phase < 0)
	   return -getSin (- Phase);
	return imag (Table [fromPhasetoIndex (Phase)]);
}

float	SinCos::getCos (float Phase) {
	if (Phase >= 0)
	   return real (Table [(int32_t (Phase * C)) % Rate]);
	else
	   return real (Table [Rate - (int32_t ( - Phase * C)) % Rate]);
	if (Phase < 0)
	   Phase = -Phase;
	return real (Table [fromPhasetoIndex (Phase)]);
}

std::complex<float>	SinCos::getComplex (float Phase) {
	if (Phase >= 0)
	   return Table [(int32_t (Phase * C)) % Rate];
	else
	   return Table [Rate - (int32_t ( - Phase * C)) % Rate];
	return Table [fromPhasetoIndex (Phase)];
}


