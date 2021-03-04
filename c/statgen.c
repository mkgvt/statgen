/*
--	StatGen - inspiration from Matt Storch's Variance.cpp
*/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define  INLINE  /* open-code stats routines */
#include "stats.h"

#define BOOL	int
#define TRUE	1
#define FALSE	0

#define INITIAL_SIZE	100000
#define	SIZE_INC			100000

#define INT_WIDTH				5
#define FLOAT_WIDTH			11
#define DECIMAL_PLACES	4

#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))

const char helpString[] = {
	"--- Usage: %s [options] [files]\n"
	"\n"
	"  General options:\n"
	"    -b\tset bit-mode (default off)\n"
	"    -h\thelp (default off)\n"
	"    -f\tformat to use for displaying output\n"
	"    -l#\tset confidence level (default # = 0.95)\n"
	"    -t\tuse the T distribution to compute standard error (default < 30)\n"
	"    -z\tuse the Z distribution to compute standard error (default >= 30)"
	"\n\n"
	"  Display options:\n"
	"    -a\taverage (default on)\n"
	"    -c\tcount (default on)\n"
	"    -d\tstandard deviation (default on)\n"
	"    -e\tstandard error (default off)\n"
	"    -m\tmaximum (default on)\n"
	"    -n\tminimum (default on)\n"
	"    -p\tconfidence interval half-width in percent (default on)"
	"\n"
	"    -s\tsum (default off)\n"
	"    -v\tvariance (default on)\n"
	"    -w\tconfidence interval half-width (default on)\n"
	"    -x\tdo not display column headings (default off)\n"
	"\n"
	"  If files are not specified, input with be from stdin.\n"
	"\n"
	"  Choosing specific display options supresses the automatic\n"
	"  display of values (by default, headings are displayed).\n"
	"\n"
	"  By Matt Storch and Mark Gardner {storch,mkgardne}@cs.uiuc.edu."
	"\n\n"
};

void ComputeStats(FILE *in);
double Z(double p);
double T(double p, int ndf);
void GetOptions(int argc, char *const argv[]);
void DisplayHeadings(void);
void DisplayValues(long cnt, double sum, double min, double max, double avg, 
   double var, double stddev, double stderror, double hwidth, double phwidth);

BOOL bit_mode, displayAll, displayAverage, displayCount;
BOOL displayStdDev, displayStdErr, displayHeading;
BOOL displayMax, displayMin, displayPercentHalfWidth;
BOOL displaySum, displayVariance, displayHalfWidth, useT, useZ;
double confidenceLevel;

int debug = 0;

int main(int argc, char *const argv[]) {
	GetOptions(argc, argv);
	if (optind == argc) {
		ComputeStats(stdin);
  } else {
		int i, saw_stdin = 0;
		for (i = optind; i < argc; i++) {
			if (!saw_stdin && strncmp(argv[i], "-", 1) == 0) {
				saw_stdin = 1;
				ComputeStats(stdin);
			} else {
				FILE *in = fopen(argv[i], "r");
				if (in == NULL) {
					fprintf(stderr, "-- Error: could not open file '%s' for reading.\n",
							argv[optind]);
					exit(-1);
				}
				ComputeStats(in);
				fclose(in);
			}
		}
  }

  return 0;
}

void ComputeStats(FILE *in) {
	int i, size;
  double n;
  double sum, avg, var;
  double min, max;
	double stdev, stderror, hwidth;

	stats_data  stats;

	stats_init(&stats);

	fscanf(in, "%lf", &n);
  while (!feof(in)) {
		stats_update(&stats, n);
		fscanf(in, "%lf", &n);
  }

	size = stats_count(&stats);
  if (size < 2) {
		fputs("-- Error: need at least two numbers as input.\n", stderr);
		exit(1);
  }

	avg = stats_mean(&stats);
	sum = size * avg;
	min = stats_min(&stats);
	max = stats_max(&stats);
	var = stats_variance(&stats);
	stdev = stats_stdev(&stats);
	stderror = stats_stderr(&stats);
	hwidth = stats_confidence(&stats, confidenceLevel);

	DisplayHeadings();
  DisplayValues(size, sum, min, max, avg, var, 
								stdev, stderror, hwidth, 100 * hwidth / avg);
}

void ShowUsage(const char *progname) {
	fprintf(stderr, helpString, progname);
}

void GetOptions(int argc, char *const argv[]) {
  int errorCount = 0;
  int c;

  bit_mode = FALSE;
  displayAll = TRUE;
  displayAverage = FALSE;
  displayCount = FALSE;
  displayStdDev = FALSE;
  displayStdErr = FALSE;
  displayHeading = TRUE;
  displayMax = FALSE;
  displayMin = FALSE;
  displayPercentHalfWidth = FALSE;
  displaySum = FALSE;
  displayVariance = FALSE;
  displayHalfWidth = FALSE;
  confidenceLevel = 0.95;
  useT = FALSE;
  useZ = FALSE;

  opterr = 0; // disable getopt generated error msg
  while ((c = getopt(argc, argv, "abcdehl:mnpstvwxz")) != -1) {
	switch (c) {
	 case 'a':
	  displayAverage = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'b':
	  bit_mode = TRUE;
	  break;

	 case 'c':
	  displayCount = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'd':
	  displayStdDev = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'e':
	  displayStdErr = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'l':
	  confidenceLevel = atof(optarg);
	  if (confidenceLevel < 0.0 || confidenceLevel > 1.0) {
			fprintf(stderr, 
							"-- Error:  confidence level must be between 0.0 and 1.0 "
							"(not %d).\n", confidenceLevel);
			errorCount++;
	  }
	  break;

	 case 'm':
	  displayMax = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'n':
	  displayMin = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'p':
	  displayPercentHalfWidth = TRUE;
	  displayAll = FALSE;
	  break;

	 case 's':
	  displaySum = TRUE;
	  displayAll = FALSE;
	  break;

	 case 't':
	  useT = TRUE;
	  useZ = FALSE;
	  break;

	 case 'v':
	  displayVariance = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'w':
	  displayHalfWidth = TRUE;
	  displayAll = FALSE;
	  break;

	 case 'x':
	  displayHeading = FALSE;
	  break;

	 case 'z':
	  useZ = TRUE;
	  useT = FALSE;
	  break;

	 default:
	  errorCount++;
	  break;
	}
  }
  if (errorCount != 0) {
	ShowUsage(argv[0]);
	exit(-1);
  }
}

void DisplayHeadings() {
  if (displayHeading) {
		if (displayAll || displayCount)
			fprintf(stdout, " %*s", INT_WIDTH, "Count");
		if (displaySum)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "Sum");
		if (displayAll || displayMin)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "Min");
		if (displayAll || displayMax)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "Max");
		if (displayAll || displayAverage)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "Avg");
		if (displayVariance)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "Var");
		if (displayAll || displayStdDev)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "StdDev");
		if (displayStdErr)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "StdErr");
		if (displayAll || displayHalfWidth)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "HWidth");
		if (displayAll || displayPercentHalfWidth)
			fprintf(stdout, " %*s", FLOAT_WIDTH, "%HWidth");
		if (displayAll || displayCount  || displaySum || displayMin || 
				displayMax || displayAverage || displayVariance || 
				displayStdDev || displayStdErr ||
				displayHalfWidth || displayPercentHalfWidth) {
			fputc('\n', stdout);
		}
  }
}

void DisplayValues(long cnt, double sum, double min, double max, double avg, 
  double var, double stddev, double stderror, double hwidth, double phwidth) {
  if (displayAll || displayCount)
		fprintf(stdout, " %*d", INT_WIDTH, cnt);
  if (displaySum)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, sum);
  if (displayAll || displayMin)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, min);
  if (displayAll || displayMax)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, max);
  if (displayAll || displayAverage)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, avg);
  if (displayVariance)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, var);
  if (displayAll || displayStdDev)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, stddev);
  if (displayStdErr)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, stderror);
  if (displayAll || displayHalfWidth)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, hwidth);
  if (displayAll || displayPercentHalfWidth)
		fprintf(stdout, " %*.*f", FLOAT_WIDTH, DECIMAL_PLACES, phwidth);
  if (displayAll || displayCount  || displaySum || displayMin || 
			displayMax || displayAverage || displayVariance || 
			displayStdDev || displayStdErr ||
			displayHalfWidth || displayPercentHalfWidth) {
		fputc('\n', stdout);
  }
}
