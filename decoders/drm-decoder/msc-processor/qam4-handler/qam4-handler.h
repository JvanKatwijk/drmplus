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
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__QAM4_HANDLER__
#define	__QAM4_HANDLER__

#include	<vector>
#include	"basics.h"
#include	"qam4-metrics.h"
#include	"mapper.h"
#include	"puncture-tables.h"
#include	"deconvolver.h"

class	viterbi_drm;
class	prbs;

class qam4_handler: public deconvolver {
public:
		qam4_handler	(drmParameters *, int, int, int);
		~qam4_handler	();
	void	process		(theSignal *, uint8_t *);
private:
	qam4_metrics	myDecoder;
	punctureTables	myTables;
	drmParameters	*params;
	int		muxLength;
	int		muxCounter;
	int		lengthA;
	int		lengthB;
	int		Rx_high;
	int		Ry_high;
	int		Rx_low;
	int		Ry_low;
	int		stream;
	void		map_protLevel	(int, int *, int*);
	int16_t		getRYlcm_4	(int16_t);
	int		N1, N2;
	uint8_t		*puncture_high;
	uint8_t		*puncture_low;
	uint8_t		*residuTable;
	int		nrBits_high;
	int		nrBits_low;
	Mapper		*Y21Mapper_high;
	Mapper		*Y21Mapper_low;
	viterbi_drm	*deconvolver;
	prbs		*thePRBS;
	uint8_t		*firstBuffer;
};
#endif

