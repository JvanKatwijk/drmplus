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
 *    along with DRM+ Decoder; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#include	"data-processor.h"
#include	"drm-decoder.h"
#include	"basics.h"
#include	<stdio.h>
#include	"fec-handler.h"
#include	<float.h>
#include	<math.h>
#include	"packet-assembler.h"

static	inline
uint16_t	get_MSCBits (uint8_t *v, int16_t offset, int16_t nr) {
int16_t		i;
uint16_t	res	= 0;

	for (i = 0; i < nr; i ++) 
	   res = (res << 1) | (v [offset + i] & 01);

	return res;
}
//
static	
uint16_t crc16_bytewise (uint8_t in [], int32_t N) {
int32_t i;
int16_t j;
uint32_t b = 0xFFFF;
uint32_t x = 0x1021;	/* (1) 0001000000100001 */
uint32_t y;

	for (i = 0; i < N - 2; i++) {
	   for (j = 7; j >= 0; j--) {
	      y = ((b >> 15) + ((uint32_t) (in [i] >> j) & 0x01)) & 0x01;
	      if (y == 1)
	         b = ((b << 1) ^ x);
	      else
	         b = (b << 1);
	   }
	}
	for (i = N - 2; i < N; i++) {
	   for (j = 7; j >= 0; j--) {
	      y = (((b >> 15) + ((uint32_t) ((in[i] >> j) & 0x01))) ^ 0x01) & 0x01;	/* extra parent pa0mbo */
	      if (y == 1)
	         b = ((b << 1) ^ x);
	      else
	         b = (b << 1);
	   }
	}
	return (b & 0xFFFF);
}

	dataProcessor::dataProcessor	(drmDecoder *drm,
	                                 drmParameters *params,
	                                 int stream) {
	this	-> drmMaster	= drm;
	this	-> params	= params;
	uint16_t applicationId	= params -> theStreams [stream].
	                                                      applicationId;
	my_packetAssembler	= new packetAssembler (params,
	                                                       drmMaster,
	                                                       applicationId);
	my_fecHandler		= new fecHandler (my_packetAssembler);
}

	dataProcessor::~dataProcessor	(void) {
	delete my_fecHandler;
	delete	my_packetAssembler;
}

void	dataProcessor::process         (uint8_t *buf_1, uint8_t *buf_2,
                                                         int stream) {
int     startPosA       = 0;
int     startPosB       = 0;

//	we first compute the length of the HP part, which - obviously -
//	is the start of the LP part
        for (int i = 0; i < 4; i ++)
           if (params -> theStreams [i]. inUse)
              startPosB += params -> theStreams [i]. lengthHigh;

//      Note that length and startPos are in bytes, input not yet packed though
        for (int i = 0; i < 4; i ++) {
           if (params -> theStreams [i]. inUse) {
              int lengthA = params -> theStreams [i]. lengthHigh;
              int lengthB = params -> theStreams [i]. lengthLow;
              if (stream == i)  {       // build up the logical frame
//	         fprintf (stderr, "stream %d %d %d\n", 
//	                                i, startPosB, lengthB);
                 uint8_t dataVec [2 * 8 * (lengthA + lengthB)];
                 memcpy (dataVec, &buf_1 [startPosA * 8], lengthA * 8);
                 memcpy (&dataVec [lengthA * 8],
                                  &buf_2 [startPosA * 8], lengthA * 8);
                 memcpy (&dataVec [2 * lengthA * 8],
                                  &buf_1 [startPosB * 8], lengthB * 8);
                 memcpy (&dataVec [2 * lengthA * 8 + lengthB * 8],
                                  &buf_2 [startPosB * 8], lengthB * 8);
//	just a reminder:
//	if the packetmode indicator == 1 we have an asynchronous stream
//	if 0, we have a synchronous stream which we don't handle yet
	         if (params -> theStreams [i]. packetModeInd == 1) {
	            process_packets (dataVec, i,
	                             0,	lengthA,
	                             lengthA, lengthB);
	         }
	         else {	// synchronous stream
	            process_syncStream (dataVec, i,
	                                0,	lengthA,
	                                lengthA, lengthB);
	         }
	      }
	      startPosA	+= lengthA;
	      startPosB	+= lengthB;
	   }
	}
}
//

void	dataProcessor::process_packets (uint8_t *v, int16_t mscIndex,
	                                int16_t startHigh, int16_t lengthHigh,
	                                int16_t startLow,  int16_t lengthLow) {
	if (lengthHigh != 0) 
	   handle_uep_packets (v, mscIndex, startHigh, lengthHigh,
	                            startLow, lengthLow);
	else
	   handle_eep_packets (v, mscIndex, startLow, lengthLow);
}

void	dataProcessor::process_syncStream (uint8_t *v, int16_t mscIndex,
	                                   int16_t startHigh,
	                                   int16_t lengthHigh,
	                                   int16_t startLow,
	                                   int16_t lengthLow) {
	if (lengthHigh != 0) 
	   handle_uep_syncStream (v, mscIndex, startHigh, lengthHigh,
	                            startLow, lengthLow);
	else
	   handle_eep_syncStream (v, mscIndex, startLow, lengthLow);
}

void	dataProcessor::handle_uep_syncStream (uint8_t *v, int16_t mscIndex,
	                           int16_t startHigh, int16_t lengthHigh,
	                           int16_t startLow, int16_t lengthLow) {
int16_t	i;
uint8_t dataBuffer [lengthLow + lengthHigh];
	
	for (i = 0; i < lengthHigh; i ++)
	   dataBuffer [i] = get_MSCBits (v, (startHigh + i) * 8, 8);
	for (i = 0; i < lengthLow; i ++)
	   dataBuffer [lengthHigh + i] =
	                    get_MSCBits (v, (startLow + i) * 8, 8);
	handle_syncStream (dataBuffer, lengthLow + lengthHigh, mscIndex);
}

void	dataProcessor::handle_eep_syncStream (uint8_t *v, int16_t mscIndex,
	                           int16_t startLow, int16_t lengthLow) {
uint8_t	dataBuffer [lengthLow];
int16_t i;

	for (i = 0; i < lengthLow; i ++)
	   dataBuffer [i] = get_MSCBits (v, (startLow + i) * 8, 8);
	handle_syncStream (dataBuffer, lengthLow, mscIndex);
}
//

void	dataProcessor::handle_syncStream (uint8_t *dataBuffer,
	                                  int16_t length,
	                                  int16_t index) {
//	for (int i = 0; i < 40; i ++)
//	   fprintf (stderr, "%x ", dataBuffer [i]);
//	fprintf (stderr, "\n");
}


void	dataProcessor::handle_uep_packets (uint8_t *v, int16_t mscIndex,
	                           int16_t startHigh, int16_t lengthHigh,
	                           int16_t startLow, int16_t lengthLow) {
int16_t	i;
uint8_t dataBuffer [lengthLow + lengthHigh];
	
	for (i = 0; i < lengthHigh; i ++)
	   dataBuffer [i] = get_MSCBits (v, (startHigh + i) * 8, 8);
	for (i = 0; i < lengthLow; i ++)
	   dataBuffer [lengthHigh + i] =
	                    get_MSCBits (v, (startLow + i) * 8, 8);
	if (params -> theStreams [mscIndex]. FEC)
	   handle_packets_with_FEC (dataBuffer,
	                            lengthLow + lengthHigh, mscIndex);
	else
	   handle_packets (dataBuffer, lengthLow + lengthHigh, mscIndex);
}

void	dataProcessor::handle_eep_packets (uint8_t *v, int16_t mscIndex,
	                                  int16_t startLow, int16_t lengthLow) {
uint8_t	dataBuffer [lengthLow];
int16_t i;

	for (i = 0; i < lengthLow; i ++)
	   dataBuffer [i] = get_MSCBits (v, (startLow + i) * 8, 8);
	if (params -> theStreams [mscIndex]. FEC)
	   handle_packets_with_FEC (dataBuffer, lengthLow, mscIndex);
	else
	   handle_packets (dataBuffer, lengthLow, mscIndex);
}

void	dataProcessor::handle_packets_with_FEC (uint8_t *v,
	                                        int16_t length,
	                                        uint8_t mscIndex) {
uint8_t *packetBuffer;
int16_t	packetLength = params -> theStreams [mscIndex]. packetLength + 3;
int16_t	i;
static	int	cnt	= 0;

//	first check the RS decoder
	my_fecHandler -> checkParameters (
	          params -> theStreams [mscIndex]. R,
	          params -> theStreams [mscIndex]. C, 
	          params -> theStreams [mscIndex]. packetLength + 3,
	          mscIndex);

	for (i = 0; i < length / packetLength; i ++) {
	   packetBuffer = &v [i * packetLength];
//	Fetch relevant info from the stream

//	packetBuffer [0] contains the header
	   uint8_t header	= packetBuffer [0];
//	   uint8_t firstBit	= (header & 0x80) >> 7;
//	   uint8_t lastBit	= (header & 0x40) >> 6;
	   uint8_t packetId	= (header & 0x30) >> 4;
	   uint8_t PPI		= (header & 0x8) >> 3;
//	   uint8_t CI		= header & 0x7;
//
	   if ((packetId == 3) && (PPI == 0)) {
	      my_fecHandler -> fec_packet (packetBuffer, packetLength);
	      cnt = 0;
	   }
	   else 
//	   if (packetId != 0) 
//	      if (PPI)
//	         fprintf (stderr, "filler with %d\n", packetBuffer [1]);
//	      else
//	      fprintf (stderr, "rommelpacket ertussen PPI = %d, CI = %d\n",
//	                               PPI, CI);
	   if (packetId == 0) {
	      my_fecHandler -> data_packet (packetBuffer, packetLength);
	      cnt ++;
	   }
	}
}

void	dataProcessor::handle_packets (uint8_t *v, int16_t length,
	                               uint8_t mscIndex) {
uint8_t *packetBuffer;
int16_t	packetLength = params -> theStreams [mscIndex]. packetLength + 3;
int16_t	i;

	for (i = 0; i < length / packetLength; i ++) {
	   packetBuffer = &v [i * packetLength];
//	Fetch relevant info from the stream
//
//	first a crc check
	   if (crc16_bytewise (packetBuffer, packetLength) != 0)
	      continue;
	   my_packetAssembler -> assemble (packetBuffer,
	                                   packetLength, mscIndex);
	}
}

