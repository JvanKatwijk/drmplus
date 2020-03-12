#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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
#include	"device-handler.h"
#include	"radio.h"

	deviceHandler::deviceHandler	(RadioInterface *radio) {
	lastFrequency	= Mhz (100);
}

	deviceHandler::~deviceHandler	(void) {
}

int32_t	deviceHandler::getRate		(void) {
	return 192000;
}

void	deviceHandler::setVFOFrequency	(int32_t f) {
	lastFrequency	= f;
}

int32_t	deviceHandler::getVFOFrequency	(void) {
	return 1000000;
}

bool	deviceHandler::restartReader	(void) {
	return true;
}

void	deviceHandler::stopReader	(void) {
}

void	deviceHandler::resetBuffer	(void) {
}

int16_t	deviceHandler::bitDepth		(void) {
	return 10;
}

