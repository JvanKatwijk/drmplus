#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of DRM+-receiver
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
#include	"sdc-processor.h"
#include	"drm-decoder.h"
#include	"referenceframe.h"
#include	<cstdlib>
//
//
static
const uint16_t crcPolynome [] = {
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 // MSB .. LSB x16 + x12 + x5 + 1
};

static  inline
uint32_t        get_SDCBits (uint8_t *v, int16_t offset, int16_t size) {
int16_t         i;
uint32_t        res     = 0;

        for (i = 0; i < size; i ++)
           res = (res << 1) | v [offset + i];

        return res;
}

static
void    MjdToDate (int Mjd, int *Year, int *Month, int *Day);
//
	sdcProcessor::sdcProcessor (drmDecoder *parent, 
	                            drmParameters *params,
	                            RingBuffer<std::complex<float>> *b):
	                              my_qam4_metrics (),
	                              theCRC (16, crcPolynome) {
int	symbol, carrier;
	this	-> theParent	= parent;
	this	-> params	= params;
	this	-> iqBuffer	= b;
	show_Constellation	= false;

	connect (this, SIGNAL (show_iq ()),
	         theParent, SLOT (show_iq ()));
	connect (this, SIGNAL (show_mer (float)),
	         theParent, SLOT (show_mer (float)));
	nr_sdcSamples		= init_sdcTable ();

	Y21Mapper	= new Mapper (2 * nr_sdcSamples, 21);
	handler_12	= new sdcHandler (params, 1, 2, nr_sdcSamples);
	handler_14	= new sdcHandler (params, 1, 4, nr_sdcSamples);
	prbs_12		= new prbs (handler_12 -> get_sdcBits ());
	prbs_14		= new prbs (handler_14 -> get_sdcBits ());
	for (int i = 0; i < 4; i ++) {
	   params -> theStreams [i]. inUse = false;
	   params -> theStreams [i]. lengthHigh	= 0;
	   params -> theStreams [i]. lengthLow	= 0;
	}
}

	sdcProcessor::~sdcProcessor	() {
	delete	Y21Mapper;
	delete	prbs_12;
	delete	prbs_14;
	delete	handler_12;
	delete	handler_14;
}
static int teller	= 0;

bool	sdcProcessor::processSDC	(theSignal **theData) {
metrics	rawBits 	[2 * nr_sdcSamples];
metrics demappedBits	[2 * nr_sdcSamples];
std::complex<float> buffer [nr_sdcSamples];
int	nr_sdcBits;
int	i;

//	first: read in the samples
	for (i = 0; i < nr_sdcSamples; i ++) {
	   int16_t symbol, carrier;
	   getSDCcell (i, &symbol, &carrier);
	   sdcCells [i] = theData [symbol][carrier - K_min];
	   buffer [i] = sdcCells [i]. signalValue;
	}

	if (show_Constellation) {
	   iqBuffer -> putDataIntoBuffer (buffer, nr_sdcSamples);
	   show_iq ();
	}

	if (++teller > 5) {
	   show_mer (my_qam4_metrics. compute_Mer (sdcCells, nr_sdcSamples));
	   teller = 0;
	}

//	second: map them to soft bits
        my_qam4_metrics. computemetrics (sdcCells,
	                                   nr_sdcSamples, rawBits);

//	interleave the soft bits
	for (i = 0; i < 2 * nr_sdcSamples; i ++) 
	   demappedBits [Y21Mapper -> mapIn (i)] = rawBits [i];

//	select and apply the deconvolver
	if (params -> theChannel. sdcMode == 1) { // protection 1/4
	   nr_sdcBits	= 4 + 16 + 8 * 55;
	   handler_14	-> process (demappedBits, &sdcBits [4]);
	   prbs_14	-> doPRBS (&sdcBits [4]);
	}
	else {	// protection 1/2
	   nr_sdcBits	= 4 + 16 + 8 * 113;
	   handler_12	-> process (demappedBits, &sdcBits [4]);
	   prbs_12	-> doPRBS (&sdcBits [4]);
	}

	for (int i = 0; i < 4; i ++)
	   sdcBits [i] = 0;

	if (!theCRC. doCRC (sdcBits, 4 + nr_sdcBits)) 
	   return false;

	extractData (&sdcBits [8], nr_sdcBits - 16 - 4);
	return true;
}

void	sdcProcessor::extractData (uint8_t *data, int size) {
int	index	= 0;
	while (index < size) 
	   index = sdcField (data, index);
}

int	sdcProcessor::sdcField (uint8_t *data, int index) {
int	bodySize	= get_SDCBits (data, index, 7);
int	entity		= get_SDCBits (data, index + 8, 4);
int	shortId, streamId;
int	base		= index + 12;
QString	languageCode;
QString	countryCode;
QString	s;
int	lengthHigh	= 0, lengthLow	= 0;
int	temp1		= 0, temp2	= 0;

//	fprintf (stderr, "sdcField key %d, length %d\n",
//	                 entity, bodySize);

	switch (entity) {
	   case 0:		// multiplex description
	      if (bodySize < 2)
	         break;
	      params	-> protLevelA	= get_SDCBits (data, base, 2);
	      params	-> protLevelB	= get_SDCBits (data, base + 2, 2);
	      for (int i = 0; i < bodySize / 3; i ++) {
	         int index = base + 4 + i * 24;
	         params -> theStreams [i]. lengthHigh =
	                        get_SDCBits (data, index, 12);
	         params -> theStreams [i]. lengthLow =
	                       get_SDCBits (data, index + 12, 12);
	         lengthHigh	+= get_SDCBits (data, index, 12);
	         lengthLow	+= get_SDCBits (data, index + 12, 12);
#if 0
	         fprintf (stderr, "stream %d prots (%d %d) lengthHigh %d  Low %d\n",
	                    i, params -> protLevelA, params -> protLevelB,
	                    get_SDCBits (data, index, 12),
	                    get_SDCBits (data, index + 12, 12));
#endif
//	         params -> theStreams [i]. inUse = true;
	      }
	      temp1	= 0; temp2	= lengthHigh;
	      for (int i = 0; i < bodySize / 3; i ++) {
	         params -> theStreams [i]. offsetHigh	=  temp1;
	         temp1 += params -> theStreams [i]. lengthHigh;
	         params -> theStreams [i]. offsetLow	= temp2;
	         temp2 += params -> theStreams [i]. lengthLow;
	      }
	      return index + 16 + 8 * bodySize;

	   case 5:
	      shortId	= get_SDCBits (data, base, 2);
	      streamId	= get_SDCBits (data, base + 2, 2);
	      params	-> theStreams [streamId]. inUse	= true;
	      params	-> theStreams [streamId]. shortId = shortId;
	      params 	-> theStreams [streamId]. audioStream = false;
	      params	-> theStreams [streamId]. packetModeInd =
	                                get_SDCBits (data, base + 4, 1);
	      if (params -> theStreams [streamId]. packetModeInd == 0) {
//	synchronous stream
	         params -> theStreams [streamId]. enhancementFlag =
	                                get_SDCBits (data, base + 8, 1);
	         params -> theStreams [streamId]. domain =
	                                get_SDCBits (data, base + 9, 3);
	      }
	      else {
//	packet mode
	         params -> theStreams [streamId]. dataUnitIndicator =
                                         get_SDCBits (data, base + 5, 1);
	         params -> theStreams [streamId]. packetId =
	                                 get_SDCBits (data, base + 6, 2);
	         params -> theStreams [streamId]. enhancementFlag =
	                                 get_SDCBits (data, base + 8, 1);
	         params -> theStreams [streamId]. domain =
	                                 get_SDCBits (data, base + 9, 3);
	         params -> theStreams [streamId]. packetLength =
	                                 get_SDCBits (data, base + 12, 8);
	         if (params -> theStreams [streamId]. domain == 0)
	            params -> theStreams [streamId]. applicationId =
	                                 get_SDCBits (data, base + 20, 16);
	         else
	         if (params -> theStreams [streamId]. domain == 1)
	            params -> theStreams [streamId]. applicationId =
	                                 get_SDCBits (data, base + 25, 11);
	      }
	      return index + 16 + 8 * bodySize;

	   case 3:
	   case 4:
	   case 11:
	   case 13:	// all to do with afs
	         return index + 16 + 8 * bodySize;
	       
	   default:
//	         fprintf (stderr, "entity %d not supported yet\n", entity);
	         return index + 16 + 8 * bodySize;
	                
	   case 1:	// label entity
	      shortId = get_SDCBits (data, base, 2);
	      params -> subChannels [shortId]. inUse = true;
	      {
	         char temp [bodySize + 1];
	         for (int i = 0; i < bodySize; i ++)
	            temp [i] = get_SDCBits (data, base + 4 + 8 * i, 8);
	         temp [bodySize] = 0;
	         if (bodySize > 1) {
	            params -> subChannels [shortId]. serviceName = 
	                                QString::fromUtf8 (temp, bodySize);
	         }
	      }
	      return index + 16 + 8 * bodySize;

	   case 12:	// language and country data 
	      shortId	= get_SDCBits (data, base, 2);
	      for (int i = 0; i < 3; i ++)
	         languageCode.
	               append (get_SDCBits (data, base + 4 + 8 * i, 8));
	      for (int i = 0; i < 2; i ++)
	         countryCode.
	               append (get_SDCBits (data, base + 28 + 8 * i, 8));
	      params	-> subChannels [shortId]. languagetxt	= languageCode;
	      params	-> subChannels [shortId]. country	= countryCode;
	      return index + 16 + 8 * bodySize;

	   case 8:	// time and date
	      if (bodySize < 3)
	         break;
	      {  uint32_t mjd	= get_SDCBits (data, base, 17);
	         uint16_t utc	= get_SDCBits (data, base + 17, 11);
	         uint8_t     ltOffsets= 0;
	         int      ltOffsetv = 0;
	         if (bodySize > 28) {
	            ltOffsets	= get_SDCBits (data, base + 30, 1);
	            ltOffsetv	= get_SDCBits (data, base + 31, 5);
	         }
	         int offset	= ltOffsets ? ltOffsetv * -30 : ltOffsetv * 30;
	         int Year, Month, Day;
	         MjdToDate (mjd, &Year, &Month, &Day);
	         int hours = (utc >> 6) & 0x1F;
	         int minutes = (utc & 0x3F) + offset;
	         while (minutes >= 60) {
	            hours = (hours + 1) % 24;
	            minutes -= 60;	
	         }
	         while (minutes < 0) {
	            hours = (hours - 1 + 24) % 24;
	            minutes += 60;
	         }
	         params -> Year		= Year;
	         params	-> Month	= Month;
	         params -> Day		= Day;
	         params -> hours	= hours;
	         params -> minutes	= minutes;
	      }
	      return index + 16 + 8 * bodySize;
//
//	for now: 
//	if the service is defined as data we do not know
//	how to handle audio in the associated stream
	   case 9:	// audio information
	      shortId	= get_SDCBits (data, base, 2);
	      streamId	= get_SDCBits (data, base + 2, 2);
	      if (!params -> subChannels [shortId]. is_audioService) {
	         params -> theStreams [streamId]. inUse = false;
	         return index + 16 + 8 * bodySize;
	      }
	      params	-> theStreams [streamId]. inUse	= true;
	      params	-> theStreams [streamId]. shortId = shortId;
	      params	-> theStreams [streamId]. audioStream = true;
	      params	-> theStreams [streamId]. audioCoding =
	                               get_SDCBits (data, base + 4, 2);
	      params	-> theStreams [streamId]. sbrFlag =
	                               get_SDCBits (data, base + 6, 1);
	      params	-> theStreams [streamId]. audioMode =
	                               get_SDCBits (data, base + 7, 2);
	      params	-> theStreams [streamId]. audioSamplingRate =
	                               get_SDCBits (data, base + 9, 3);
	      params	-> theStreams [streamId]. textFlag =
	                               get_SDCBits (data, base + 12, 1);
	      params	-> theStreams [streamId]. enhancementFlag =
	                               get_SDCBits (data, base + 13, 1);
	      params	-> theStreams [streamId]. coderField =
	                               get_SDCBits (data, base + 14, 5);
	      (void) get_SDCBits (data, base + 19, 1);		// rfa
//
//	if xHE-AAC we need to collect the decoderspecific data
	      if (params -> theStreams [streamId]. audioCoding == 03) {
	         int bytes = (index + 16 + 8 * bodySize - (base + 20)) / 8;
	         params -> theStreams [streamId]. xHE_AAC. resize (0);
	         for (int i = 0; i < bytes; i ++) {
	            uint8_t t = get_SDCBits (data, base + 20 + 8 * i, 8);
	            params -> theStreams [streamId]. xHE_AAC.  push_back (t);
	         }
	      }
	      return index + 16 + 8 * bodySize;

	   case 14:	//packet stream FEC parameters
	      streamId	= get_SDCBits (data, base + 2, 2);
	      params	-> theStreams [streamId]. R =
	                                get_SDCBits (data, base + 4, 8);
	      params	-> theStreams [streamId]. C =
	                                get_SDCBits (data, base + 12, 8);
	      params	-> theStreams [streamId]. packetLength =
	                                get_SDCBits (data, base + 20, 8);
	      params	-> theStreams [streamId]. FEC	= true;
	}
	return index + 12;
}


static
void	MjdToDate (int Mjd, int *Year, int *Month, int *Day) {
long J, C, Y, M;

	J = Mjd + 2400001 + 68569;
	C = 4 * J / 146097;
	J = J - (146097 * C + 3) / 4;
	Y = 4000 * (J + 1) / 1461001;
	J = J - 1461 * Y / 4 + 31;
	M = 80 * J / 2447;
	*Day = J - 2447 * M / 80;
	J = M / 11;
	*Month = M + 2 - (12 * J);
	*Year = 100 * (C - 49) + Y + J;
}

void	sdcProcessor::set_Viewer	(bool b) {
	show_Constellation = b;
}

