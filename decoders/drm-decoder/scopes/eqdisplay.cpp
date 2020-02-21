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

#include	"eqdisplay.h"
#include        <QtCharts/QScatterSeries>
#include        <QtCharts/QLegendMarker>
#include        <QtCharts/QValueAxis>
#include        <QtGui/QImage>
#include        <QtGui/QPainter>
#include        <QtCore/QtMath>


	EQDisplay::EQDisplay	(QFrame *parent):
	                               QChartView (new QChart (), parent) {

        this    -> parent       = parent;

	amplitudes	= new QLineSeries ();
	phases		= new QLineSeries ();
	for (int i = 0; i < 206; i ++) {
	   amplitudes -> append (i, 0.05 * i);
	   phases     -> append (i, i % 10);
	}
	setRenderHint (QPainter::Antialiasing);
        chart ()        -> setTitle ("equalizer ");
	axisXlog	= new QValueAxis;
        axisXlog        -> setTitleText ("amplitude and phase");
	chart ()	-> setToolTip ("Green line is phase, other the amplitude");
        axisXlog        -> setTitleBrush (QBrush("#AADDDD"));
        axisXlog        -> setLabelsColor ("#C4C4C4");
        axisYlog	= new QValueAxis;
        axisYlog        -> setTitleBrush (QBrush ("#AADDDD"));
        axisYlog        -> setLabelsColor("#C4C4C4");
        chart ()        -> addAxis (axisXlog, Qt::AlignBottom);
        chart ()        -> addAxis (axisYlog, Qt::AlignLeft);

	chart ()	-> addSeries	(amplitudes);
	chart ()	-> addSeries	(phases);
}

	EQDisplay::~EQDisplay	() {}

void	EQDisplay::show		() {
	QChartView::show ();
}

void	EQDisplay::hide		() {
	QChartView::hide ();
}

bool	EQDisplay::isHidden	() {
	return QChartView::isHidden ();
}

static int xxx = 0;
void	EQDisplay::show		(std::complex<float> *v, int amount) {
float	max	= 0;
	for (int i = 0; i < amount; i ++)
	   if (abs (v [i]) > max)
	      max = abs (v [i]);

	if (++xxx <= 3)
	   return;
	xxx = 0;
	max	= 2 * max;
	amplitudes	-> clear	();
	phases		-> clear	();
	for (int i = 0; i < amount; i ++) {
	   amplitudes -> append (i, abs (v [i]));
	   phases     -> append (i, arg (v [i]) + M_PI);
	}

        axisXlog        -> setRange (0, amount);
        axisYlog        -> setRange (0, max);
}

