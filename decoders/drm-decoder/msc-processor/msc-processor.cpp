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

#include	"msc-processor.h"
#include	"drm-decoder.h"
#include	"msc-handler.h"
#include	"qam4-handler.h"
#include	"qam16-handler.h"
#include	"referenceframe.h"
#include	"data-processor.h"
#include	"audioframe-processor.h"

	mscProcessor::mscProcessor (drmDecoder		*parent,
	                            drmParameters	*params,
	                            RingBuffer<std::complex<float>> *iqBuffer) {
	int nrCells	= 0;
	int symbol, carrier;
	this	-> theParent	= parent;
	this	-> params	= params;
	this	-> iqBuffer	= iqBuffer;
	show_Constellation	= false;
	this	-> the_postProcessor	= nullptr;
	for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	   for (carrier = K_min; carrier <= K_max; carrier ++) {
	      if (isFACcell (symbol, carrier)) {
	         frame_1 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_4 [symbol * nrCarriers + (carrier - K_min)] = false;
	      }
	      else
	      if (isGainCell (symbol, carrier)) {
	         frame_1 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_4 [symbol * nrCarriers + (carrier - K_min)] = false;
	      }
	      else {
	         frame_1 [symbol * nrCarriers + (carrier - K_min)] = true;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = true;
	         frame_4 [symbol * nrCarriers + (carrier - K_min)] = true;
	      }
	   }
	}
//
//	the timecells are only in symbol 0
	for (carrier = K_min; carrier <= K_max; carrier ++) {
	   if (isTimeCell (carrier)) {
	      frame_1 [carrier - K_min] = false;
	      frame_23 [carrier - K_min] = false;
	      frame_4 [carrier - K_min] = false;
	   }
	}

//	some frames are special
	for (carrier = K_min; carrier <= K_max; carrier ++) {
	   if (isAfsCell (4, carrier)) {
	      frame_1 [4 * nrCarriers + (carrier - K_min)] = false;
	   }
	}

	for (carrier  = K_min; carrier <= K_max; carrier ++) {
	   if (isAfsCell (39, carrier)) {
	      frame_4 [39 * nrCarriers + (carrier - K_min)] = false;
	   }
	}
	
	for (symbol = 0; symbol < 5; symbol ++) {
	   for (carrier = K_min; carrier <= K_max; carrier ++) {
	      if (isSDCcell (symbol, carrier))
	         frame_1 [symbol * nrCarriers + (carrier - K_min)] = false;
           }
        }

//	check on the number of cells in a superframe
	nrCells	= 0;
	for (symbol = 0; symbol < symbolsperFrame; symbol ++) 
	   for (carrier = K_min; carrier <= K_max; carrier ++) 
	      if (frame_1  [symbol * nrCarriers + (carrier - K_min)])
	         nrCells ++;
	start_frame_2	= nrCells;

	for (symbol = 0; symbol < symbolsperFrame; symbol ++)
	   for (carrier = K_min; carrier <= K_max; carrier ++)
	      if (frame_23 [symbol * nrCarriers + (carrier - K_min)])
	         nrCells ++;
	start_frame_3	= nrCells;

	for (symbol = 0; symbol < symbolsperFrame; symbol ++)
	   for (carrier = K_min; carrier <= K_max; carrier ++)
	      if (frame_23 [symbol * nrCarriers + (carrier - K_min)])
	         nrCells ++;
	start_frame_4	= nrCells;

	for (symbol = 0; symbol < symbolsperFrame; symbol ++)
	   for (carrier = K_min; carrier <= K_max; carrier ++)
	      if (frame_4 [symbol * nrCarriers + (carrier - K_min)])
	         nrCells ++;

	this	-> muxLength	= nrCells / 4;
	fprintf (stderr, "muxLength = %d\n",
	                            muxLength);
	params	-> muxLength		= muxLength;
	this	-> muxsampleBuf		= new theSignal [muxLength];
	this	-> my_deInterleaver	=
	                     new deInterleaver_long (muxLength, 6);
	bufferP				= 0;
	my_mscHandler			= nullptr;
	connect (this, SIGNAL (show_const ()),
	         theParent, SLOT (show_iq ()));
}

	mscProcessor::~mscProcessor	() {
	delete muxsampleBuf;
	if (my_mscHandler != nullptr)	
	   delete my_mscHandler;
	delete my_deInterleaver;
}

void	mscProcessor::processFrame (theSignal ** outbank) {
int symbol, carrier;
static	bool toggler = false;

	switch (params -> theChannel. Identity) {
	   default:
	   case 0:
	   case 3:
	      bufferP	= 0;
	      toggler	= false;
	      for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	         for (carrier = K_min; carrier <= K_max; carrier ++) {
	            if (frame_1 [symbol * nrCarriers + (carrier - K_min)]) {
	               muxsampleBuf [bufferP ++] =
	                         outbank [symbol][carrier - K_min];
	               if (bufferP >= muxLength) {
	                  process_mux (muxsampleBuf, toggler);
	                  toggler = !toggler;
	                  bufferP = 0;
	               }
	            }
	         }
	      }
	      break;

	   case 1:
	      for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	         for (carrier = K_min; carrier <= K_max; carrier ++) {
	            if (frame_23 [symbol * nrCarriers + (carrier - K_min)]) {
	               muxsampleBuf [bufferP ++] =
	                              outbank [symbol][carrier - K_min];
	               if (bufferP >= muxLength) {
	                  process_mux (muxsampleBuf, toggler);
                          toggler = !toggler;
	                  muxCounter ++;
	                  bufferP = 0;
	               }
	            }
	         }
	      }
	      break;

	   case 2:
	      for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	         for (carrier = K_min; carrier <= K_max; carrier ++) {
	            if (frame_4 [symbol * nrCarriers + (carrier - K_min)]) {
	               muxsampleBuf [bufferP ++] =
	                              outbank [symbol][carrier - K_min];
	               if (bufferP >= muxLength) {
	                  process_mux (muxsampleBuf, toggler);
	                  toggler = !toggler;
	                  muxCounter ++;
	                  bufferP = 0;
	                }
	            }
	         }
	      }
	      break;
	}
}

void	mscProcessor::selectService	(int shortId) {
	locker. lock ();
	if (my_mscHandler != nullptr)
	   delete my_mscHandler;

	if (the_postProcessor != nullptr)
	   delete the_postProcessor;
	if (params -> subChannels [shortId]. is_audioService)
	   the_postProcessor	= new audioFrameProcessor (theParent,
	                                             params,
	                                             shortId);
	else
	   the_postProcessor	= new dataProcessor (theParent,
	                                             params,
	                                             shortId);
	if (params -> theChannel. MSC_Mode == 0)
	   my_mscHandler	= new qam16_handler (params,
	                                             the_postProcessor, 
	                                             muxLength, shortId);
	else
	   my_mscHandler	= new qam4_handler (params,
	                                            the_postProcessor,
	                                            muxLength, shortId);
	locker. unlock ();
}

static int teller	= 0;
void	mscProcessor::process_mux (theSignal *mux, bool toggler) {
theSignal muxsampleBuf [muxLength];

	if (show_Constellation && (++teller >= 10)) {
	   for (int i = 0; i < 128; i ++) {
	       std::complex<float> temp =
	                   mux [i]. signalValue;
	       iqBuffer -> putDataIntoBuffer (&temp, 1);
	   }
	   show_const ();
	   teller = 0;
	}

	if (my_mscHandler == nullptr)
	   return;
	my_deInterleaver -> deInterleave (mux, muxsampleBuf);
	locker. lock ();
	my_mscHandler -> process (muxsampleBuf, toggler);
	locker. unlock ();
}

void	mscProcessor::set_Viewer (bool b) {
	show_Constellation	= b;
}

