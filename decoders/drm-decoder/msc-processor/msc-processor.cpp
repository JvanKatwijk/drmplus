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
//#include	"dataframe-processor.h"

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
	my_audioFrameProcessor	= nullptr;
//	my_dataFrameProcessor	= nullptr;
	my_deconvolver		= nullptr;
	serviceSelected. store (-1);

	for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	   for (carrier = K_min; carrier <= K_max; carrier ++) {
	      if (isFACcell (symbol, carrier)) {
	         frame_1  [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_4  [symbol * nrCarriers + (carrier - K_min)] = false;
	      }
	      else
	      if (isGainCell (symbol, carrier)) {
	         frame_1  [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = false;
	         frame_4  [symbol * nrCarriers + (carrier - K_min)] = false;
	      }
	      else {
	         frame_1  [symbol * nrCarriers + (carrier - K_min)] = true;
	         frame_23 [symbol * nrCarriers + (carrier - K_min)] = true;
	         frame_4  [symbol * nrCarriers + (carrier - K_min)] = true;
	      }
	   }
	}
//
//	the timecells are only in symbol 0 in all frame types
	for (carrier = K_min; carrier <= K_max; carrier ++) {
	   if (isTimeCell (carrier)) {
	      frame_1  [carrier - K_min] = false;
	      frame_23 [carrier - K_min] = false;
	      frame_4  [carrier - K_min] = false;
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
//	fprintf (stderr, "muxLength = %d\n", muxLength);
	params	-> muxLength		= muxLength;
	this	-> muxsampleBuf		= new theSignal [muxLength];
	this	-> my_deInterleaver	=
	                     new deInterleaver_long (muxLength, 6);
	connect (this, SIGNAL (show_const ()),
	         theParent, SLOT (show_iq ()));
	bufferP				= 0;
	toggleFlag			= 0;
}

	mscProcessor::~mscProcessor	() {
	delete muxsampleBuf;
	delete my_deInterleaver;
}
//
//
//	It is known that the frames 1 and 4 have less datacells, a few
//	cells are taken by the Afs cells. So, therefore we
//	can decided that no frame with identify 0 is complete,
//	the end will be signalled in a frame type 1.
//	In a sequence {0, 3} 1.. 1 2 the number of data frames is 4,
//	the toggleFlag will tell whether the datablock, when complete
//	is a "left" or a "right" block
void	mscProcessor::processFrame (theSignal ** outbank) {
int symbol, carrier;

//	fprintf (stderr, "new frame %d (block %d)\n",
//	                 params -> theChannel. toggleFlag,
//	                 params -> theChannel. Identity);
	switch (params -> theChannel. Identity) {
	   default:	// does not happen
	   case 0:
	   case 3:
	      bufferP	= 0;
	      toggleFlag = params -> theChannel. toggleFlag;
	      for (symbol = 0; symbol < symbolsperFrame; symbol ++) {
	         for (carrier = K_min; carrier <= K_max; carrier ++) {
	            if (frame_1 [symbol * nrCarriers + (carrier - K_min)]) {
	               muxsampleBuf [bufferP ++] =
	                         outbank [symbol][carrier - K_min];
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
	                  process_mux (muxsampleBuf, toggleFlag);
	      	          toggleFlag = params -> theChannel. toggleFlag;
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
	                  process_mux (muxsampleBuf, toggleFlag);
	      	          toggleFlag = params -> theChannel. toggleFlag;
	                  bufferP = 0;
	                  muxCounter ++;
	                }
	            }
	         }
	      }
//	      fprintf (stderr, "bufferP = %d, muxLength = %d\n",
//	                      bufferP, muxLength);
	      break;
	}
}

void	mscProcessor::selectService	(int shortId) {
int	streamId	= -1;
int	lengthA, lengthB;
#if 1
	for (int i = 0; i < 4; i ++) 
	   fprintf (stderr, "stream %d -> shortId %d (inUse %d)\n",
	                      i, 
	                      params -> theStreams [i]. shortId,	
	                      params -> theStreams [i]. inUse);
#endif
	serviceSelected. store (-1);
	locker. lock ();
	if (my_audioFrameProcessor != nullptr)
	   delete my_audioFrameProcessor;
//	if (my_dataFrameProcessor != nullptr)
//	   delete my_dataFrameProcessor;
	my_audioFrameProcessor	= nullptr;
//	my_dataFrameProcessor	= nullptr;
	if (my_deconvolver != nullptr)
	   delete my_deconvolver;
	my_deconvolver		= nullptr;

	lengthA		= 0;
	lengthB		= 0;
	for (int i = 0; i < 4; i ++) {
	   if (params -> theStreams [i]. inUse) {
	      lengthA += params -> theStreams [i]. lengthHigh * 8;
	      lengthB += params -> theStreams [i]. lengthLow  * 8;
	   }
	}

	muxBuffer. resize (2 * lengthA + 2 * lengthB);

	if (params	-> theChannel. MSC_Mode == 0)	// QAM16
	   my_deconvolver	= new qam16_handler (params, muxLength,
	                                             lengthA, lengthB);
	else
	   my_deconvolver	= new qam4_handler (params, muxLength,
	                                            lengthA, lengthB);

//	OK, for now we only deal with the - more or less - primary
//	data for the different services. I.e. an audio service will
//	only provide audio,

	if (params -> subChannels [shortId]. is_audioService) {
	   for (int i = 0; i < 4; i ++) {
	      if (params -> theStreams [i]. inUse &&
	          params -> theStreams [i]. audioStream &&
	          (params -> theStreams [i]. shortId == shortId)) {
	         streamId	= i;
	         my_audioFrameProcessor	= new audioFrameProcessor (theParent,
	                                                           params,
	                                                           shortId,
	                                                           i);
	         fprintf (stderr, "selected streamId %d\n", i);
	         break;
	      }
	   }
	}
	else {		// there is a data service
//	   for (int i = 0; i < 4; i ++) {
//	      if (params -> theStreams [i]. inUse &&
//	          !params -> theStreams [i]. audioStream &&
//	          (params -> theStreams [i]. shortId == shortId)) {
//	         streamId	= i;
//	         my_dataFrameProcessor	= new dataFrameProcessor (theParent,
//	                                                          params,
//	                                                          shortId,
//	                                                          i);
//	         fprintf (stderr, "shortId %d carrier data on stream %d\n",
//	                              shortId, i);
//	         break;
//	      }
//	   }
	}
	locker. unlock ();
	if (streamId == -1)
	   return;
	else
	   serviceSelected. store (shortId);
}

static int teller	= 0;
void	mscProcessor::process_mux (theSignal *mux, int frameNumber) {
//void	mscProcessor::process_mux (theSignal *mux, bool toggler) {
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

//	fprintf (stderr, "processing mux %d\n", frameNumber);
	if (serviceSelected. load () == -1)
	   return;
//
	locker. lock ();
	my_deInterleaver -> deInterleave (mux, muxsampleBuf);
	my_deconvolver	-> process (muxsampleBuf, muxBuffer. data ());

	if (my_audioFrameProcessor != nullptr)
	   my_audioFrameProcessor -> process (muxBuffer. data (), frameNumber);
//	   my_audioFrameProcessor -> process (muxBuffer. data (), !toggler);
//	if (my_dataFrameProcessor != nullptr) 
//	   my_dataFrameProcessor -> process (muxBuffer. data (), !toggler);

	locker. unlock ();
}

void	mscProcessor::set_Viewer (bool b) {
	show_Constellation	= b;
}

