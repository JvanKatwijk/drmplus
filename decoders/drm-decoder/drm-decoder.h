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

#ifndef __DRM_PLUS_DECODER__
#define __DRM_PLUS_DECODER__

#include	<QObject>
#include        <QFrame>
#include        <atomic>
#include        <complex>
#include        "ringbuffer.h"
#include        "ui_drm-decoder.h"
#include        "basics.h"
#include	"virtual-decoder.h"

class		frameHandler;
class           RadioInterface;
class		EQDisplay;
class		IQDisplay;

//#include	"ui_technical-data.h"
#include	"ui_audiodata.h"

class   drmDecoder: public virtualDecoder, private Ui_drmdecoder {
Q_OBJECT
public:
        drmDecoder	(RadioInterface *,
	                 RingBuffer<std::complex<float>> *,
	                 RingBuffer<std::complex<float>> *);
        ~drmDecoder	();

private:
        RadioInterface  *theRadio;
	RingBuffer<std::complex<float>>	*audioBuffer;
	std::atomic<bool>       running;
	QFrame		*myFrame;
//	QFrame		*techFrame;
	QFrame		*audioFrame;
//	Ui_technicaldata techData;
	Ui_audiodata	audioData;
	EQDisplay	*my_eqDisplay;
	IQDisplay	*my_iqDisplay;
	frameHandler	*my_frameHandler;
	RingBuffer<std::complex<float>> *eqBuffer;
	RingBuffer<std::complex<float>> *iqBuffer;
	float		phaseOffset;
	drmParameters	params;
	void		show_audioData		(drmParameters *, int);
	QString		getProgramType		(int);
public slots:
	void		show_stationLabel	(const QString &);
	void		show_eqsymbol		(int);
	void		show_iq			();
	void		show_mer		(float);
	void		show_audioMode		(QString);
	void		show_coarseOffset	(float);
	void		show_inputShift		(int);
	void		show_timeDelay		(float);
	void		show_timeOffset		(float);
	void		show_clockOffset	(float);
	void		show_angle		(float);
//	void		sampleOut		(float, float);
	void		samplesAvailable	();
	void		faadSuccess		(bool);
	void		showMOT			(QByteArray, int);
	void		aacData			(QString);
	void		show_country		(QString);
	void		show_time		(QString);
	void		setTimeSync		(bool);
	void		setFACSync		(bool);
	void		update_GUI		();
	void		cleanup_db		();
	
private slots:
	void		executeSDCSync		(bool);
	void		showSNR			(float);
	void		showMessage		(QString);
//
	void		selectChannel_0		(void);
	void		selectChannel_1		(void);
	void		selectChannel_2		(void);
	void		selectChannel_3		(void);

	void		set_phaseOffset		(int);
	void		handle_constellationSelector	(const QString &);
	void		handle_equalizerButton	();
	void		handle_streamData	();
//signals:
//	void		audioAvailable		(int, int);
};

#endif

