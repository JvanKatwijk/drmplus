#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of drm+-receiver
 *
 *    drm+ receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    drm+ receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with drm+ receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__SDC_HANDLER__
#define	__SDC_HANDLER__

#include	"basics.h"
#include	"viterbi-drm.h"
#include	"puncture-tables.h"

class	sdcHandler {
public:
	sdcHandler	(drmParameters *, uint8_t, uint8_t, int);
	~sdcHandler	();
void	process		(metrics *, uint8_t *);
int	get_sdcBits	();
private:
	punctureTables	pt;
	drmParameters	*params;
	int		Rx, Ry;
	int		nr_sdcSamples;
	int		nr_sdcBits;
	int		deconvolveLength;
	viterbi_drm	deconvolver;
	uint8_t		*residuTable;
	uint8_t		*punctureTable;
	int		punctureSize;
};

#endif


