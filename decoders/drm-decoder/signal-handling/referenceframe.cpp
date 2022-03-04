#
/*
 *    Copyright (C) 2013
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J (JSDR).
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#include	"referenceframe.h"
#include	"basics.h"
#include	<cstring>
//
//	support functions
//
struct {
        int     carrier;
        float   phase;
} timeRefs [] = {
        {-80, 219}, {-79, 475}, {-77, 989}, {-53, 652},
        {-52, 652}, {-51, 140}, {-32, 819}, {-31, 819},
        { 12, 907}, { 13, 907}, { 14, 651}, { 21, 903},
        { 22, 391}, { 23, 903}, { 40, 203}, { 41, 203},
        { 42, 203}, { 67, 797}, { 68,  29}, { 79, 508},
        { 80, 508}, { -1,   0} };


bool	isTimeCell (int16_t carrier) {
int16_t	i;

	for (i = 0; timeRefs [i]. carrier != -1; i ++)
	   if (timeRefs [i]. carrier == carrier)
	      return true;

	return false;
}

std::complex<float> getTimeRef (int16_t carrier) {
int16_t i;

	for (i = 0; timeRefs [i]. carrier != -1; i ++)
	   if (timeRefs [i]. carrier == carrier)
	      return valueFor (sqrt (2), timeRefs [i]. phase);

	return 0;
}

bool	isGainCell (int16_t symbol, int16_t carrier) {
int16_t	p;
	p = carrier - 2 - 4 * (symbol % 4);
	if (p < 0) p = -p;
	   return (p % 16 == 0);
}

bool	isBoostCell (int16_t symbol, int16_t carrier) {
	(void)symbol;
	return (carrier == -106) || (carrier == -102) ||
	       (carrier ==  102) || (carrier == 106);
}

static int R_1024 [4][10] = {
	{ 39, 118, 197, 276, 354, 433,  39, 118, 197, 276},
	{ 37, 183, 402,  37, 183, 402,  37, 183, 402,  47},
	{110, 329, 475, 110, 329, 475, 110, 329, 475, 110},
	{ 79, 158, 236, 315, 394, 473,  79, 158, 236, 315}
};

static int Z_1024 [4][10] = {
	{473, 394, 315, 236, 158,  79,   0,   0,   0,   0},
	{183, 914, 402,  37, 475, 841, 768, 768, 987, 183},
	{549, 622, 475, 110,  37, 622, 256, 768, 329, 549},
	{ 79, 158, 236, 315, 394, 473, 158, 315, 473, 630}
};

static int Q_1024 [4][10] = {
	{329,  489, 894, 419, 607, 519, 1020, 942,  817, 939},
	{824, 1023,  74, 319, 225, 207,  348, 422,  395,  92},
	{959,  379,   7, 738, 500, 920,  440, 727,  263, 733},
	{907,  946, 924,  91, 189, 133,  910, 804, 1022, 433}
};

std::complex<float>	getGainValue (int16_t symbol, int16_t carrier) {
int16_t	x	= 4;
int16_t	y	= 4;
int16_t	k0	= 2;
int16_t	n	= symbol % y;
int16_t	m	= floor (symbol / y);
int16_t	p	= int16_t((float(carrier - k0 - n * x)) / (x * y));

int16_t	phase	= ((p * p) * R_1024 [n][m] + p * Z_1024 [n][m] 
	                                          + Q_1024 [n][m]) % 1024;

	if (isBoostCell (symbol, carrier))
	   return valueFor (sqrt (4.0), phase);
	else
	   return valueFor (sqrt (2.0), phase);
}

static struct {
	int	carrier;
	int	phase_1;
	int	phase_2;
} afsCells [] = {
	{-106, 134, 115}, {-102, 866, 135}, {-98, 588, 194}, {-94, 325, 293},
	{ -90,  77, 431}, {- 86, 868, 608}, {-82, 649, 825}, {-78, 445,  57},
	{ -74, 256, 353}, {- 70,  82, 688}, {-66, 946,  36}, {-62, 801, 452},
	{ -58, 671, 905}, {- 54, 566, 373}, {-50, 455, 905}, {-46, 369, 452},
	{- 42, 298,  39}, {- 38, 242, 689}, {-34, 200, 354}, {-30, 173,  59},
	{- 26, 161, 827}, {- 22, 164, 610}, {-18, 181, 433}, {-14, 213, 295},
	{- 10, 260, 197}, {-  6, 322, 138}, {- 2, 398, 118}, {  2, 489, 138},
	{  6, 595,  197}, {  10, 716, 295}, { 14, 851, 433}, { 18, 1001, 610},
	{  22, 142, 827}, {  26, 322,  59}, { 30, 516, 354}, { 34, 725, 689},
	{  38, 949,  39}, {  42, 164, 452}, { 46, 417, 905}, {  50, 685, 373},
	{  54, 968, 905}, {  58, 242, 452}, { 62, 554,  38}, {  66, 881, 688},
	{  70, 199, 353}, {  74, 556,  57}, { 78, 927, 825}, {  82, 289, 608},
	{  86, 690, 431}, {  90,  82, 293}, { 94, 512, 194}, {  98, 957, 135},
	{ 102, 393, 115}, { 106, 868, 134},  { -1,  -1,  -1}
};

bool	isAfsCell (int16_t symbol, int16_t carrier) {
	for (int i = 0; afsCells [i]. carrier != -1; i ++)
	   if (afsCells [i]. carrier == carrier)
	      return (symbol == 4) || (symbol == 39);
	return false;
}

struct {
	int symbol;
	int carrier;
} facTable [] = {
	{5, -78}, {5, -62}, {5, -46}, {5, -30}, {5, -14},
	{5,   2}, {5,  18}, {5,  34}, {5, 50}, {5, 66}, {5, 82},
	{6, -90}, {6, -74}, {6, -58}, {6, -42}, {6, -26}, {6, -10},
	{6,   6}, {6,  22}, {6, 38}, {6, 54}, {6, 70}, {6, 86},
	{7, -96}, {7, -70}, {7, -54}, {7, -38}, {7, -22}, {7, -6},
	{7,  10}, {7,  26}, {7,  42}, {7, 58}, {7, 74}, {7, 90},
	{8, -82}, {8, -66}, {8, -50}, {8, -34}, {8, -18}, {8, -2},
	{8,  14}, {8,  30}, {8, 46}, {8, 62}, {8, 78},
	{9, -78}, {9, -62}, {9, -46}, {9, -30}, {9,  -14},
	{9,   2}, {9,  18}, {9, 34}, {9, 50}, {9, 66}, {9, 82},
	{10,-90}, {10,-74}, {10, -58}, {10, -42}, {10, -26}, {10, -10},
	{10,  6}, {10, 22}, {10, 38}, {10, 54}, {10, 70}, {10, 86},
	{11,-86}, {11,-70}, {11, -54}, {11, -38}, {11, -22}, {11, -6},
	{11, 10}, {11, 26}, {11, 42}, {11, 58}, {11, 74}, {11, 90},
	{12,-82}, {12,-66}, {12, -50}, {12, -34}, {12, -18}, {12, -2},
	{12, 14}, {12, 30}, {12, 46}, {12, 62}, {12, 78},
	{13,-78}, {13,-62}, {13, -46}, {13, -30}, {13, -14},
	{13,  2}, {13, 18}, {13, 34}, {13, 50}, {13, 66}, {13, 82},
	{14,-90}, {14,-74}, {14, -58}, {14, -42}, {14, -26}, {14, -10},
	{14,  6}, {14, 22}, {14, 38}, {14, 54}, {14, 70}, {14, 86},
	{15,-86}, {15,-70}, {15, -54}, {15, -38}, {15, -22}, {15, -6},
	{15, 10}, {15, 26}, {15, 42}, {15, 58}, {15, 74},  {15, 90},
	{16,-82}, {16,-66}, {16, -50}, {16, -34}, {16, -18}, {16, -2},
	{16, 14}, {16, 30}, {16, 46}, {16, 62}, {16, 78},
	{17,-78}, {17,-62}, {17, -46}, {17, -30}, {17, -14},
	{17,  2}, {17, 18}, {17, 34}, {17, 50}, {17, 66}, {17, 82},
	{18,-90}, {18,-74}, {18, -58}, {18, -42}, {18, -26}, {18, -10},
	{18,  6}, {18, 22}, {18, 38},  {18, 54}, {18, 70},  {18, 86},
	{19,-86}, {19,-70}, {19, -54}, {19, -38}, {19, -22}, {19, -6},
	{19, 10}, {19, 26}, {19, 42}, {19, 58}, {19, 74}, {19, 90},
	{20,-82}, {20,-66}, {20, -50}, {20, -34}, {20, -18}, {20, -2},
	{20, 14}, {20, 30}, {20, 46},  {20, 62}, {20, 78},
	{21,-78}, {21,-62}, {21, -46}, {21, -30}, {21, -14},
	{21,  2}, {21, 18}, {21, 34}, {21, 50},  {21, 66}, {21, 82},
	{22,-90}, {22,-74}, {22, -58}, {22, -42}, {22, -26},  {22, -10},
	{22,  6}, {22, 22}, {22, 38}, {22, 54}, {22, 70},  {22, 86},
	{23,-86}, {23,-70}, {23, -54}, {23, -38}, {23, -22}, {23, -6},
	{23, 10}, {23, 26}, {23, 42}, {23, 58}, {23, 74}, {23, 90},
	{24,-82}, {24,-66}, {24, -50}, {24, -34}, {24, -18}, {24, -2},
	{24, 14}, {24, 30}, {24, 46}, {24, 62}, {24, 78},
	{25,-78}, {25,-62}, {25, -46}, {25, -30}, {25, -14},
	{25,  2}, {25, 18}, {25, 34}, {25, 50}, {25, 66}, {25, 82},
	{26,-90}, {26,-74}, {26, -58}, {-1, -1}
};

bool	isFACcell (int16_t symbol, int16_t carrier) {
	for (int index = 0; index < 244; index ++)
	   if ((facTable [index]. symbol == symbol) &&
	       (facTable [index]. carrier == carrier))
	      return true;
	return false;
}

void	getFACcell (int16_t index, int16_t *symbol, int16_t *carrier) {
	if ((index < 0) || (index >= 244)) {
	   *symbol = -1;
	   *carrier = -1;
	}
	else {
	   *symbol = facTable [index]. symbol;
	   *carrier = facTable [index]. carrier;
	}
}


struct {
	int symbol;
	int carrier;
} sdcTable [936];

bool	isSDCcell (int16_t symbol, int16_t carrier) {
	for (int index = 0; index < 936; index ++)
	   if ((sdcTable [index]. symbol == symbol) &&
	       (sdcTable [index]. carrier == carrier))
	      return true;
	return false;
}

void	getSDCcell (int16_t index, int16_t *symbol, int16_t *carrier) {
	if ((index < 0) || (index >= 936)) {
	   *symbol = -1;
	   *carrier = -1;
	}
	else {
	   *symbol = sdcTable [index]. symbol;
	   *carrier = sdcTable [index]. carrier;
	}
}

int	init_sdcTable () {
int nr_sdcSamples = 0;
	for (int symbol = 0; symbol < 5; symbol ++) {
           for (int carrier = K_min; carrier <= K_max; carrier ++) {
              if (isGainCell (symbol, carrier))
                 continue;
              if ((symbol == 0) && isTimeCell (carrier))
                 continue;
              if ((symbol == 4) && (isAfsCell (symbol, carrier)))
                 continue;
              sdcTable [nr_sdcSamples]. symbol = symbol;
              sdcTable [nr_sdcSamples]. carrier  = carrier;
              nr_sdcSamples ++;
           }
        }
	return nr_sdcSamples;
}

bool	frame_1 [symbolsperFrame * nrCarriers];
bool	frame_23 [symbolsperFrame * nrCarriers];
bool	frame_4 [symbolsperFrame * nrCarriers];

