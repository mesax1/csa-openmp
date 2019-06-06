#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "CSA_Problem.h"
/**
  \file CSA_Problem.cpp
  \brief Source code for test problems
*/

/** \brief Collection of global optimization test problems.

	This function evaluate the cost of a collection of test problems
	for global optimization.
	\param *solution a pointer to a vector of doubles of size \c dimension
		containing the solution to be evaluated. It has to be numbers [-1,1]
	\param dimension an integer number with the dimension of the problem.
	\param functionNumber an integer number identifyiong the test problem.
		Currently possibilities are:
		- 2001: Sphere function
		- 2003: Ackley's function
		- 2006: Rastringin's function
*/
double CSA_EvalCost(double *solution, unsigned int dimension, int functionNumber){
  double e;
  double solutionScale, tmpvar1, tmpvar2;
  int i;

  switch(functionNumber){
    case 2001: // Sphere function
      solutionScale = 100.0;
      e = 0.0;
      for (i=0; i<dimension; i++)
        e += pow(solutionScale*solution[i],2.0);
    break;
    case 2003: // Ackley's function
      solutionScale = 32.768;
      tmpvar1 = 0.0;
      for (i=0; i<dimension; i++)
        tmpvar1 += pow(solutionScale*solution[i],2.0);
      tmpvar2 = 0.0;
      for (i=0; i<dimension; i++)
        tmpvar2 += cos(2.0*PI*solutionScale*solution[i]);
      e = -20.0*exp(-0.2*sqrt(tmpvar1/( (double)dimension ) ) ) - exp( tmpvar2/( (double)dimension ) )  + 20.0 + exp(1.0);
    break;
    case 2006: // Rastringin's function
      solutionScale = 5.12;
      e = 0.0;
      for (i=0; i<dimension; i++)
        e += pow(solutionScale*solution[i],2.0) - 10.0*cos(2.0*PI*solutionScale*solution[i]) + 10.0;
    break;
    default:
      e = HUGE_VAL;
  }
  return e;
}
