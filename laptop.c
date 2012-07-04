/*
 *        Project:  dwmstatus
 *       Filename:  main.c
 *
 *    Description:  status feed, based on TrilbyWhite's dwmStatus,
 *					Writes to root window, to be read by DWM
 *					Includes some characters from the ohsnap.icons
 *					font.
 *
 *        Version:  1.0
 *        Created:  2012-07-03 16:50
 *       Compiler:  gcc
 *
 *         Author:  Ian D Brunton (ib), iandbrunton at gmail .com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

/* below this number, battery is highlighted */
#define BATT_LOW	11
/* above this number, cpu is highlighted */
#define CPU_HIGH	50
/* interval between updates */
#define INTERVAL	1

/* files read for system info */
#define COLOUR_FILE	"/home/ian/.config/herbstluftwm/colours"
#define CPU_FILE	"/proc/stat"
#define MEM_FILE	"/proc/meminfo"
#define BATT_NOW	"/sys/class/power_supply/BAT1/charge_now"
#define BATT_FULL	"/sys/class/power_supply/BAT1/charge_full"
#define BATT_STAT	"/sys/class/power_supply/BAT1/status"
#define VOL_FILE	"/home/ian/.local/share/volume"
#define PAC_FILE	"/home/ian/.local/share/pacman_count"
#define AUR_FILE	"/home/ian/.local/share/aur_count"
#define W_MAIL		"/home/ian/.local/share/mailcount_wolfshift"
#define I_MAIL		"/home/ian/.local/share/mailcount_iandbrunton"
#define DROPBOX		"dropbox status"

/* display format strings */
#define CPU_STR			"Ñ %d%%"				/* CPU percent when below CPU_HI% */
#define CPU_HI_STR		"Ñ %d%%"                /* CPU percent when above CPU_HI */
#define MEM_STR			"   Mem %ld (%ld%%)"		/* memory, takes (up to) 3 integers: free, buffers, and cache */
#define BATT_STR		"   µ %d%%"				/* Battery, unplugged, above BATT_LOW% */
#define BATT_LOW_STR	"   µ %d%%"					/* Battery, unplugged, below BATT_LOW% remaining */
#define BATT_CHRG_STR	"   µ %d%%"					/* Battery, when charging (plugged into AC) */
#define VOL_STR			"   í %d%%"
#define DB_IDLE			"   Ñ"                      		/* dropbox idle */
#define DB_UP			"   Û"                  /* dropbox uploading */
#define DB_DOWN			"   Ú"                  /* dropbox downloading */
#define PAC0_STR		"   Pac %d"                         /* pacman, no updates available */
#define PAC_STR			"   Pac %d"
#define AUR0_STR		" / AUR %d"                         /* AUR, no updates available */
#define AUR_STR			" / AUR %d"
#define W_MAIL_STR		"   Wolf %d"
#define W_NO_MAIL_STR	"   Wolf %d"
#define I_MAIL_STR		" / Ian %d"
#define I_NO_MAIL_STR	" / Ian %d"
#define DATE_TIME_FMT	"   |   %a %d %b * %H:%M "

int
main (int argc, char *argv[])
{
	Display *display;
	Window root;
	int num, num2;
	long jif1, jif2, jif3, jift;
	long lnum1, lnum2, lnum3, lnum4;
	char statnext[80], status[256];
	char dbstatus[13];
	time_t current;
	FILE *infile;

	infile = fopen (CPU_FILE, "r");
	fscanf (infile, "cpu %ld %ld %ld %ld", &jif1, &jif2, &jif3, &jift);
	fclose (infile);

	display = XOpenDisplay (NULL);
	if (display == NULL) {
		fprintf (stderr, "ERROR: could not open display\n");
		exit (1);
	}
	root = XRootWindow (display, DefaultScreen (display));

	for (;;) {
		status[0] = '\0';

		/* cpu use: */
		infile = fopen (CPU_FILE, "r");
		fscanf (infile, "cpu %ld %ld %ld %ld", &lnum1, &lnum2, &lnum3, &lnum4);
		fclose (infile);

		if (lnum4 > jift)
			num = (int) 100 * (((lnum1 - jif1) + (lnum2 - jif2) + (lnum3 - jif3)) / (lnum4 - jift));
		else
			num = 0;

		jif1 = lnum1;
		jif2 = lnum2;
		jif3 = lnum3;
		jift = lnum4;

		if (num > CPU_HIGH)
			sprintf (statnext, CPU_HI_STR, num);
		else
			sprintf (statnext, CPU_STR, num);
		strcat (status, statnext);

		/* memory use: */
		infile = fopen (MEM_FILE, "r");
		fscanf (infile, "MemTotal: %ld kB\nMemFree: %ld kB\nBuffers %ld kB\nCached: %ld kB\n",
				&lnum1, &lnum2, &lnum3, &lnum4);
		fclose (infile);
		sprintf (statnext, MEM_STR, lnum1 - lnum2, 100 * (lnum1 - lnum2) / lnum1);
		strcat (status, statnext);
		
		/* power/battery: */
		infile = fopen (BATT_NOW, "r");
		fscanf (infile, "%ld\n", &lnum1);
		fclose (infile);

		infile = fopen (BATT_FULL, "r");
		fscanf (infile, "%ld\n", &lnum2);
		fclose (infile);

		infile = fopen (BATT_STAT, "r");
		fscanf (infile, "%s\n", statnext);
		fclose (infile);

		num = lnum1 * 100 / lnum2;

		if (strncmp (statnext, "Charging", 8) == 0)
			sprintf (statnext, BATT_CHRG_STR, num);
		else {
			if (num < BATT_LOW)
				sprintf (statnext, BATT_LOW_STR, num);
			else
				sprintf (statnext, BATT_STR, num);
		}
		strcat (status, statnext);

		/* volume */
		infile = fopen (VOL_FILE, "r");
		fscanf (infile, "%d", &num);
		fclose (infile);
		sprintf (statnext, VOL_STR, num);
		strcat (status, statnext);

		/* dropbox */
		infile = popen (DROPBOX, "r");
		fscanf (infile, "%s", dbstatus);
		pclose (infile);
		if (strcmp (dbstatus, "Downloading") == 0)
			sprintf (statnext, DB_DOWN);
		else if (strcmp (dbstatus, "Uploading") == 0)
			sprintf (statnext, DB_UP);
		else
			sprintf (statnext, DB_IDLE);
		strcat (status, statnext);

		/* pacman/aur */
		infile = fopen (PAC_FILE, "r");
		fscanf (infile, "%d", &num);
		fclose (infile);
		if (num > 0)
			sprintf (statnext, PAC_STR, num);
		else
			sprintf (statnext, PAC0_STR, num);
		strcat (status, statnext);

		infile = fopen (AUR_FILE, "r");
		fscanf (infile, "%d", &num2);
		fclose (infile);
		num2 -= num;
		if (num2 > 0)
			sprintf (statnext, AUR_STR, num2);
		else
			sprintf (statnext, AUR0_STR, num2);
		strcat (status, statnext);

		/* mail: */
		infile = fopen (W_MAIL, "r");
		fscanf (infile, "%d", &num);
		fclose (infile);
		if (num > 0)
			sprintf (statnext, W_MAIL_STR, num);
		else
			sprintf (statnext, W_NO_MAIL_STR, num);
		strcat (status, statnext);

		infile = fopen (I_MAIL, "r");
		fscanf (infile, "%d", &num);
		fclose (infile);
		if (num > 0)
			sprintf (statnext, I_MAIL_STR, num);
		else
			sprintf (statnext, I_NO_MAIL_STR, num);
		strcat (status, statnext);

		/* date and time: */
		time (&current);
		strftime (statnext, 35, DATE_TIME_FMT, localtime (&current));
		strcat (status, statnext);

		/* set root window name */
		XStoreName (display, root, status);
		XFlush (display);
		sleep (INTERVAL);
	}

	XCloseDisplay (display);
	return EXIT_SUCCESS;
}
