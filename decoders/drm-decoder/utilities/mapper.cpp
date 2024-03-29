#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ decoder
 *
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
 *    along with DREM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#include	"mapper.h"
#include	<math.h>
#include	<stdio.h>
//
//	Mapper for the FAC and SDC bits, according to the standard
//	
static	inline
int	upLogger (int x) {
uint32_t	help	= 01;
	while (help < x) {
	   help <<= 1;
	}
	return help;
}

	Mapper::Mapper (int size, int t0) {
int	i;
int s, q;
	if (size < 0)
	   fprintf (stderr, "wat maak je me nou met %d\n", size);
	mapperTable	= new int [size + 10];
	if (t0 == 0) {
	   for (i = 0; i < size; i ++)
	      mapperTable [i] = i;
	   return;
	}
//	the standard prescribes the rounding to infinity
//	for FAC, size = 130, i.e. log2 (size) > log2 (128)
//	s		= 2 ^ (log2 (size));	//  size app 130, i.e. 256
	s		= upLogger (size);
	q		= s / 4 - 1;

	mapperTable [0] = 0;
	for (i = 1; i < size; i ++) {
	   mapperTable [i] = ((t0 * mapperTable [i - 1]) + q) % s;
	   while (mapperTable [i] >= size) 
	      mapperTable [i] = (t0 * mapperTable [i] + q) % s;
	}
}

	Mapper::~Mapper (void) {
	delete[] mapperTable;
}


int	Mapper::mapIn (int  a) {
	return mapperTable [a];
}

