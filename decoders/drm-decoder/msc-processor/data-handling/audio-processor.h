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

#ifndef	__AUDIO_PROCESSOR__
#define	__AUDIO_PROCESSOR__

#include	"basics.h"
#include        <QObject>
#include        <cstring>
#include	"fir-filters.h"
#include        "drm-aacdecoder.h"
#include	"message-processor.h"
#include	"post-processor.h"

typedef struct frame {
	int16_t length, startPos;
	uint8_t aac_crc;
	uint8_t audio [512];
} audioFrame;

class	drmDecoder;

class	audioProcessor: public postProcessor {
Q_OBJECT
public:
			audioProcessor	(drmDecoder *, drmParameters *);
			~audioProcessor	();
	void		process		(uint8_t *, uint8_t *, int);
private:
	drmDecoder	*parent;
	drmParameters	*params;
	messageProcessor my_messageProcessor;
	DRM_aacDecoder	my_aacDecoder;
	LowPassFIR      upFilter_24000;
	int		numFrames;
	void		processAudio	(uint8_t *, int16_t,
	                                 int16_t, int16_t, int16_t, int16_t);
	void		process_aac     (uint8_t *, int16_t, int16_t,
	                                 int16_t, int16_t, int16_t);
	void		handle_uep_audio (uint8_t *, int16_t,
	                                 int16_t, int16_t, int16_t, int16_t);
	void		handle_eep_audio (uint8_t *, int16_t,
	                                          int16_t, int16_t);
	void		process_usac	(uint8_t *v, int16_t mscIndex,
                                         int16_t startHigh, int16_t lengthHigh,
                                         int16_t startLow, int16_t lengthLow);

	void		writeOut	(int16_t *, int16_t, int32_t);
	void		toOutput	(float *, int16_t);
	void		playOut		(int16_t);
signals:
	void            show_audioMode  (QString);
	void            putSample       (float, float);
	void            faadSuccess     (bool);
};

#endif

