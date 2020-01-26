#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ decoder
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
#
#ifndef	__PUNCTURE_TABLES__
#define	__PUNCTURE_TABLES__

#include	<stdint.h>


class	punctureTables {
public:
		punctureTables	(void);
		~punctureTables	(void);

	uint8_t	*getPunctureTable	(uint8_t, uint8_t);
	uint8_t	*getResiduTable		(uint8_t, uint8_t, int16_t);
	int16_t	getResiduBits		(uint8_t, uint8_t, int16_t);
};


#endif

