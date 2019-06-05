#ifndef __CSA_Problem_h__
#define __CSA_Problem_h__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifndef PI
#define PI 3.14159265358979323846264338327
#endif

#define UPPER_BOUNDBOX 2.0*PI
#define LOWER_BOUNDBOX 0.0

double CSA_EvalCost_CG(double *solution, unsigned int dimension, int functionNumber);

#endif
