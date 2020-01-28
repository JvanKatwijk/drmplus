#
/*
 *    Copyright (C)  2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
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

#ifndef	__DRM_RECEIVER__
#define	__DRM_RECEIVER__

#include	<complex>
#include	<vector>
#include        <QMainWindow>
#include        <QTimer>
#include        <QWheelEvent>
#include        <QLineEdit>
#include	<QTimer>
#include	<QComboBox>
#include	<sndfile.h>
#include        "ui_drm-receiver.h"
#include        "ringbuffer.h"

class		deviceHandler;
class           QSettings;
class           fftScope;
class           fft_scope;
class           keyPad;
class           programList;
class		audioSink;

class		virtualDecoder;

class	RadioInterface:public QMainWindow,
	               private Ui_MainWindow {
Q_OBJECT
public:
		RadioInterface (QSettings	*sI,
	                        QString		stationList,
	                        int		inputRate,
	                        QWidget		*parent = NULL);
		~RadioInterface	(void);
private:
	struct band {
	   int32_t	lowF;
	   int32_t	highF;
	   int32_t	currentOffset;
	}theBand;

	virtualDecoder	*theDecoder;
	QSettings       *settings;
	QString		stationList;
        int32_t         inputRate;
	int32_t		scopeWidth;
        int16_t         displaySize;
        int16_t         spectrumSize;
        double          *displayBuffer;
	audioSink	*audioHandler;
        int16_t         *outTable;
        deviceHandler	*theDevice;
        RingBuffer<std::complex<float> > *inputData;
        RingBuffer<std::complex<float> > *bufferData;
        RingBuffer<std::complex<float> > *audioData;
        fftScope        *hfScope;
	int16_t		delayCount;

	QTimer		secondsTimer;
	void		setupSoundOut   (QComboBox        *streamOutSelector,
                                         audioSink        *our_audioSink,
                                         int32_t          cardRate,
                                         int16_t          *table);

        keyPad          *mykeyPad;
        programList     *myList;
        QLineEdit       *myLine;
        void            adjust          	(int32_t);
private slots:
	void		doStart			(const QString &);
	void		processAudio		(int, int);
        deviceHandler	*setDevice	(const QString &,
	                                   RingBuffer<std::complex<float>> *);
        void            adjustFrequency_khz	(int);
        void            adjustFrequency_hz	(int);
        void            handle_myLine		(void);
        void            set_hfscopeLevel	(int);
	void		setFrequency		(uint32_t);
        void            handle_freqButton       (void);
	void		set_freqSave		(void);
	void		handle_quitButton	(void);
	void		updateTime		(void);
	void		closeEvent		(QCloseEvent *event);
	void		handle_decoderSelect	(const QString &);
	void		handle_frequencyBackwards	();
	void		handle_frequencyForwards	();
public slots:
	void		sampleHandler		(int amount);
};

#endif
