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


	EQDisplay::EQDisplay	(QwtPlot *plotgrid) {
	this	-> plotgrid	= plotgrid;
	plotgrid		-> setCanvasBackground (QColor ("white"));
	grid                    = new QwtPlotGrid;
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
        grid    -> setMajPen (QPen(QColor ("black"), 0, Qt::DotLine));
#else
        grid    -> setMajorPen (QPen(QColor ("black"), 0, Qt::DotLine));
#endif
        grid    -> enableXMin (true);
        grid    -> enableYMin (true);
#if defined QWT_VERSION && ((QWT_VERSION >> 8) < 0x0601)
        grid    -> setMinPen (QPen(QColor ("black"), 0, Qt::DotLine));
#else
        grid    -> setMinorPen (QPen(QColor ("black"), 0, Qt::DotLine));
#endif
        grid    -> attach (plotgrid);

	spectrumCurve   = new QwtPlotCurve ("");
        spectrumCurve   -> setPen (QPen(Qt::red));
        spectrumCurve   -> setOrientation (Qt::Horizontal);
        spectrumCurve   -> setBaseline  (0);
        spectrumCurve   -> attach (plotgrid);
	phaseCurve	= new QwtPlotCurve ("");
        phaseCurve	-> setPen (QPen(Qt::blue));
        phaseCurve	-> setOrientation (Qt::Horizontal);
        phaseCurve	-> setBaseline  (0);
        phaseCurve	-> attach (plotgrid);
}

	EQDisplay::~EQDisplay	() {}

void	EQDisplay::show		(std::complex<float> *v, int amount) {
double	max	= 0;
int	i;
double X_axis	[amount];
double plotData [amount];
double phaseData [amount];

	for (i = 0; i < amount; i ++) {
	   X_axis	[i] = i - amount / 2;
	   plotData	[i] = abs (v [i]);
	   if (plotData [i] > max)
	      max = plotData [i];
	}
	for (i = 0; i < amount; i ++)
	   plotData [i] = plotData [i] / max * 10;
	for (i = 0; i < amount; i ++) 
	   phaseData [i] = arg (v [i]) + 5;

	plotgrid	-> setAxisScale (QwtPlot::xBottom,
	                                 (double)X_axis [0],
	                                 (double)X_axis [amount - 1]);
	plotgrid	-> enableAxis   (QwtPlot::xBottom);
	plotgrid	-> setAxisScale (QwtPlot::yLeft, 0, 10);
	spectrumCurve	-> setSamples (X_axis, plotData, amount);
	phaseCurve	-> setSamples (X_axis, phaseData, amount);
	plotgrid	-> replot ();
}

