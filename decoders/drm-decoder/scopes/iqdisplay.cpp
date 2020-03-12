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
#include	"iqdisplay.h"
#include	<QtCharts/QScatterSeries>
#include	<QtCharts/QLegendMarker>
#include        <QtCharts/QValueAxis>
#include	<QtGui/QImage>
#include	<QtGui/QPainter>
#include	<QtCore/QtMath>

	IQDisplay::IQDisplay (QFrame *parent):
	                          QChartView (new QChart (), parent) {

	this	-> parent	= parent;
	series0	= new QScatterSeries ();
	series0 -> setName ("IQ");
	series0 -> setMarkerShape (QScatterSeries::MarkerShapeCircle);
	series0 -> setMarkerSize (8.0);

	for (int i = 0; i < 100; i ++)
           series0 -> append (-2 + 0.02 * i, fmod (-2 + 0.1 * i, 2));

	this	-> setSizePolicy (QSizePolicy::Ignored,
	                                  QSizePolicy::Ignored);
	setRenderHint (QPainter::Antialiasing);
	chart ()	-> setTitle ("constellation diagram");
	chart ()	-> legend()->setAlignment(Qt::AlignBottom);
	chart ()	-> setTheme(QChart::ChartThemeBlueCerulean);
	chart ()	-> resize (250, 250);
	QValueAxis * axisXlog = new QValueAxis;
//	axisXlog	-> setMinorTickCount (0.2);
        axisXlog	-> setRange (-2, 2);
//	axisXlog	-> setMinorGridLineColor("#3F4F4F");
//	axisXlog	-> setGridLineColor("#4F5F5F");
        axisXlog	-> setTitleText ("I");
        axisXlog	-> setTitleBrush (QBrush("#AADDDD"));
	axisXlog	-> setLabelsColor ("#C4C4C4");

	QValueAxis * axisYlog = new QValueAxis;
//	axisYlog	-> setMinorTickCount (0.2);
        axisYlog	-> setRange (-2, 2);
//	axisXlog	-> setMinorGridLineColor ("#3F4F4F");
//	axisYlog	-> setGridLineColor("#4F5F5F");
        axisYlog	-> setTitleText ("Q");
        axisYlog	-> setTitleBrush (QBrush ("#AADDDD"));
        axisYlog	-> setLabelsColor("#C4C4C4");

        chart () 	-> addAxis (axisXlog, Qt::AlignBottom);
        chart () 	-> addAxis (axisYlog, Qt::AlignLeft);
	chart ()	-> createDefaultAxes();

	chart () 	-> setDropShadowEnabled (false);
	chart ()	-> addSeries (series0);
}

	IQDisplay::~IQDisplay() {
}

void	IQDisplay::show		() {
	QChartView::show ();
}

void	IQDisplay::hide		() {
	QChartView::hide ();
}

void	IQDisplay::DisplayIQ (std::complex<float> *z, float scale) {
QVector<QPointF> data;
	for (int i = 0; i < 100; i ++) {
	   data. append (QPointF (real (z [i]) - 1,
	                          imag (z [i])));
	}
	series0 -> replace (data);
}

