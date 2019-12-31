#include <stdio.h>
#include <gb/gb.h>

enum note_t {
  C0 = 44, Cd0 = 156, D0 = 262, Dd0 = 363, E0 = 457, F0 = 547, Fd0 = 631, G0 = 710, Gd0 = 786, A0 = 854, Ad0 = 923, B0 = 986,
  C1 = 1046, Cd1 = 1102, D1 = 1155, Dd1 = 1205, E1 = 1253, F1 = 1297, Fd1 = 1339, G1 = 1379, Gd1 = 1417, A1 = 1452, Ad1 = 1486, B1 = 1517,
  C2 = 1546, Cd2 = 1575, D2 = 1602, Dd2 = 1627, E2 = 1650, F2 = 1673, Fd2 = 1694, G2 = 1714, Gd2 = 1732, A2 = 1750, Ad2 = 1767, B2 = 1783,
  C3 = 1798, Cd3 = 1812, D3 = 1825, Dd3 = 1837, E3 = 1849, F3 = 1860, Fd3 = 1871, G3 = 1881, Gd3 = 1890, A3 = 1899, Ad3 = 1907, B3 = 1915,
  C4 = 1923, Cd4 = 1930, D4 = 1936, Dd4 = 1943, E4 = 1949, F4 = 1954, Fd4 = 1959, G4 = 1964, Gd4 = 1969, A4 = 1974, Ad4 = 1978, B4 = 1982,
  C5 = 1985, Cd5 = 1988, D5 = 1992, Dd5 = 1995, E5 = 1998, F5 = 2001, Fd5 = 2004, G5 = 2006, Gd5 = 2009, A5 = 2011, Ad5 = 2013, B5 = 2015,
  SILENCE, END
};

void sound_init()
{
	NR52_REG = 0x8BU; /* make sure sound is enabled */
	NR51_REG = 0xFFU; /* send sound to left and rignt */
	NR50_REG = 0x77U;
}

void sound_play(UINT16 sweeptime, UINT16 sweepmode, UINT16 sweepshifts, UINT16 patternduty, UINT16 soundlength, UINT16 envinit, UINT16 envmode,	UINT16 envsteps, UINT16 freq, UINT16 consecutive,	UINT16 initialflag)
{
	NR10_REG = (sweepshifts | (sweepmode << 3) | (sweeptime << 4));
	NR11_REG = (patternduty << 6) | soundlength;
	NR12_REG = (envsteps | (envmode << 3) | (envinit << 4));
	NR13_REG = freq;
	NR14_REG = ((freq>>8) | (consecutive << 6) | (initialflag << 7));
}
// flip
void sound_flip()
{
	sound_play(7, 0, 5, 2, 30, 15, 1, 4, G1, 1, 1);
}

// die
void sound_die()
{
	sound_play(3, 1, 2, 0, 0, 15, 0, 4, Cd1, 0, 1);
}

// check
void sound_check()
{
	sound_play(0, 0, 0, 2, 20, 15, 0, 2, Dd2, 1, 1);
}

// coin 
void sound_coin()
{
	sound_play(5, 0, 4, 2, 0, 15, 0, 4, Dd2, 0, 1);
}

void sound_boom()
{
	int length;
	int envinit;
	int envmode;
	int envsteps;
	int polyfreq; 
	int polystep;
	int polydiv;
	int consecutive;
	int initialflag;
	length = 0; /* 0 - 63 */
	envinit = 15; /* 0 - 15 */
	envsteps = 5; /* 0 - 7 */
	envmode = 0; /* 0 = decrease 1 = increase */
	polyfreq = 4; /* 0 - 7 */
	polystep = 0; /* 0 - 1 */
	polydiv = 5; /* 0 - 7 */
	consecutive = 0; /* 0 - 1 */
	initialflag = 1; /* 0 - 1 */
	NR41_REG = length;
	NR42_REG = (envsteps + (envmode * 8) + (envinit * 16));
	NR43_REG = (polydiv + (polystep * 8) + (polyfreq * 16));
	NR44_REG = (consecutive * 64) + 128;
}