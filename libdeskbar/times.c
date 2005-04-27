#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include "times.h"
#include "log.h"

static time_t tm;

static double update_delta	= 0,
							update_diff		= 0,
							update_time		= 0,
							update_last		= 0;

double 
db_time_current (void)
{
  struct timeval tv;
	
  gettimeofday (&tv, 0);
	
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

double
db_time_delta (void)
{
	return (update_delta);
}

double
db_time_update_time (void)
{
	return (update_time);
}

double
db_time_update_diff (void)
{
	return (update_diff);
}

void
db_time_update (void)
{
	update_time		= db_time_current ();
	update_delta	= (update_time - update_last);
	update_diff		= (1 - update_delta);
	update_last		= update_time;
}

struct tm *
db_time_local (void)
{
  tm  = time (NULL);

  return (localtime (&tm));
}	
