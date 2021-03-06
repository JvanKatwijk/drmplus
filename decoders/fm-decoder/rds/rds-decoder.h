#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *	This part of the jsdr is a mixture of code  based on code from
 *	various sources. Two in particular:
 *	
 *    FMSTACK Copyright (C) 2010 Michael Feilen
 * 
 *    Author(s)       : Michael Feilen (michael.feilen@tum.de)
 *    Initial release : 01.09.2009
 *    Last changed    : 09.03.2010
 *	
 *	cuteSDR (c) M Wheatly 2011
 *
 *    This file is part of the DRM+ Decoder
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

#ifndef	__RDS_DECODER__
#define	__RDS_DECODER__

#include	<QObject>
#include	"radio-constants.h"
#include	"rds-group.h"
#include	"rds-blocksynchronizer.h"
#include	"rds-groupdecoder.h"
#include	"fft.h"
#include	"iir-filters.h"
#include	"sincos.h"

class	fmDecoder;

class	rdsDecoder : public QObject {
Q_OBJECT
public:
		rdsDecoder (fmDecoder *, int32_t, SinCos *);
		~rdsDecoder (void);
	enum RdsMode {
	   NO_RDS	= 0,
	   RDS1		= 1,
	   RDS2		= 2
	};
	void	doDecode	(float	, float *, RdsMode);
	void	reset		(void);
private:
	void			processBit	(bool);
	void			doDecode1 (float, float *);
	void			doDecode2 (float, float *);
	int32_t			sampleRate;
	int32_t			numofFrames;
	SinCos			*mySinCos;
	fmDecoder		*myDecoder;
	RDSGroup		*my_rdsGroup;
	rdsBlockSynchronizer	*my_rdsBlockSync;
	rdsGroupDecoder		*my_rdsGroupDecoder;
	float			omegaRDS;
	int32_t			symbolCeiling;
	int32_t			symbolFloor;
	bool			prevBit;
	float			bitIntegrator;
	float			bitClkPhase;
	float			prev_clkState;
	bool			Resync;

	float			*rdsBuffer;
	float			*rdsKernel;
	int16_t			ip;
	int16_t			rdsfilterSize;
	float			Match		(float);
	BandPassIIR		*sharpFilter;
	float			rdsLastSyncSlope;
	float			rdsLastSync;
	float			rdsLastData;
	bool			previousBit;
	float			*syncBuffer;
	int16_t			p;
	void			synchronizeOnBitClk	(float	 *, int16_t);
signals:
	void			setCRCErrors		(int);
	void			setSyncErrors		(int);
	void			setbitErrorRate		(int);
};

#endif

