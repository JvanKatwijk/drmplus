#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the DRM+ decoder
 *    DRM+ decoder is free software; you can redistribute it and/or modify
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
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__REFERENCE_FRAME__
#define	__REFERENCE_FRAME__

#include	"radio-constants.h"
#include	"basics.h"

bool			isTimeCell	(int16_t);
std::complex<float>	getTimeRef	(int16_t);
bool			isGainCell	(int16_t, int16_t);
bool			isBoostCell	(int16_t, int16_t);
std::complex<float>	getGainValue	(int16_t, int16_t);
std::complex<float>	getGainRef	(int16_t, int16_t);
bool			isAfsCell	(int16_t, int16_t);
bool			isFACcell	(int16_t, int16_t);
void			getFACcell	(int16_t, int16_t *, int16_t *);
int			init_sdcTable	();
bool			isSDCcell	(int16_t, int16_t);
void			getSDCcell	(int16_t, int16_t *, int16_t *);

extern	bool	frame_1	[];
extern	bool	frame_23 [];
extern	bool	frame_4 [];
#endif


