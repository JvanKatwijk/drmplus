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

#ifndef	__FM_DECODER__
#define	__FM_DECODER__

#include	<QObject>
#include	<QFrame>
#include	<QDialog>
#include	"radio-constants.h"
#include	"ringbuffer.h"
#include	"ui_fm-decoder.h"
#include	"virtual-decoder.h"

class	fmProcessor;
class	RadioInterface;

class	fmDecoder: public virtualDecoder, private Ui_fmdecoder  {
Q_OBJECT
public:
	fmDecoder (RadioInterface *,
	           RingBuffer<std::complex<float>> *,
	           RingBuffer<std::complex<float>> *);
	~fmDecoder	();
private:
	fmProcessor	*theProcessor;
	QFrame		*myFrame;
private slots:
	void		handle_demodulatorSelector	(const QString &);
	void		handle_fmDeemphasisSelector	(const QString &);
	void		handle_fmMode			(const QString &);
	void		handle_audioChannelSelect	(const QString &);
	void		handle_squelchButton		();
	void		handle_squelchSlider		(int);
public slots:
	void		showLocked	(bool, float);
	void		audioAvailable	(int, int);
	void		setRDSisSynchronized (bool b);
	void		setbitErrorRate (double d);
	void		setGroup (int i);
	void		setPTYCode (int i);
	void		setPiCode (int i);
	void		setStationLabel (const QString &s);
	void		clearStationLabel (void);
	void		setRadioText (const QString &s);
	void		clearRadioText (void);
	void		setAFDisplay (int i);
	void		setMusicSpeechFlag (int i);
	void		clearMusicSpeechFlag (void);
	void		setCRCErrors (int n);
	void		setSyncErrors (int s);

signals:
	void		processAudio	(int, int);
};

#endif

	
