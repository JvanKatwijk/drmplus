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

#ifndef	__FAC_PROCESSOR__
#define	__FAC_PROCESSOR__

#include	<QObject>
#include	"basics.h"
#include	"drm-decoder.h"
#include        "mapper.h"
#include        "qam4-metrics.h"
#include        "prbs.h"
#include        "checkcrc.h"
#include        "viterbi-drm.h"
#include	"ringbuffer.h"

class	drmDecoder;
typedef	struct {
	int symbol;
	int carrier;
} facCell;

class	facProcessor: public QObject {
Q_OBJECT
public:
			facProcessor	(drmDecoder	*,
	                                 drmParameters	*,
	                                 RingBuffer<std::complex<float>> *);
			~facProcessor	();

	bool		processFAC	(theSignal **,
	                                 float, std::complex<float> **);
	void		set_Viewer	(bool b);
private:
	drmDecoder	*theRadio;
	drmParameters	*params;
	RingBuffer<std::complex<float>> *iqBuffer;
	bool		show_Constellation;
	Mapper          myMapper;
        prbs            thePRBS;
        checkCRC        theCRC;
        qam4_metrics    myMetrics;
        viterbi_drm     deconvolver;
        void            fromSamplestoBits (theSignal *, uint8_t *);
	void		channelData	  (uint8_t *);
	void		serviceData	  (uint8_t *);
signals:
	void		show_iq		();
	void		showSNR		(float);
};

#endif
