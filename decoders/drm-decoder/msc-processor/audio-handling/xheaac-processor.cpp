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

#include	"xheaac-processor.h"
#include	"basics.h"
#include	"drm-decoder.h"
#include	"rate-converter.h"
#include	<deque>
#include	<vector>
#include	<complex>

vector<uint8_t>  frameBuffer;
vector<uint32_t> borders;

static
const uint16_t crcPolynome [] = {
        0, 0, 0, 1, 1, 1, 0     // MSB .. LSB x⁸ + x⁴ + x³ + x² + 1
};

static
const uint16_t polynome_16 [] = {
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 // MSB .. LSB x16 + x12 + x5 + 1
};

//
static  inline
uint16_t        get_MSCBits (uint8_t *v, int16_t offset, int16_t nr) {
int16_t         i;
uint16_t        res     = 0;

        for (i = 0; i < nr; i ++)
           res = (res << 1) | (v [offset + i] & 01);

        return res;
}

//	the 16 bit CRC - computed over bytes - has 
//	as polynome x^16 + x^12 + x^5 + 1
static inline
bool	check_crc_bytes (uint8_t *msg, int32_t len) {
int i, j;
uint16_t	accumulator	= 0xFFFF;
uint16_t	crc;
uint16_t	genpoly		= 0x1021;

	for (i = 0; i < len; i ++) {
	   int16_t data = msg [i] << 8;
	   for (j = 8; j > 0; j --) {
	      if ((data ^ accumulator) & 0x8000)
	         accumulator = ((accumulator << 1) ^ genpoly) & 0xFFFF;
	      else
	         accumulator = (accumulator << 1) & 0xFFFF;
	      data = (data << 1) & 0xFFFF;
	   }
	}
//
//	ok, now check with the crc that is contained
//	in the au
	crc	= ~((msg [len] << 8) | msg [len + 1]) & 0xFFFF;
//	fprintf (stderr, "crc = %X, accu %X\n",
//	           (msg [len] << 8) | msg [len + 1],  ~accumulator & 0xFFFF);
	return (crc ^ accumulator) == 0;
}

	xheaacProcessor::xheaacProcessor (drmDecoder *drm,
	                                  drmParameters *params,
	                                  decoderBase *my_aacDecoder):
	                                    theCRC (8, crcPolynome),
	                                    CRC_16 (16, polynome_16) { 
	this	-> parent	= drm;
	this	-> params	= params;
	connect (this, SIGNAL (putSample (float, float)),
	         parent, SLOT (sampleOut (float, float)));
	connect (this, SIGNAL (faadSuccess (bool)),
	         parent, SLOT (faadSuccess (bool)));
	this	-> my_aacDecoder	= my_aacDecoder;
	this	-> theConverter		= nullptr;
	currentRate			= 0;
	frameBuffer. resize (0);
	borders. resize (0);
}

	xheaacProcessor::~xheaacProcessor	() {
	if (theConverter != nullptr)
	   delete theConverter;
}
//
//	actually, we know that lengthHigh == 0, and therefore
//	that startLow = 0 as well
void	xheaacProcessor::process_usac	(uint8_t *v, int16_t streamId,
                                         int16_t startHigh, int16_t lengthHigh,
                                         int16_t startLow, int16_t lengthLow) {
int	frameBorderCount	= get_MSCBits (v, 0, 4);
int	bitReservoirLevel	= get_MSCBits (v, 4, 4);
int	crc			= get_MSCBits (v, 8, 8);
int	length			= lengthHigh + lengthLow;
int	numChannels		=
	         params -> theStreams [streamId]. audioMode == 0 ? 1 : 2;
int	elementsUsed		= 0;

	if (!theCRC. doCRC (v, 16)) {
	   fprintf (stderr, "oei\n");
	}

	uint32_t bitResLevel	= (bitReservoirLevel + 1) * 384 *
	                                               numChannels;
//
//	we do not look at the USAC crc at the end of the USAC frame
	uint32_t directoryOffset = length - 2 * frameBorderCount - 2;

	if (frameBorderCount <= 0) 
	   return;

	borders. 	resize	(frameBorderCount);
	std::vector<uint8_t> audioDescriptor =
	                         getAudioInformation (params, streamId);
	my_aacDecoder -> reinit (audioDescriptor, streamId);

	for (int i = 0; i < frameBorderCount; i++) {
	   uint32_t frameBorderIndex =
	                    get_MSCBits (v, 8 * length - 16 - 16 * i, 12);
	   uint32_t frameBorderCountRepeat = 
	                    get_MSCBits (v, 8 * length - 16 - 16 * i + 12, 4);
#if 0
	   fprintf (stderr, "frameBorderIndex %d, check %d\n",
	                        frameBorderIndex, frameBorderCountRepeat);
#endif
	   if (frameBorderCountRepeat != frameBorderCount) {
	      resetBuffers ();
	      fprintf (stderr, "foute as\n");
	      return;
	   }
	   borders [i] = frameBorderIndex;
	}
	elementsUsed = 0;
//
//	The first frameBorderIndex might point to the last one or
//	two bytes of the previous afs.
//	fprintf (stderr, "borders [0] = %d,  borders ]%d] = %d, length = %d\n",
//	          borders [0], frameBorderCount, borders [frameBorderCount - 1],
//	                                   length);
	switch (borders [0]) {
	   case 0xffe: // delayed from previous afs
//	first frame has two bytes in previous afs
	      if (frameBuffer. size () < 2) {
	         resetBuffers ();
	         return;
	      }
//
//	if the "frameBuffer" contains more than 2 bytes, there was
//	a non-empty last part in the previous afs
	      if (frameBuffer. size () > 2) {
	         playOut (frameBuffer, frameBuffer. size (), -2);
	      }
	      break;

	   case 0xfff:
//	first frame has one byte in previous afs
	      if (frameBuffer. size () < 1) {
	         resetBuffers ();
	         return;
	      }

	      if (frameBuffer. size () > 1) {
	         playOut (frameBuffer, frameBuffer. size (), -1);
	      }
	      break;

	   default: // boundary in this afs
//	boundary in this afs, process the last part of the previous afs
//	together with what is here as audioFrame
	      if (borders [0] < 2) {
	         resetBuffers ();
	         return;
              }
//
//	elementsUsed will be used to keep track on the progress
//	in handling the elements of this afs
	      for (elementsUsed = 0;
	                elementsUsed < borders [0]; elementsUsed ++) {
	         frameBuffer.
	              push_back (get_MSCBits (v, 16 + elementsUsed * 8, 8));
	      }
//	      if (!check_crc_bytes (frameBuffer. data (), 
//	                              frameBuffer. size () - 2))
//	         fprintf (stderr, ">>>>>>>>>>>>>>>>>>>>Fail for 0 (%d, %d) out of %d\n",
//	                       borders [0], frameBuffer. size (),
//	                       frameBorderCount);
	      playOut (frameBuffer, frameBuffer. size (), 0);
	      break;
	}

	for (int i = 1; i < frameBorderCount; i ++) {
	   frameBuffer. resize (0);
	   int buff = borders [i] - elementsUsed;
	   for (; elementsUsed < borders [i]; elementsUsed ++) {
	      frameBuffer.
	               push_back (get_MSCBits (v, 16 + elementsUsed * 8, 8));
	   }

//	   if (check_crc_bytes (frameBuffer. data (), frameBuffer. size () - 2))
//	      fprintf (stderr, ">>>>>>>>>>>>>>>>>>>Fail for %d (%d %d) out of %d\n", i,
//	                                  borders [i], frameBuffer. size (),
//	                                      frameBorderCount);
	   playOut (frameBuffer, frameBuffer. size (), i);
	}

	frameBuffer. resize (0);
// at the end, save for the next afs
	for (int i = borders [frameBorderCount - 1];
	                        i < directoryOffset; i ++)
	   frameBuffer. push_back (get_MSCBits (v, 16 + i * 8, 8));
}
//
void	xheaacProcessor::resetBuffers	() {
	frameBuffer. resize (0);
}
//
static int good	= 0;
static int fout	= 0;
static
int16_t outBuffer [16 * 960];
void	xheaacProcessor::
	          playOut (std::vector<uint8_t> &f, int size, int index) {
static bool	convOK = false;
int16_t	cnt;
int32_t	rate;
	my_aacDecoder ->  decodeFrame (f. data (),
	                               f. size (),
//	                               f. size () - 2,
	                               &convOK,
	                               outBuffer,
	                               &cnt, &rate);
	if (convOK) {
	   faadSuccess (true);
	   writeOut (outBuffer, cnt, rate);
	   good ++;
	}
	else {
	   fout ++;
	   faadSuccess (false);
	}

	if (good + fout >= 10) {
//	   fprintf (stderr, "%d goed out of %d\n", good, good + fout);
	   good = 0;
	   fout = 0;
	}
}
//
void	xheaacProcessor::toOutput (std::complex<float> *b, int16_t cnt) {
int16_t i;
        if (cnt == 0)
           return;

        for (i = 0; i < cnt; i ++)
           putSample (real (b [i]), imag (b [i]));
}

void	xheaacProcessor::writeOut (int16_t *buffer, int16_t cnt,
	                           int32_t pcmRate) {
	if (theConverter == nullptr) {
	   theConverter = new rateConverter (pcmRate, 48000, pcmRate / 10);
	   fprintf (stderr, "converter set to %d\n", pcmRate);
	   currentRate	= pcmRate;
	}

	if (pcmRate != currentRate) {
	   delete theConverter;
	   theConverter = new rateConverter (pcmRate, 48000, pcmRate / 10);
	   currentRate = pcmRate;
	   fprintf (stderr, "converter set to %d\n", pcmRate);
	}
#if 0
	fprintf (stderr, "processing %d samples (rate %d)\n",
	                  cnt, pcmRate);
#endif
	std::complex<float> local [theConverter -> getOutputsize ()];
	for (int i = 0; i < cnt; i ++) {
	   std::complex<float> tmp = 
	                    std::complex<float> (buffer [2 * i] / 8192.0,
	                                         buffer [2 * i + 1] / 8192.0);
	   if (pcmRate == 48000)
	      toOutput (&tmp, 1);
	   else {
	      int amount;
	      bool b = theConverter -> convert (tmp, local, &amount);
	      if (b)
	         toOutput (local, amount);
	   }
	}
}

std::vector<uint8_t>
	xheaacProcessor::getAudioInformation (drmParameters *drm,
	                                                int streamId) {
std::vector<uint8_t> temp;
streamParameters *sp = &(drm -> theStreams [streamId]);
uint8_t	xxx	= 0;

	xxx	= sp -> audioCoding << 6;
	xxx	|= (sp -> sbrFlag << 5);
	xxx	|= (sp -> audioMode << 3);
	xxx	|= sp -> audioSamplingRate;
	temp. push_back (xxx);
	xxx	= 0;
	xxx	= sp -> textFlag << 7;
	xxx	|= (sp -> enhancementFlag << 6);
	xxx	|= (sp -> coderField) << 1;
	temp. push_back (xxx);
	for (int i = 0; i < sp -> xHE_AAC. size (); i ++)
	   temp. push_back (sp -> xHE_AAC. at (i));
	return temp;
}

