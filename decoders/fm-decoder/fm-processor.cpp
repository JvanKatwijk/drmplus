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
#include	"fm-processor.h"
#include	"fm-decoder.h"
#include	"fm-demodulator.h"
#include	"rds-decoder.h"
#include	"sincos.h"
#include	"rate-converter.h"
#include	"squelchClass.h"

#define	AUDIO_FREQ_DEV_PROPORTION 0.85f
#define	PILOT_FREQUENCY		19000
#define	RDS_FREQUENCY		(3 * PILOT_FREQUENCY)
#define	OMEGA_DEMOD		2 * M_PI / fmRate
#define	OMEGA_PILOT	((float (PILOT_FREQUENCY)) / fmRate) * (2 * M_PI)
#define	OMEGA_RDS	((float) RDS_FREQUENCY / fmRate) * (2 * M_PI)

//
	fmProcessor::fmProcessor (fmDecoder	*RI,
	                          RingBuffer<std::complex<float>> *inputData,
	                          RingBuffer<std::complex<float>> *audioData) {
	running. store (false);
	this	-> myDecoder		= RI;
	this	-> inputData		= inputData;
	this	-> audioData		= audioData;
	this	-> inputRate		= 192000;
	this	-> fmRate		= 192000;
	this	-> audioRate		= 48000;
	Lgain				= 20;
	Rgain				= 20;

	myRdsDecoder			= NULL;

	this	-> mySinCos		= new SinCos (fmRate);
	this	-> omega_demod		= 2 * M_PI / fmRate;
/*
 *	default values, will be set through the user interface
 *	to their appropriate values
 */
	this	-> fmModus		= FM_STEREO;
	this	-> selector		= S_STEREO;
	this	-> balance		= 0;
	this	-> leftChannel		= - (balance - 50.0) / 100.0;
	this	-> rightChannel		= (balance + 50.0) / 100.0;
	this	-> audioDecimator	=
	                         new rateConverter (fmRate,
	                                            audioRate,
	                                            audioRate / 200);
	this	-> audioOut		=
	                         new std::complex<float> [audioDecimator ->
	                                                  getOutputsize ()];

	connect (this, SIGNAL (audioAvailable (int, int)),
	         myDecoder, SLOT (audioAvailable (int, int)));
	connect (this, SIGNAL (showLocked (bool, float)),
	         myDecoder, SLOT (showLocked (bool, float)));
/*
 *	averagePeakLevel and audioGain are set
 *	prior to calling the processFM method
 */
	this	-> peakLevel		= -100;
	this	-> peakLevelcnt		= 0;
	this	-> max_freq_deviation	= 0.95 * (0.5 * fmRate);
	this	-> norm_freq_deviation	= 0.6 * max_freq_deviation;
	this	-> audioGain		= 20;
	pilotLevel			= 0;

	this	-> squelchOn		= false;
	squelchValue			= 0;
	old_squelchValue		= 0;
//	to isolate the pilot signal, we need a reasonable
//	filter. The filtered signal is beautified by a pll
	pilotBandFilter		= new fftFilter (FFT_SIZE, PILOTFILTER_SIZE);
	pilotBandFilter		-> setBand (PILOT_FREQUENCY - PILOT_WIDTH / 2,
		                            PILOT_FREQUENCY + PILOT_WIDTH / 2,
		                            fmRate);
	pilotRecover		= new pilotRecovery (fmRate,
	                                             OMEGA_PILOT,
	                                             25 * omega_demod,
	                                             mySinCos);
	pilotDelay	= (FFT_SIZE - PILOTFILTER_SIZE) * OMEGA_PILOT;

	rdsLowPassFilter	= new fftFilter (FFT_SIZE, RDSLOWPASS_SIZE);
	rdsLowPassFilter	-> setLowPass (RDS_WIDTH, fmRate);
//
//	the constant K_FM is still subject to many questions
	float	F_G	= 0.65 * fmRate / 2; // highest freq in message
	float	Delta_F	= 0.95 * fmRate / 2;	//
	float	B_FM	= 2 * (Delta_F + F_G);
	K_FM			= B_FM * M_PI / F_G;
	theDemodulator		= new fm_Demodulator (fmRate,
	                                              mySinCos, K_FM);
	theDemodulator	-> setDecoder (4);
//
//	In the case of mono we do not assume a pilot
//	to be available. We borrow the approach from CuteSDR
	rdsHilbertFilter	= new HilbertFilter (HILBERT_SIZE,
	                                     (float)RDS_FREQUENCY / fmRate,
	                                             fmRate);
	rdsBandFilter		= new fftFilter (FFT_SIZE,
	                                         RDSBANDFILTER_SIZE);
	rdsBandFilter		-> setBand (RDS_FREQUENCY - RDS_WIDTH / 2,
	                                      RDS_FREQUENCY + RDS_WIDTH / 2,
	                                      fmRate);
	rds_plldecoder		= new pllC (fmRate,
	                                    RDS_FREQUENCY,
	                                    RDS_FREQUENCY - 50,
	                                    RDS_FREQUENCY + 50,
	                                    200,
	                                    mySinCos);

//	for the deemphasis we use an in-line filter with
	xkm1			= 0;
	ykm1			= 0;
	alpha			= 1.0 / (fmRate / (1000000.0 / 50.0 + 1));

	myCount			= 0;
	start	();
}

	fmProcessor::~fmProcessor (void) {
	stop	();
}

void	fmProcessor::set_squelchValue	(int16_t n) {
	squelchValue	= n;
}

void	fmProcessor::set_squelchMode	(bool b) {
	squelchOn	= b;
}

void	fmProcessor::stop	(void) {
	if (running. load ()) {
	   running. store (false);
	   while (!isFinished ())
	      usleep (100);
	}
}

const char  *fmProcessor::nameofDecoder	(void) {
	if (running. load ())
	   return theDemodulator -> nameofDecoder ();
	return " ";
}
//
void	fmProcessor::setfmMode (uint8_t m) {
	fmModus	= m ? FM_STEREO : FM_MONO;
}

void	fmProcessor::setFMdecoder (int8_t d) {
	if (running. load ())
	   theDemodulator	-> setDecoder (d);
}

//	Deemphasis	= 50 usec (3183 Hz, Europe)
//	Deemphasis	= 75 usec (2122 Hz US)
//	tau		= 2 * M_PI * Freq = 1000000 / time
void	fmProcessor::setDeemphasis	(int16_t v) {
float	Tau;
	switch (v) {
	   default:
	      v	= 1;
	   case	1:
	   case 50:
	   case 75:
//	pass the Tau
	      Tau	= 1000000.0 / v;
	      alpha	= 1.0 / (float (fmRate) / Tau + 1.0);
	}
}


void	fmProcessor::setAttenuation (int16_t l, int16_t r) {
	Lgain	= l;
	Rgain	= r;
}

//	In this variant, we have a separate thread for the
//	fm processing

void	fmProcessor::run (void) {
std::complex<float>	result;
float 		rdsData;
int32_t		bufferSize	= 2 * 8192;
std::complex<float>	dataBuffer [bufferSize];
int32_t		i, k;
std::complex<float>	out;
float		audioGainAverage	= 0;
#define	RDS_DECIMATOR	8
squelch		mySquelch (1, fmRate / 10, fmRate / 20, fmRate);

	myRdsDecoder		= new rdsDecoder (myDecoder,
	                                          fmRate / RDS_DECIMATOR,
	                                          mySinCos);

	running. store (true);		// will be set from the outside
	while (running. load ()) {
	   while (running. load () &&
	          (inputData -> GetRingBufferReadAvailable () < bufferSize)) 
	      msleep (1);	// should be enough
	   if (!running. load ())
	      break;

           if (squelchValue != old_squelchValue) {
              mySquelch. setSquelchLevel (squelchValue);
              old_squelchValue = squelchValue;
           }

	   int amount = inputData -> getDataFromBuffer (dataBuffer, bufferSize);

//	We assume that if/when the pilot is no more than 3 db's above
//	the noise around it, it is better to decode mono
	   for (i = 0; i < amount; i ++) {
	      std::complex<float> v = 
	          std::complex<float> (real (dataBuffer [i]) * Lgain,
	                               imag (dataBuffer [i]) * Rgain);
	      if (abs (v) > peakLevel)
	         peakLevel = abs (v);
	      if (++peakLevelcnt >= fmRate / 4) {
	         float	ratio	= 
	                          max_freq_deviation / norm_freq_deviation;
	         if (peakLevel > 0)
	            this -> audioGain	= 
	                  (ratio / peakLevel) / AUDIO_FREQ_DEV_PROPORTION;
	         if (audioGain <= 0.1)
	            audioGain = 0.1;
	         audioGain	= 0.99 * audioGainAverage + 0.01 * audioGain;
	         audioGainAverage = audioGain;
	         peakLevelcnt	= 0;
//	         fprintf (stderr, "peakLevel = %f\n", peakLevel);
	         peakLevel	= -100;
	      }

	      float demod	= theDemodulator  -> demodulate (v);
	      
	      if ((fmModus == FM_STEREO)) {
	         stereo (demod, &result, &rdsData);
	      }
	      else
	         mono (demod, &result, &rdsData);

	      result	= cmul (result, audioGain);
//	"result" now contains the audio sample, either stereo
//	or mono
	      int audioAmount = 0;
	      if (audioDecimator -> convert (result, audioOut, &audioAmount)) {
	         for (k = 0; k < audioAmount; k ++) 
                    if (squelchOn)
	              audioOut [k] = mySquelch. do_squelch (audioOut [k]);

	         audioData -> putDataIntoBuffer (audioOut, audioAmount);
	         if (audioData -> GetRingBufferReadAvailable () >=
	                                                audioRate / 10)
	            audioAvailable (audioRate / 10, audioRate);
	      }

	      if (true) {
	         float mag;
	         static int cnt = 0;
	         if (++cnt >= RDS_DECIMATOR) {
	            myRdsDecoder -> doDecode (rdsData, &mag,
	                                      (rdsDecoder::RdsMode)1);
	            cnt = 0;
	         }
	      }
	      if (++myCount > fmRate) {
	         myCount = 0;
	         showLocked (pilotLocked (),
	                     get_dcComponent   ());
	      }
	   }
	}
}

void	fmProcessor::mono (float	demod,
	                   std::complex<float>	*audioOut,
	                   float	*rdsValue) {
float	Re, Im;
std::complex<float>	rdsBase;

//	deemphasize
	Re	= xkm1 = (demod - xkm1) * alpha + xkm1;
	Im	= ykm1 = (demod - ykm1) * alpha + ykm1;
	*audioOut	= std::complex<float> (Re, Im);
//
//	fully inspired by cuteSDR, we try to decode the rds stream
//	by simply am decoding it (after creating a decent complex
//	signal by Hilbert filtering)
	rdsBase	= std::complex<float> (5 * demod, 5 * demod);
	rdsBase = rdsHilbertFilter -> Pass (rdsBandFilter -> Pass (rdsBase));
	rds_plldecoder -> do_pll (rdsBase);
	float rdsDelay = imag (rds_plldecoder -> getDelay ());
	*rdsValue = rdsLowPassFilter -> Pass (5 * rdsDelay);
}

void	fmProcessor::stereo (float	demod,
	                     std::complex<float>	*audioOut,
	                     float	*rdsValue) {
float	LRPlus	= 0;
float	LRDiff	= 0;
float	pilot	= 0;
float	currentPilotPhase;
float	PhaseforLRDiff	= 0;
float	PhaseforRds	= 0;
/*
 */
	LRPlus = LRDiff = pilot	= demod;
/*
 *	get the phase for the "carrier to be inserted" right
 */
	pilot		= pilotBandFilter -> Pass (5 * pilot);
	currentPilotPhase = pilotRecover -> getPilotPhase (5 * pilot);
/*
 *	Now we have the right - i.e. synchronized - signal to work with
 */
	PhaseforLRDiff	= 2 * (currentPilotPhase + pilotDelay);
	PhaseforRds	= 3 * (currentPilotPhase + pilotDelay);
//
//	Due to filtering the real amplitude of the LRDiff might have
//	to be adjusted, we guess
	LRDiff	= 2.0 * mySinCos	-> getCos (PhaseforLRDiff) * LRDiff;
	float  MixerValue = mySinCos -> getCos (PhaseforRds);
	*rdsValue = 5 * rdsLowPassFilter -> Pass (MixerValue * demod);

//	apply deemphasis
	LRPlus		= xkm1	= (LRPlus - xkm1) * alpha + xkm1;
	LRDiff		= ykm1	= (LRDiff - ykm1) * alpha + ykm1;
        *audioOut		= std::complex<float> (LRPlus, LRDiff);
}

void	fmProcessor::sendSampletoOutput (std::complex<float> s) {

	audioData -> putDataIntoBuffer (&s, 1);
	if (audioData -> GetRingBufferReadAvailable () >= audioRate / 10)
	   audioAvailable (audioRate / 10, audioRate);
}

bool	fmProcessor::pilotLocked  (void) {
	if (running. load ())
	   return pilotRecover	-> isLocked ();
        return false;
}

float	fmProcessor::get_dcComponent    (void) {
        if (running. load ())
           return theDemodulator        -> get_DcComponent ();
        return 0.0;
}

