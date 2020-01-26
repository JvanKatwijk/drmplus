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

#ifndef	__FRAME_HANDLER__
#define	__FRAME_HANDLER__

#include	<QThread>
#include	<atomic>
#include	<complex>
#include	"ringbuffer.h"
#include	"ui_drm-decoder.h"
#include	"basics.h"
#include	"reader.h"
#include	"freqsyncer.h"
#include	"timesyncer.h"
#include	"wordcollector.h"
#include	"time-correlator.h"
#include	"equalizer-1.h"
#include	"fac-processor.h"
#include	"sdc-processor.h"
#include	"msc-processor.h"

class		drmDecoder;

class	frameHandler: public QThread {
Q_OBJECT
public:
		frameHandler	(drmDecoder *,
	                 	 RingBuffer<std::complex<float>> *,
	                         drmParameters	*,
	                         RingBuffer<std::complex<float>> *,
	                         RingBuffer<std::complex<float>> *);
		~frameHandler	();
	void	selectService	(int);
	void	set_constellationView	(const QString &);
private:
	drmDecoder	*theRadio;
	drmParameters	*params;
	RingBuffer<std::complex<float>> *eqBuffer;
	RingBuffer<std::complex<float>> *iqBuffer;
	theReader	myReader;
	freqSyncer	my_freqSyncer;
	timeSyncer	my_timeSyncer;
	wordCollector	my_wordCollector;
	timeCorrelator	my_timeCorrelator;
	equalizer_1	my_equalizer;
	mscProcessor	*my_mscProcessor;
	facProcessor    my_facProcessor;
	sdcProcessor    my_sdcProcessor;
	void		run		();
	int		nSymbols;
	std::atomic<bool>	running;
signals:
	void		setTimeSync		(bool);
	void		setFACSync		(bool);
	void		setSDCSync		(bool);
	void		show_fineOffset		(float);
	void		show_coarseOffset	(float);
	void		show_timeDelay		(float);
	void		show_timeOffset		(float);
	void		show_clockOffset	(float);
	void		show_angle		(float);
	void		cleanup_db		();
	void		update_GUI		();
};

#endif

