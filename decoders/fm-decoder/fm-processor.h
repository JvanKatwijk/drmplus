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
 *    DRM+ Decoder is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef	__FM_PROCESSOR__
#define	__FM_PROCESSOR__

#include	<QThread>
#include	<QObject>
#include	<atomic>
#include	"radio-constants.h"
#include	"fir-filters.h"
#include	"fft-filters.h"
#include	"sincos.h"
#include	"pllC.h"
#include	"ringbuffer.h"

class		fmDecoder;
class		fm_Demodulator;
class		rdsDecoder;
class		audioSink;
class		rateConverter;

#define PILOTFILTER_SIZE        31
#define RDSLOWPASS_SIZE         89
#define HILBERT_SIZE            13
#define RDSBANDFILTER_SIZE      49      
#define FFT_SIZE                256
#define PILOT_WIDTH             1000
#define RDS_WIDTH               1500

class	fmProcessor:public QThread {
Q_OBJECT
public:
			fmProcessor (fmDecoder *,
	                             RingBuffer<std::complex<float>> *,
	                             RingBuffer<std::complex<float>> *);
	        	~fmProcessor (void);
	void		stop		(void);		// stop the processor
	void		setDeemphasis	(int16_t);
	void		resetRds	(void);
	void		set_squelchMode	(bool);
	void		set_squelchValue	(int16_t);
	const char *	nameofDecoder	(void);

	enum Channels {
	   S_STEREO		= 0,
	   S_LEFT		= 1,
	   S_RIGHT		= 2,
	   S_LEFTplusRIGHT	= 0103,
	   S_LEFTminusRIGHT	= 0104
	};
	enum Mode {
	   FM_STEREO	= 0,
	   FM_MONO	= 1
	};

	void		setfmMode	(uint8_t m);
	void		setFMdecoder	(int8_t d);
	void		setAttenuation	(int16_t l, int16_t r);
private:
virtual	void		run		(void);
	fmDecoder	*myDecoder;
	RingBuffer<std::complex<float>> *inputData;
	RingBuffer<std::complex<float>> *audioData;
	int32_t		inputRate;
	int32_t		fmRate;
	int32_t		workingRate;
	int32_t		audioRate;
	void		sendSampletoOutput	(std::complex<float>);
	std::atomic<bool>	running;
	SinCos		*mySinCos;
	uint8_t		fmModus;
	uint8_t		selector;
	int16_t		balance;
	int16_t		leftChannel;
	int16_t		rightChannel;
	int16_t		Lgain;
	int16_t		Rgain;
        int16_t         old_squelchValue;
        int16_t         squelchValue;
	bool		squelchOn;

	bool		pilotLocked		();
	float		get_dcComponent		();
	rateConverter	*audioDecimator;
	std::complex<float>	*audioOut;
	rdsDecoder	*myRdsDecoder;

	void		stereo	(float, std::complex<float> *, float *);
	void		mono	(float, std::complex<float> *, float *);
	fftFilter	*pilotBandFilter;
	fftFilter	*rdsBandFilter;
	fftFilter	*rdsLowPassFilter;
	HilbertFilter	*rdsHilbertFilter;

	float		pilotDelay;
	std::complex<float>	audioGainCorrection	(std::complex<float>);
	float	audioGain;
	int32_t		max_freq_deviation;
	int32_t		norm_freq_deviation;
	float		omega_demod;

	float		peakLevel;
	int32_t		peakLevelcnt;
	fm_Demodulator	*theDemodulator;

	int8_t		rdsModus;

	float		noiseLevel;
	float		pilotLevel;
	float		rdsLevel;
	int8_t		viewSelector;
	pllC		*rds_plldecoder;
	float		K_FM;
	int		myCount;
	float	xkm1;
	float	ykm1;
	float	alpha;
	class	pilotRecovery {
	   private:
	      int32_t	Rate_in;
	      float	pilot_OscillatorPhase;
	      float	pilot_oldValue;
	      float	omega;
	      float	gain;
	      SinCos	*mySinCos;
	      float	pilot_Lock;
	      bool	pll_isLocked;
	      float	quadRef;
	      float	accumulator;
	      int32_t	count;
	   public:
	      pilotRecovery (int32_t	Rate_in,
	                     float	omega,
	                     float	gain,
	                     SinCos	*mySinCos) {
	         this	-> Rate_in	= Rate_in;
	         this	-> omega	= omega;
	         this	-> gain		= gain;
	         this	-> mySinCos	= mySinCos;
	         pll_isLocked		= false;
	         pilot_Lock		= 0;
	         pilot_oldValue		= 0;
	         pilot_OscillatorPhase	= 0;
	      }

	      ~pilotRecovery (void) {
	      }

	      bool	isLocked (void) {
	         return pll_isLocked;
	      }

	      float	getPilotPhase	(float pilot) {
	      float	OscillatorValue =
	                  mySinCos -> getCos (pilot_OscillatorPhase);
	      float	PhaseError	= pilot * OscillatorValue;
	      float	currentPhase;
	         pilot_OscillatorPhase += PhaseError * gain;
	         currentPhase		= PI_Constrain (pilot_OscillatorPhase);

	         pilot_OscillatorPhase =
	                   PI_Constrain (pilot_OscillatorPhase + omega);
	         
	         quadRef	= (OscillatorValue - pilot_oldValue) / omega;
//	         quadRef	= PI_Constrain (quadRef);
	         pilot_oldValue	= OscillatorValue;
	         pilot_Lock	= 1.0 / 30 * (- quadRef * pilot) +
	                          pilot_Lock * (1.0 - (1.0 / 30)); 
	         pll_isLocked	= pilot_Lock > 0.1;
	         return currentPhase;
	      }
	};
	      
	pilotRecovery	*pilotRecover;

signals:
	void	audioAvailable	(int, int);
	void	showLocked	(bool, float);
};

#endif

