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
#ifndef	__PACKET_ASSEMBLER__
#define	__PACKET_ASSEMBLER__

#include	<QByteArray>
#include	<QString>
#include	<stdint.h>
#include	"basics.h"

class	drmDecoder;
class	motHandler;
class	virtual_dataHandler;

class	packetAssembler {
public:
		packetAssembler		(drmParameters *,
	                                 drmDecoder *, uint16_t);
		~packetAssembler	(void);
	void	assemble	(uint8_t *, int16_t, int16_t);
private:
	drmParameters	*params;
	uint16_t	applicationId;
	int16_t		mscIndex;
	bool		waitforFirst;
	int16_t		old_CI;
	QByteArray	series;
	virtual_dataHandler	*my_dataHandler;
	void		add_mscDatagroup	(QByteArray &);
};

#endif

