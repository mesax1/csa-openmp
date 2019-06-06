#ifndef __CSA_Problem_h__
#define __CSA_Problem_h__

#ifndef PI
#define PI 3.14159265358979323846264338327
#endif

#ifndef E
#define E 2.71828182845904523536028747135
#endif

#define UPPER_BOUNDBOX 2.0*PI
#define LOWER_BOUNDBOX 0.0

double CSA_EvalCost(double *solution, unsigned int dimension, int functionNumber);

#endif
