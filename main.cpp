#
/*
 *    Copyright (C) 2020
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the DRM+ receiver
 *
 *    DRM+ receiver is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    DRM+ receiver is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with DRM+ receiver; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Main program
 */
#include	<QApplication>
#include	<QSettings>
#include	<QDir>
#include	<unistd.h>
#include	"radio.h"


QString fullPathfor (QString v) {
QString fileName;

	if (v == QString (""))
	   return QString ("/tmp/xxx");

	if (v. at (0) == QChar ('/'))           // full path specified
	   return v;

	fileName = QDir::homePath ();
	fileName. append ("/");
	fileName. append (v);
	fileName = QDir::toNativeSeparators (fileName);

	if (!fileName. endsWith (".ini"))
	   fileName. append (".ini");

	return fileName;
}

#define	DEFAULT_INI	".drm-radio.ini"
#define	STATION_LIST	".drmradio-stations.bin"
//#define	STATION_LIST	 ".jsdr-fm-stations.bin"

int	main (int argc, char **argv) {
int32_t		opt;
/*
 *	The default values
 */
#if QT_VERSION >= 0x050600
        QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);
#endif

	QApplication a (argc, argv);
QSettings	*ISettings;		/* .ini file	*/
RadioInterface	*MyRadioInterface;
QString iniFile = QDir::homePath ();
QString stationList     = QDir::homePath ();
        iniFile. append ("/");
        iniFile. append (DEFAULT_INI);
        iniFile = QDir::toNativeSeparators (iniFile);

        stationList. append ("/");
        stationList. append (STATION_LIST);
        stationList = QDir::toNativeSeparators (stationList);

	ISettings	= new QSettings (iniFile, QSettings::IniFormat);
/*
 *	Before we connect control to the gui, we have to
 *	instantiate
 */
        MyRadioInterface = new RadioInterface (ISettings,
	                                       stationList, 192000);
	MyRadioInterface -> show ();
        a. exec ();
/*
 *	done:
 */
	fflush (stdout);
	fflush (stderr);
	qDebug ("It is done\n");
	ISettings	-> sync ();
	fprintf (stderr, "we gaan deleten\n");
	delete MyRadioInterface;
//	ISettings	-> ~QSettings ();
}
