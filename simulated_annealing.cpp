/*
* COMPILE: g++ -g -Wall -fopenmp -o simulated_annealing simulated_annealing.cpp
* RUN: ./simulated_annealing
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#ifndef PI
    #define PI 3.14159265358979323846264338327
#endif


double f(double *solution, unsigned int dimension){
  double e;
  double solutionScale;
  int i;

      solutionScale = 100.0;
      e = 0.0;
      for (i=0; i<dimension; i++)
        e += pow(solutionScale*solution[i],2.0);
    return e;
  }

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

    double t_gen = 100; // temperatura de geracao
    double t_ac = 100; // temperatura de aceitacao
    int dim; // dimensao do problema
    int num_otimizadores; // numero de threads/num_otimizadores
    double var_desejada; // variancia desejada

    num_otimizadores = argv[1]; // pega a numero de threads do programa
    dim = argv[2]; // pega a dimensao do arg da linha de comando

    // calculo da variancia desejada
    var_desejada = 0.99 * ((num_otimizadores - 1)/num_otimizadores);

    int k = 1;
	double *sol_corrente = (double *)malloc(dim * sizeof(double));
	double *sol_nova = (double *)malloc(dim * sizeof(double));
	double *tmp =NULL;
    double num_aleatorio = 0.0;
	//double melhor_custo;

    # pragma omp parallel num_threads(num_otimizadores) default(none) private(sol_corrente, sol_nova)
    {
        struct drand48_data buffer; // semente
        srand48_r(time(NULL),&buffer); // Gera semente

        //Gera soluções iniciais
       	for (int i = 0; i < dim; i++){
       		drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
    		sol_corrente[i] = 2.0*num_aleatorio-1.0;
    	}

    }




	double custo_sol_corrente, custo_sol_nova, custo_sol_melhor;



	custo_sol_corrente = f(sol_corrente,dim);
	custo_sol_melhor = custo_sol_corrente;
	//printf("%1.2e\n", custo_sol_corrente);

	//Loop Principal - Critério de Parada
	for (int i = 0; i<1000000; i++){
		//Gerar nova solução
		for (int j = 0; j<dim; j++){
	   		drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
			sol_nova[j] = fmod(  sol_corrente[j]+ temperatura*tan(PI*(num_aleatorio-0.5)) ,1.0 );
			//printf("%1.2e\n",sol_nova[j] );
		}
		custo_sol_nova = f(sol_nova,dim);
		//Avaliar nova solução
		if (custo_sol_nova < custo_sol_corrente){
			//copio sol_nova para sol_corrente
			tmp = sol_corrente;
			sol_corrente =  sol_nova;
			sol_nova = tmp;
			custo_sol_corrente = custo_sol_nova;
			if (custo_sol_nova < custo_sol_melhor){
				custo_sol_melhor = custo_sol_nova;
				//printf("TESTE: melhor custo: %1.2e\n",custo_sol_melhor);
			}
		}
		else{
			drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
			if (  exp(-1.0*(custo_sol_nova - custo_sol_corrente )/temperatura) > num_aleatorio ){
				tmp = sol_corrente;
				sol_corrente =  sol_nova;
				sol_nova = tmp;
				custo_sol_corrente = custo_sol_nova;
			}

		}

		//escalono a temperatura
		temperatura = 0.99991*temperatura;

	}

	printf("\nA melhor solução tem custo: %1.2e\n",custo_sol_melhor);


   return 0;
}  /* main */
