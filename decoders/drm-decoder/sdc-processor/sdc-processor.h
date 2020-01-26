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

#ifndef	__SDC_PROCESSOR__
#define	__SDC_PROCESSOR__

#include	<QObject>
#include	"basics.h"
#include	"ringbuffer.h"
#include	"checkcrc.h"
#include	"prbs.h"
#include	"mapper.h"
#include	"qam4-metrics.h"
#include	"sdc-handler.h"
class	drmDecoder;

	class	sdcProcessor: public QObject {
Q_OBJECT
public:
		sdcProcessor	(drmDecoder *theParent,
	                         drmParameters *params,
	                         RingBuffer<std::complex<float>> *);
	       ~sdcProcessor	();
	bool	processSDC	(theSignal **);
	void	set_Viewer	(bool);
private:
typedef struct {
	int	symbol;
	int	index;
} address;
	drmDecoder	*theParent;
	drmParameters	*params;
	RingBuffer<std::complex<float>> *iqBuffer;
	bool		show_Constellation;
	prbs		*prbs_12;
	prbs		*prbs_14;
	qam4_metrics	my_qam4_metrics;
	checkCRC	theCRC;
	Mapper		*Y21Mapper;
	sdcHandler	*handler_14;
	sdcHandler	*handler_12;
	theSignal	sdcCells [936 + 10];
	int		nr_sdcSamples;
	uint8_t		sdcBits	[4 + 4 + 16 + 8 * 113];
	void		extractData	(uint8_t *data, int size);
	int		sdcField	(uint8_t *data, int index);

signals:
	void		show_iq		();
};
#endif

