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
#ifndef	__DATA_FRAME_PROCESSOR__
#define	__DATA_FRAME_PROCESSOR__

#include	<QObject>
#include	<cstring>
#include	"basics.h"

class	drmDecoder;
class	packetAssembler;
class	fecHandler;

class	dataFrameProcessor: QObject {
Q_OBJECT
public:
		dataFrameProcessor	(drmDecoder *, drmParameters *,
	                                              int, int);
		~dataFrameProcessor	(void);
	void	process		(uint8_t *, bool);
private:
	drmDecoder	*drmMaster;
	drmParameters	*params;
	int		shortId;
	int		streamId;
	int		lengthA_total;
	int		lengthB_total;
	uint8_t		*firstBuffer;
	int16_t		numFrames;
	fecHandler	*my_fecHandler;
	packetAssembler	*my_packetAssembler;
	int16_t		old_CI;
	void	process_packets	(uint8_t *, int16_t,
	                         int16_t, int16_t, int16_t, int16_t);
	void	handle_uep_packets	(uint8_t *, int16_t,
	                         int16_t, int16_t, int16_t, int16_t);
	void	handle_eep_packets	(uint8_t *, int16_t, int16_t, int16_t);
	void	handle_packets_with_FEC	(uint8_t *, int16_t, uint8_t);
	void	handle_packets	(uint8_t *, int16_t, uint8_t);

	void	process_syncStream (uint8_t *v, int16_t mscIndex,
                                    int16_t startHigh,
                                    int16_t lengthHigh,
                                    int16_t startLow,
                                    int16_t lengthLow);
	void	handle_uep_syncStream (uint8_t *v, int16_t mscIndex,
                                   int16_t startHigh, int16_t lengthHigh,
                                   int16_t startLow, int16_t lengthLow);
	void	handle_eep_syncStream (uint8_t *v, int16_t mscIndex,
                                   int16_t startLow, int16_t lengthLow);
	void	handle_syncStream	(uint8_t *v, int16_t, int16_t);
};

#endif

