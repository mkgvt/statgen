/*
-- Statistics ADT
-- 
-- Copyright (c) 1998-2002, Mark K. Gardner
-- All rights reserved.
-- 
-- Note: update the mean and variance using the recursive definitions
-- in Sheldon Ross, Simulation, 2nd Edition, Academic Press, 1997, pg.
-- 116.
*/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include "stats.h"


#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define SQR(x) ((x) * (x))


INLINE static double Z(double p);
INLINE static double T(double p, int ndf);


INLINE void stats_init(stats_data *data) {
	data->count    = 0;
	data->min      = DBL_MAX;
	data->max      = DBL_MIN;
	data->mean     = 0.0;
	data->variance = 0.0;
}

INLINE void stats_update(stats_data *data, STATS_DATATYPE x) {
	double oldmean = data->mean;
	data->min = MIN(data->min, x);
	data->max = MAX(data->max, x);
	data->mean += (x - data->mean) / (data->count + 1);
	if (data->count > 0) {
		data->variance = (1.0 - 1.0/data->count) * data->variance + 
			(data->count + 1) * SQR(data->mean - oldmean);
	}
	data->count++;
}

INLINE uint32 stats_count(stats_data *data) {
	return data->count;
}

INLINE STATS_DATATYPE stats_min(stats_data *data) {
	return data->min;
}

INLINE STATS_DATATYPE stats_max(stats_data *data) {
	return data->max;
}

INLINE double stats_mean(stats_data *data) {
	return data->mean;
}

INLINE double stats_variance(stats_data *data) {
	return data->variance;
}

INLINE double stats_stdev(stats_data *data) {
	return sqrt(data->variance);
}

INLINE double stats_stderr(stats_data *data) {
	return sqrt(data->variance / data->count);
}

INLINE double stats_confidence(stats_data *data, double level) {
	double std_error = stats_stderr(data);
	double fudge;
	if (data->count > 1 && data->count < 30) {
			fudge = T((1.0 - level) / 2, data->count - 1);
	} else {
		if (data->count < 2) {
			fputs("-- Warning: less than two points are "
					"available to compute confidence interval.", stderr);
		}
		fudge = Z((1.0 - level) / 2);
	}
	return fudge * std_error;
}


// Z() and T() are taken from p. 276 of "Simulating Computer Systems" by
// M. H. MacDougall.
INLINE double Z(double p) {
  double q, z1, n, d;

  q = (p > 0.5) ? (1 - p) : p;
  z1 = sqrt(-2.0 * log(q));
  n = (0.010328 * z1 + 0.802853) * z1 + 2.515517;
  d = ((0.0013080 * z1 + 0.189269) * z1 + 1.43278) * z1 + 1;
  z1 -= n / d;
  if (p > 0.5)
	z1 = -z1;
  return z1;
}

INLINE double T(double p, int ndf) {
  int i;
  double z1, z2, h[4],  x = 0.0;
  
  z1 = fabs(Z(p));
  z2 = z1 * z1;

  h[0] = 0.25 * z1 * (z2 + 1.0);
  h[1] = 0.010416667 * z1 * ((5.0 * z2 + 16.0) * z2 + 3.0);
  h[2] = 0.002604167 * z1 * ((3.0 * z2 + 19.0) * z2 - 15.0);
  h[3] = z1 * ((((79.0*z2 + 776.0)* z2 + 1482.0) * z2 - 1920.0) * z2 - 945.0);
  h[3] *= 0.000010851;
  
  for (i = 3; i >= 0; i--)
	x = (x + h[i]) / (double)ndf;
  z1 += x;
  if (p > 0.5)
	z1 = -z1;
  return z1;
}
