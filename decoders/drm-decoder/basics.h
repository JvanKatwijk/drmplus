#
/*
 *    Copyright (C) 2013 .. 2017
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
 *    DRM+ is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
//
//	just some very general stuff
#ifndef	__DRM_BASICS__
#define	__DRM_BASICS__
#include	<QString>
#include	<stdint.h>
#include	<complex>

#define	SAMPLE_RATE	192000

//enum    {
//        QAM4, QAM16, QAM64
//};
//
//enum    {
//        AUDIO_STREAM, DATA_STREAM
//};

#define	FINE_TIME_BLOCKS	2
typedef struct {
	uint8_t		Identity;		// FAC
	uint8_t		RMFlag;
	uint8_t		InterleaverDepthFlag;
	uint8_t		MSC_Mode;
	uint8_t		sdcMode;
	uint8_t		serviceEncoding;
	int		nrServices;
	uint8_t		toggleFlag;
} channelParameters;

typedef struct {
	bool		inUse;
	uint32_t	serviceId;
	uint8_t		shortId;
	QString		serviceName;
	QString		programType;
	QString		languagetxt;
	QString		country;
	int		language;		// set by FAC
	bool		is_audio;
	uint8_t		serviceDescriptor;
	uint8_t		streamId;
	int16_t		lengthHigh;
	int16_t		lengthLow;
	int		offsetHigh;
	int		offsetLow;
	uint8_t		audioCoding;
	uint8_t		sbrFlag;
	uint8_t		audioMode;
	uint8_t		audioSamplingRate;
	uint8_t		textFlag;
	uint8_t		enhancementFlag;
	uint8_t		coderField;

	uint8_t		packetModeInd;
	uint8_t		domain;
	uint8_t		dataUnitIndicator;
	int16_t		packetId;
	int16_t		packetLength;
	int		applicationId;
	bool		FEC;
	int		R, C;
} streamParameters;

typedef struct	{
	float		timeOffset;
	float		sampleRate_offset;
	float		freqOffset_fract;
	float		timeOffset_fractional;
	int16_t		freqOffset_integer;
	int16_t		timeOffset_integer;
	int		nullCarrier;
//
	channelParameters	theChannel;
	streamParameters	theStreams [4];
	uint8_t		protLevelA;		// set by SDC
	uint8_t		protLevelB;		// set by SDC
	int		muxLength;
	int		interleaverDepth;	// set by SDC
	int		audioServices;		// set by FAC
	int		dataServices;		// set by FAC
//
	int		Year;
	int		Month;
	int		Day;
	int		hours;
	int		minutes;
} drmParameters;

typedef struct	ourSignal {
	std::complex<float>	signalValue;
	double		rTrans;
} theSignal;

typedef struct metrics_struct {
	float	rTow0;
	float	rTow1;
} metrics;

//
//	rChan was computed by the equalizer,
//	in the current setting, the best results are
//	with a plain rDist (or squared)
static inline
float computeMetric (const float rDist, const float rChan) {
	return rDist * rDist * rChan;
//	return rDist * rChan;
//	return rDist * rDist;
	return rDist;
#ifdef USE_MAX_LOG_MAP
/* | r / h - s | ^ 2 * | h | ^ 2 */
	return rDist * rDist * rChan;
#else
/* | r / h - s | * | h | */
	return rDist * rChan;
#endif
}

static inline
float Minimum1 (const float rA, const float rB, const float rChan) {
//	The minimum in case of only one parameter is trivial 
//	return fabs(rA - rB);
	return computeMetric (fabs(rA - rB), rChan);
}

static inline
float	Minimum2 (const float rA,	// value to consider
	          const float rB1,	// reference 1
	          const float rB2,	// reference 2
	          const float rChan) {
        /* First, calculate all distances */
const float rResult1 = fabs (rA - rB1);
const float rResult2 = fabs (rA - rB2);

/* Return smallest one */
        return  (rResult1 < rResult2) ?
	              computeMetric (rResult1, rChan) :
	              computeMetric (rResult2, rChan);
}

static inline
float	Minimum2 (const float rA,
	          const float rX0,
	          const float rX1,
                  const float rChan,
	          const float rLVal0) {
        /* X0: L0 < 0
           X1: L0 > 0 */

/* First, calculate all distances */
float rResult1	= computeMetric (fabs (rA - rX0), rChan);
float rResult2	= computeMetric (fabs (rA - rX1), rChan);

/* Add L-value to metrics which to not correspond to correct hard decision */
	if (rLVal0 > 0.0)
            rResult1 += rLVal0;
        else
            rResult2 -= rLVal0;

/* Return smallest one */
        if (rResult1 < rResult2)
            return rResult1;
        else
            return rResult2;
}

static inline 
float	Minimum4 (const float rA, const float rB1, const float rB2,
	          const float rB3, float rB4, const float rChan) {
/* First, calculate all distances */
const float rResult1 = fabs (rA - rB1);
const float rResult2 = fabs (rA - rB2);
const float rResult3 = fabs (rA - rB3);
const float rResult4 = fabs (rA - rB4);

        /* Search for smallest one */
float rReturn = rResult1;
        if (rResult2 < rReturn)
            rReturn = rResult2;
        if (rResult3 < rReturn)
            rReturn = rResult3;
        if (rResult4 < rReturn)
            rReturn = rResult4;

        return computeMetric (rReturn, rChan);
}

static inline
std::complex<float> valueFor (float amp, int16_t phase) {
	return std::complex<float> (amp * cos (2 * M_PI * phase / 1024),
	                            amp * sin (2 * M_PI * phase / 1024));
}

#define		BITSPERBYTE	8

#define		Ts_t		480
#define		Tu_t		432
#define		Tg_t		48
#define		symbolsperFrame	40
#define		groupsperFrame	10
#define		pilotDistance	16
#define		symbolspergroup	(symbolsperFrame / groupsperFrame)
#define		K_min		-106
#define		K_max		+106
#define		nrCarriers	(K_max - K_min + 1)
float		sinc		(float);
#endif

