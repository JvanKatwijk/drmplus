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

//	design of the central framehandler for DAB+ decoder

#include	"frame-handler.h"
#include	"radio.h"
#include	"eqdisplay.h"
#include	"drm-decoder.h"

	frameHandler::frameHandler (drmDecoder 		*theRadio,
	                            RingBuffer<std::complex<float>> *b,
	                            drmParameters	*params,
	                            RingBuffer<std::complex<float>> *eqBuffer,
	                            RingBuffer<std::complex<float>> *iqBuffer):
	                               myReader         (b, 4 * 32768),
	                               my_timeSyncer	(&myReader, 30),
	                               my_freqSyncer	(&myReader),
	                               my_wordCollector (&myReader),
	                               my_timeCorrelator (),
	                               my_equalizer	(theRadio, eqBuffer),
//	                               my_mscProcessor  (theRadio,
//	                                                 params, iqBuffer),
	                               my_facProcessor	(theRadio,
	                                                 params, iqBuffer),
	                               my_sdcProcessor	(theRadio,
	                                                 params, iqBuffer) {
	this	-> theRadio	= theRadio;
	this	-> params	= params;
	this	-> eqBuffer	= eqBuffer;
	this	-> iqBuffer	= iqBuffer;
	this	-> nSymbols	= 20;

	my_mscProcessor	= new mscProcessor (theRadio, params, iqBuffer);
	connect (this, SIGNAL (setTimeSync (bool)),
	          theRadio, SLOT (setTimeSync (bool)));
        connect (this, SIGNAL (setFACSync (bool)),
                 theRadio, SLOT (setFACSync (bool)));
	connect (this, SIGNAL (setSDCSync (bool)),
                 theRadio, SLOT (executeSDCSync (bool)));
	connect (this, SIGNAL (show_fineOffset (float)),
                 theRadio, SLOT (show_fineOffset (float)));
        connect (this, SIGNAL (show_coarseOffset (float)),
                 theRadio, SLOT (show_coarseOffset (float)));
        connect (this, SIGNAL (show_timeDelay   (float)),
                 theRadio, SLOT (show_timeDelay (float)));
        connect (this, SIGNAL (show_timeOffset  (float)),
                 theRadio, SLOT (show_timeOffset (float)));
        connect (this, SIGNAL (show_clockOffset (float)),
                 theRadio, SLOT (show_clockOffset (float)));
        connect (this, SIGNAL (show_angle (float)),
                 theRadio, SLOT (show_angle (float)));
	connect (this, SIGNAL (update_GUI ()),
	         theRadio, SLOT (update_GUI ()));
	connect (this, SIGNAL (cleanup_db ()),
	         theRadio, SLOT (cleanup_db ()));
	set_constellationView	("FAC");
	start ();
}

	frameHandler::~frameHandler () {
	running. store (false);
	myReader. stopSign ();
	while (isRunning ())
	   usleep (1000);
}


void	frameHandler::run () {
std::complex<float> *inbank  [symbolsperFrame];
theSignal	    *outbank [symbolsperFrame];
int	symbol_no;
int	i;
int	updateTimer	= 10;
	for (i = 0; i < symbolsperFrame; i ++) {
	   inbank [i]	= new std::complex<float> [nrCarriers];
	   outbank [i]	= new theSignal [nrCarriers];
	}
	cleanup_db ();
	running. store (true);
	myReader. shiftBuffer (Ts_t / 2);
	try {
	   bool the_sdcFlag = false;
L1:
	   if (!running. load ())
	      return;
//	at first we need to find the first sample of a symbol
	   my_wordCollector. reset ();
	   setTimeSync	(false);
           setFACSync	(false);
	   int lc	= 0;
	   bool	frameReady	= false;
	   bool timeSynced	= my_timeSyncer. dosync (params);
	   while (!timeSynced && running. load ()) {
	      myReader. shiftBuffer (Ts_t / 4);
	      timeSynced = my_timeSyncer. dosync (params);
	   }
	   if (!running. load ())
	      return;

	   setTimeSync (true);
	   params -> timeOffset_integer =
	                       floor (params -> timeOffset + 0.5);
	   params -> timeOffset_fractional =
	                       params -> timeOffset - 
	                                      params -> timeOffset_integer;
	   fprintf (stderr,
	           "found timeoffset %d, sampleRateOffset %f, freqOffset %d\n",
	           params -> timeOffset_integer,
	           params -> sampleRate_offset,
	           (int)(params -> freqOffset_fract / (2 * M_PI) * 192000 / Tu_t));
	   myReader. shiftBuffer (floor (params -> timeOffset_integer));
	   params -> timeOffset_integer = 0;
           my_freqSyncer. sync (params);

	   int32_t intOffset = params -> freqOffset_integer;
//	we now read in a full frame to detect/determine
//	the very first symbol of that frame
	   for (int symbol = 0;
                symbol < symbolsperFrame; symbol ++) {
	      my_wordCollector. getWord (inbank [symbol], params);
	      my_timeCorrelator. getCorr (symbol, inbank [symbol]);
	      params -> timeOffset_fractional +=
	                           Ts_t * params -> sampleRate_offset;
	   }
//
//	continue reading until we find a "first" symbol
	   lc = 0;
	   while (running. load ()) {
	      my_wordCollector. getWord (inbank [lc], params);
	      my_timeCorrelator. getCorr (lc, inbank [lc]);
	      params -> timeOffset_fractional +=
	                           Ts_t * params -> sampleRate_offset;
	      lc = (lc + 1) % symbolsperFrame;
	      if (my_timeCorrelator. is_bestIndex (lc))  
                 break;
	   }

	   if (!running. load ())
	      return;
	   for (symbol_no = 0;
                   symbol_no < symbolsperFrame; symbol_no ++)
                 (void) my_equalizer.
                    equalize (inbank [(lc + symbol_no) % symbolsperFrame],
                                                    symbol_no, outbank);

	   symbol_no         = 0;
	   frameReady        = false;
	   while (!frameReady && running. load ()) {
	      my_wordCollector.
                      getWord (inbank [lc], params);
	      params -> timeOffset_fractional +=
	                           Ts_t * params -> sampleRate_offset;
	      frameReady = my_equalizer. equalize (inbank [lc],
                                                   symbol_no,
                                                   outbank);
	      lc	= (lc + 1) % symbolsperFrame;
	      symbol_no = (symbol_no + 1) % symbolsperFrame;
	   }

	   if (!running. load ())
	      return;

	static int facError = 0;
	   bool fac_synced	= false;
	   fprintf (stderr, "lc = %d\n", lc);
	   if (my_facProcessor. processFAC (outbank, 
	                                    my_equalizer. getMeanEnergy (),
		                            my_equalizer. getChannels   ())) {
	      setFACSync (true);
	      fac_synced	= true;
	      facError		= 0;
	   }
	   else {
	      setFACSync (false);
	      goto L1;
	   }
	   bool firstTime = true;
	   while (running. load ()) {
	      float	offsetFractional	= 0;    //
	      int16_t	offsetInteger		= 0;
	      float	deltaFreqOffset		= 0;
	      float	sampleclockOffset	= 0;
	      float	angle			= 0;
	      frameReady	= false;
	      for (i = 0; !frameReady && (i < symbolsperFrame); i ++) {
	         angle = my_wordCollector.
	              getWord (inbank [(lc + i) % symbolsperFrame], params);
	         frameReady = my_equalizer.
	                 equalize (inbank [(lc + i) % symbolsperFrame],
	                           (symbol_no + i) % symbolsperFrame,
	                           outbank,
	                           &params -> timeOffset_fractional,
	                           &params -> freqOffset_fract,
	                           &params -> sampleRate_offset);
	      }
	      static int displayDelay = 0;
	      if (++ displayDelay > 4) {
	         float temp = SAMPLE_RATE / (float)Tu_t * 1.0 / (2 * M_PI);
	         show_angle		(angle * temp);
	         show_fineOffset	(deltaFreqOffset * temp);
	         show_coarseOffset	(params -> freqOffset_integer * SAMPLE_RATE / (float)Tu_t);
	         show_timeDelay		(params -> timeOffset_fractional);
	         show_clockOffset	(params -> sampleRate_offset);
	         displayDelay = 0;
	      }
	      if (!my_facProcessor. processFAC (outbank,
	                                        my_equalizer. getMeanEnergy (),
	                                        my_equalizer. getChannels ())) {
	         if (facError >= 1) {
	            setFACSync (false);
	            break;
	         }
	         else
	           facError ++;
	      }

	      if ((params -> theChannel. Identity == 0) || 	
	          (params -> theChannel. Identity == 3)) {
	         bool sdcFlag = my_sdcProcessor. processSDC (outbank);
	         if (sdcFlag != the_sdcFlag) {
	            the_sdcFlag = sdcFlag;
	            setSDCSync (the_sdcFlag);
	         }
	      }
	      updateTimer --;
	      if (updateTimer == 0) {
	         update_GUI ();
	         updateTimer = 10;
	      }
	      my_mscProcessor ->  processFrame (outbank);
	   }
	   if (!running. load ())
	      return;
	   goto L1;
	}
	catch (int e) {}
}

void	frameHandler::selectService	(int stream) {
	my_mscProcessor -> selectService (stream);
}

void	frameHandler::set_constellationView	(const QString &s) {
	if (s == "FAC") {
	   my_mscProcessor ->	set_Viewer (false);
	   my_sdcProcessor.	set_Viewer (false);
	   my_facProcessor.	set_Viewer (true);	
	}
	else
	if (s == "SDC") {
	   my_mscProcessor ->	set_Viewer (false);
	   my_facProcessor.	set_Viewer (false);	
	   my_sdcProcessor.	set_Viewer (true);
	}
	else {
	   my_facProcessor.	set_Viewer (false);	
	   my_sdcProcessor.	set_Viewer (false);
	   my_mscProcessor ->	set_Viewer (true);
	}
}

