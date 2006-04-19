#include "coneiso.h"
#include <stdlib.h>
#include <stdio.h>

VSGTRIVAL _trival_l_on;
VSGTRIVAL _trival_l_off;
VSGTRIVAL _trival_m_on;
VSGTRIVAL _trival_m_off;
VSGTRIVAL _trival_s_on;
VSGTRIVAL _trival_s_off;
bool _b_coneiso_read_done=false;
char* coneisoFileDefault="c:\\coneiso.txt";

bool read_coneiso_data()
{
	char *cf=NULL;
	FILE *fp=NULL;

	if (_b_coneiso_read_done) return true;

	// check if the env var CONEISO is defined. If it is, use its value as the coneiso file. 
	// Otherwise, use the default.
	cf = getenv("CONEISO");
	if (!cf) 
	{
		cf = coneisoFileDefault;
		printf("CONEISO env var not defined. Using this file for coneiso values: %s\n", cf);
	}
	else
	{
		printf("CONEISO env var defined. Using this file for coneiso values: %s\n", cf);
	}

	// try to open file, then read values
	fp = fopen(cf, "r");
	if (!fp)
	{
		printf("Cannot open coneiso data file %s\n", cf);
	}
	else
	{
		// Expecting to read lines which look like this:
		// l r_off g_off b_off r_on g_on b_on
		// Lines which begin with a "#" are ignored
		char line[255];
		char ctyp;
		double r, g, b;
		double rr, gg, bb;
		bool b_l_ok = false;
		bool b_m_ok = false;
		bool b_s_ok = false;
		while (fgets(line, 255, fp))
		{
			if (line[0] != '#')
			{
				// attempt to read values.
				if (7 == sscanf(line, "%c %lf %lf %lf %lf %lf %lf", &ctyp, &r, &g, &b, &rr, &gg, &bb))
				{
					switch(ctyp)
					{
					case 'l':
					case 'L':
						_trival_l_off.a = r;
						_trival_l_off.b = g;
						_trival_l_off.c = b;
						_trival_l_on.a = rr;
						_trival_l_on.b = gg;
						_trival_l_on.c = bb;
						b_l_ok = true;
						break;
					case 'm':
					case 'M':
						_trival_m_off.a = r;
						_trival_m_off.b = g;
						_trival_m_off.c = b;
						_trival_m_on.a = rr;
						_trival_m_on.b = gg;
						_trival_m_on.c = bb;
						b_m_ok = true;
						break;
					case 's':
					case 'S':
						_trival_s_off.a = r;
						_trival_s_off.b = g;
						_trival_s_off.c = b;
						_trival_s_on.a = rr;
						_trival_s_on.b = gg;
						_trival_s_on.c = bb;
						b_s_ok = true;
						break;
					default:
						printf("Unknown cone type: %c\n", ctyp);
						break;
					}
				}
			}
		}
		fclose(fp);

		if (b_l_ok && b_m_ok && b_s_ok) _b_coneiso_read_done = true;
		else printf("Did not find valid lines for all three coneiso types!\n");
	}

	return _b_coneiso_read_done;
}


bool coneiso_l(VSGTRIVAL& from, VSGTRIVAL& to)
{
	if (!read_coneiso_data()) return false;
	from	= _trival_l_off;
	to		= _trival_l_on;
	return true;
}

bool coneiso_m(VSGTRIVAL& from, VSGTRIVAL& to)
{
	if (!read_coneiso_data()) return false;
	from	= _trival_m_off;
	to		= _trival_m_on;
	return true;
}

bool coneiso_s(VSGTRIVAL& from, VSGTRIVAL& to)
{
	if (!read_coneiso_data()) return false;
	from	= _trival_s_off;
	to		= _trival_s_on;
	return true;
}
