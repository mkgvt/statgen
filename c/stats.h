/*
-- Statistics ADT
-- 
-- Copyright (c) 1998-2002, Mark K. Gardner
-- All rights reserved.
*/

#ifndef __STATS_H
#define __STATS_H

/* Define some convenient types for module parameterization purposes. */

#ifndef uint32
typedef unsigned int        uint32;
#endif /* uint32 */

#ifndef int32
typedef int                 int32;
#endif /* int32 */

#ifndef uint64
typedef unsigned long long  uint64;
#endif /* uint64 */

#ifndef int64
typedef long long           int64;
#endif /* int64 */

/* Give a default module parameterization. */
#ifndef STATS_DATATYPE
#	define STATS_DATATYPE double
#endif /* STATS_DATATYPE */

/* Data structure to hold intermediate results. */
typedef struct stat_data {
	uint32          count;
	STATS_DATATYPE  min;
	STATS_DATATYPE  max;
	double          mean;
	double          variance;
} stats_data;


/*
-- Public interface of the module:
--
--  If INLINE is not defined, stats.c needs to be compiled and linked in.
--  Otherwise, stats.c is included inline automatically.
*/
#ifndef  INLINE
#	undef  INLINE
#	define INLINE
	extern void            stats_init(stats_data *data);
	extern void            stats_update(stats_data *data, STATS_DATATYPE x);
	extern uint32          stats_count(stats_data *data);
	extern STATS_DATATYPE  stats_min(stats_data *data);
	extern STATS_DATATYPE  stats_max(stats_data *data);
	extern double          stats_mean(stats_data *data);
	extern double          stats_variance(stats_data *data);
	extern double          stats_stdev(stats_data *data);
	extern double          stats_stderr(stats_data *data);
	extern double          stats_confidence(stats_data *data, double level);
#else
#	undef  INLINE
#	define INLINE inline
#	include "stats.c"
#endif /* INLINE */

#endif /* __STATS_H */
