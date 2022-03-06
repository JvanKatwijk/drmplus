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
//	This class basically controls the processing of the
//	DRM+ datastream

#include	"frame-handler.h"
#include	"radio.h"
#include	"eqdisplay.h"
#include	"drm-decoder.h"

static	int goodFrames	= 0;

	frameHandler::frameHandler (drmDecoder 		*theRadio,
	                            RingBuffer<std::complex<float>> *b,
	                            RingBuffer<std::complex<float>> *audioBuffer,
	                            drmParameters	*params,
	                            RingBuffer<std::complex<float>> *eqBuffer,
	                            RingBuffer<std::complex<float>> *iqBuffer):
	                               myReader         (b, 8 * 32768),
	                               my_timeSyncer	(&myReader,
	                                                 symbolsperFrame),
	                               my_freqSyncer	(&myReader),
	                               my_wordCollector (theRadio,
	                                                 &myReader),
	                               my_timeCorrelator (),
	                               my_equalizer	(theRadio, eqBuffer),
	                               my_facProcessor	(theRadio,
	                                                 params, iqBuffer),
	                               my_sdcProcessor	(theRadio,
	                                                 params, iqBuffer) {
	this	-> theRadio	= theRadio;
	this	-> audioBuffer	= audioBuffer;
	this	-> params	= params;
	this	-> eqBuffer	= eqBuffer;
	this	-> iqBuffer	= iqBuffer;

	my_mscProcessor	= new mscProcessor (theRadio, audioBuffer,
	                                     params, iqBuffer);
	connect (this, SIGNAL (setTimeSync (bool)),
	          theRadio, SLOT (setTimeSync (bool)));
        connect (this, SIGNAL (setFACSync (bool)),
                 theRadio, SLOT (setFACSync (bool)));
	connect (this, SIGNAL (setSDCSync (bool)),
                 theRadio, SLOT (executeSDCSync (bool)));
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
std::complex<float> fine_timeBlocks [(FINE_TIME_BLOCKS + 1) * Ts_t];

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
	   bool	frameReady	= false;
	   if (!running. load ())
	      return;
//	   cleanup_db ();
	   goodFrames	= 0;

//	at first we need to find the first sample of a symbol
	   my_wordCollector. reset ();
	   setTimeSync	(false);
           setFACSync	(false);
	   int lc	= 0;
//
//	first step: find the start of a symbol, use lots of symbols
	   bool timeSynced	= my_timeSyncer. dosync (params);
	   while (!timeSynced && running. load ()) {
	      myReader. shiftBuffer (Ts_t / 2);
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
//
//	second step: figure out the course frequency offset
           my_freqSyncer. sync (params);
//	.... and write that in Hz rather than bins
	   params -> freqOffset_integer =
	             params -> freqOffset_integer * 192000 / Tu_t;
//
//	We work with a "working buffer" of size FINE_TIME_BLOCKS + 1
//	since we want to be able to handle negative time shifts
//	we start the symbols at position Ts_t / 2, so we might use
//	FINE_TIME_BLOCKS symbols for fine time syncing
	   myReader. shiftBuffer (Ts_t / 2);
	   myReader. read (fine_timeBlocks,
	                   (FINE_TIME_BLOCKS + 1) * Ts_t,
	                   params -> freqOffset_integer);
//
//	OK, here we really start, read in a full frame
	   for (int symbol = 0;
                symbol < symbolsperFrame; symbol ++) {
	      my_wordCollector. getWord (params,
	                                 fine_timeBlocks, inbank [symbol]);
	      my_timeCorrelator. getCorr (symbol, inbank [symbol]);
	   }
//
//	continue reading until we find a "first" symbol
	   lc = 0;
	   while (running. load ()) {
	      params -> timeOffset_integer =
	                  my_wordCollector.
	                             fine_timeSync (params,
	                                            fine_timeBlocks);
	      my_wordCollector. getWord (params,
	                                 fine_timeBlocks, inbank [lc]);
	      my_timeCorrelator. getCorr (lc, inbank [lc]);
	      lc = (lc + 1) % symbolsperFrame;
	      if (my_timeCorrelator. is_bestIndex (lc))  
                 break;
	   }

	   fprintf (stderr, "best index %d\n", lc);
	   if (!running. load ())
	      return;
//
//	and equalize the first frame encountered
	   for (symbol_no = 0;
                   symbol_no < symbolsperFrame; symbol_no ++)
                 (void) my_equalizer.
                    equalize (inbank [(lc + symbol_no) % symbolsperFrame],
	                                               symbol_no, outbank);

	   symbol_no         = 0;
	   frameReady        = false;
//
//	our equalizer will take some symbols as look ahead,
//	so we continue
	   while (!frameReady && running. load ()) {
	      params -> timeOffset_integer =
	                  my_wordCollector.
	                             fine_timeSync (params,
	                                            fine_timeBlocks);
	      my_wordCollector.
                      getWord (params,
	                       fine_timeBlocks, inbank [lc]);
	      frameReady = my_equalizer. equalize (inbank [lc],
                                                   symbol_no,
                                                   outbank);
	      lc	= (lc + 1) % symbolsperFrame;
	      symbol_no = (symbol_no + 1) % symbolsperFrame;
	   }

	   if (!running. load ())
	      return;
//
//	it seems we have a frame, check the FAC
	   bool fac_synced	= false;
	   fprintf (stderr, "lc = %d\n", lc);
	   if (my_facProcessor. processFAC (outbank, 
	                                    my_equalizer. getMeanEnergy (),
		                            my_equalizer. getChannels   ())) {
	      setFACSync (true);
	      fac_synced	= true;
	   }
	   else {
	      setFACSync (false);	// sorry ....
	      goto L1;
	   }
//
//	The main loop, we assume everything is allright, and
//	
	   while (running. load ()) {
	      float	angle			= 0;
	      float v;
	      frameReady	= false;
	      for (i = 0; !frameReady && (i < symbolsperFrame); i ++) {
	          params -> timeOffset_integer =
	                  my_wordCollector.
	                             fine_timeSync (params,
	                                            fine_timeBlocks);
	         angle = my_wordCollector.
	              getWord (params,
	                       fine_timeBlocks,
	                       inbank [(lc + i) % symbolsperFrame]);
	         frameReady = my_equalizer.
	                 equalize (inbank [(lc + i) % symbolsperFrame],
	                           (symbol_no + i) % symbolsperFrame,
	                           outbank,
	                           &params -> timeOffset_fractional,
	                           &v,
	                           &params -> sampleRate_offset);
	      }

	      static int displayDelay = 0;
	      if (++ displayDelay > 4) {
	         float temp = SAMPLE_RATE / (float)Tu_t * 1.0 / (2 * M_PI);
	         show_angle		(angle * temp);
	         show_coarseOffset	(params -> freqOffset_integer);
	         show_timeDelay		(params -> timeOffset_fractional);
	         show_clockOffset	(params -> sampleRate_offset);
	         displayDelay = 0;
	      }

	      if (!my_facProcessor. processFAC (outbank,
	                                        my_equalizer. getMeanEnergy (),
	                                        my_equalizer. getChannels ())) {
	         setFACSync (false);
	         fprintf (stderr, "%d good frames before error with time error %f and timeOffset %d\n",
	                               goodFrames,
	                               params -> timeOffset_fractional,
	                               params -> timeOffset_integer);
	         break;
	      }

	      goodFrames ++;
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
//
//	if we - for whatever reason - left the loop,
//	we're done, either need to stop or lost sync
	   if (!running. load ())
	      return;
	   goto L1;
	}
	catch (int e) {}
	
	for (i = 0; i < symbolsperFrame; i ++) {
	   delete [] inbank [i];
	   delete [] outbank [i];
	}
}

void	frameHandler::selectService	(int shortId) {
	my_mscProcessor -> selectService (shortId);
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

