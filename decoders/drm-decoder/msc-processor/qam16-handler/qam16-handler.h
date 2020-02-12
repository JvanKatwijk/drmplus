#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ receiver
 *
 *    DRM+ receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef	__QAM16_HANDLER__
#define	__QAM16_HANDLER__

#include	"basics.h"
#include	"msc-handler.h"
#include	"qam16-metrics.h"

class	frameProcessor;
class	Mapper;
class	MSC_streamer;
class	prbs;

class qam16_handler: public mscHandler {
public:
		qam16_handler	(drmParameters *, frameProcessor *, int, int);
		~qam16_handler	();
	void	process		(theSignal *, bool);
private:
	drmParameters	*params;
	frameProcessor	*the_postProcessor;
	prbs		*thePRBS;
	int		muxLength;
	int		streamIndex;
	qam16_metrics   myDecoder;
	int16_t         lengthA;
	int16_t         lengthB;
	uint8_t         *out;
	MSC_streamer    *stream_0;
	MSC_streamer    *stream_1;
	Mapper          *Y13mapper_high;
	Mapper          *Y21mapper_high;
	Mapper          *Y13mapper_low;
	Mapper          *Y21mapper_low;
	int16_t         N1, N2;
	uint8_t		*firstBuffer;
};
#endif

