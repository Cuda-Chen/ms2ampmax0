#include "min_max.h"
#include <stdio.h>

void
getMinMaxAndDemean (double *data, uint64_t dataSize,
                    double *min, double *max,
                    double *minDemean, double *maxDemean, double mean)
{
  *min = *max = data[0];
  uint64_t i;
  for (i = 1; i < dataSize; i++)
  {
    if (*min > data[i])
    {
      *min = data[i];
    }
    if (*max < data[i])
    {
      *max = data[i];
    }
  }

#ifdef DEBUG
  printf ("min: %lf, max: %lf\n", *min, *max);
#endif

  *minDemean = *min - mean;
  *maxDemean = *max - mean;

#ifdef DEBUG
  printf ("minDemean: %lf, maxDemean: %lf, mean: %lf\n", *minDemean, *maxDemean, mean);
#endif
}

void
getMinMax (double *data, uint64_t dataSize, double *min, double *max)
{
  *min = *max = data[0];
  uint64_t i;
  for (i = 1; i < dataSize; i++)
  {
    if (*min > data[i])
    {
      *min = data[i];
    }
    if (*max < data[i])
    {
      *max = data[i];
    }
  }
}
