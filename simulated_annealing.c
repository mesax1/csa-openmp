/*
* COMPILE: g++ -g -Wall -fopenmp -o simulated_annealing simulated_annealing.cpp
* RUN: ./simulated_annealing
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>
#include "CSA_Problem.h"

#ifndef PI
    #define PI 3.14159265358979323846264338327
#endif

double maxValue(double myArray[], size_t size) {
    assert(myArray && size);
    size_t i;
    int maxValue = myArray[0];

    for (i = 1; i < size; ++i) {
        if ( myArray[i] > maxValue ) {
            maxValue = myArray[i];
        }
    }
    return maxValue;
}


/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

    double t_gen = 100; // temperatura de geracao
    double t_ac = 100; // temperatura de aceitacao
    double num_aleatorio = 0.0; // numero aleatorio
    double custo_sol_corrente, custo_sol_nova; // energias corrente e nova
    unsigned int dim; // dimensao do problema
    int k = 1;
    int num_otimizadores; // numero de threads/num_otimizadores
    int num_function_choices[3] = {2001,2003,2006}; // opcoes de funcao
    int num_function; // funcao selecionada
    int avaliacoes = 0;

    /* atribuicoes iniciais */
    num_otimizadores = atoi(argv[1]); // pega a numero de threads do programa
    dim = atoi(argv[2]); // pega a dimensao do arg da linha de comando
    num_function = num_function_choices[atoi(argv[3])]; // pega a funcao a ser usada

    double var_desejada = 0.99 * ((num_otimizadores - 1)/num_otimizadores); // calculo da variancia desejada
    double max_sol;
    double *sol_corrente = (double *)malloc(dim * sizeof(double)); // solucao corrente
    double *sol_nova = (double *)malloc(dim * sizeof(double)); // solucao nova
    double *tmp =NULL; // usado para trocar valores
    double *atuais_solucoes = (double *)malloc(num_otimizadores * sizeof(double));
    double termo_acoplamento = 0;

    /* inicia regiao paralela */
    # pragma omp parallel num_threads(num_otimizadores) \
    default(none) \
    shared(termo_acoplamento, avaliacoes, t_gen, t_ac, num_otimizadores, dim, num_function, var_desejada, atuais_solucoes, max_sol) \
    firstprivate(num_aleatorio, sol_corrente, sol_nova) \
    private(custo_sol_corrente, custo_sol_nova, tmp)
    {
        int my_rank = omp_get_thread_num(); // rank da thread/otimizador
        struct drand48_data buffer; // semente

        srand48_r(time(NULL),&buffer); // Gera semente

        //Gera soluções iniciais
       	for (unsigned int i = 0; i < dim; i++){
       		drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
    		sol_corrente[i] = 2.0*num_aleatorio-1.0;
    	}

        custo_sol_corrente = CSA_EvalCost(sol_corrente, dim, num_function); // nova energia corrente
        
        # pragma omp critical
        {
            avaliacoes++;
        }

        atuais_solucoes[my_rank] = custo_sol_corrente; // coloca a atual energia num vetor auxiliar de energias

        # pragma omp barrier // sincronizacao

        // calculo do termo de acoplamento
        # pragma omp single
        {
            int i;
            max_sol = maxValue(atuais_solucoes, num_otimizadores); // maior/pior solucao atual

            for (i = 0; i < num_otimizadores; i++) {
                termo_acoplamento += pow(E, ((atuais_solucoes[i] - max_sol)/t_ac));
            }
        }

        /*
            Geracoes de Yi a partir de Xi
            O laco so termina quando a funcao for avaliada 1 KK de vezes
        */
        while(avaliacoes > 0){
            // gera a nova solucao a partir da corrente
            for (size_t i = 0; i < num_otimizadores; i++) {
                drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
                sol_nova[i] = fmod((sol_corrente[i] + t_gen * tan(PI*(num_aleatorio-0.5))), 1.0);
            }

            // nova energia
            custo_sol_nova = CSA_EvalCost(sol_nova, dim, num_function);

            // incrementa +1 avaliacao na funcao objetivo
            # pragma omp critical
            {
                avaliacoes++;
            }
        
            drand48_r(&buffer, &num_aleatorio); // novo numero aleatorio entre 0 e 1
            double func_prob = pow(( ), E);

            // avaliacao dos atuais custos/energias
            if (custo_sol_nova < custo_sol_corrente){
                custo_sol_corrente = custo_sol_nova;
                atuais_solucoes[my_rank] = custo_sol_corrente;
            } else if (){

            }

        }
    






    }

    //
    //
    //
    //
	// custo_sol_corrente = f(sol_corrente,dim);
	// custo_sol_melhor = custo_sol_corrente;
	// //printf("%1.2e\n", custo_sol_corrente);
    //
	// //Loop Principal - Critério de Parada
	// for (int i = 0; i<1000000; i++){
	// 	//Gerar nova solução
	// 	for (int j = 0; j<dim; j++){
	//    		drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
	// 		sol_nova[j] = fmod(  sol_corrente[j]+ temperatura*tan(PI*(num_aleatorio-0.5)) ,1.0 );
	// 		//printf("%1.2e\n",sol_nova[j] );
	// 	}
	// 	custo_sol_nova = f(sol_nova,dim);
	// 	//Avaliar nova solução
	// 	if (custo_sol_nova < custo_sol_corrente){
	// 		//copio sol_nova para sol_corrente
	// 		tmp = sol_corrente;
	// 		sol_corrente =  sol_nova;
	// 		sol_nova = tmp;
	// 		custo_sol_corrente = custo_sol_nova;
	// 		if (custo_sol_nova < custo_sol_melhor){
	// 			custo_sol_melhor = custo_sol_nova;
	// 			//printf("TESTE: melhor custo: %1.2e\n",custo_sol_melhor);
	// 		}
	// 	}
	// 	else{
	// 		drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
	// 		if (  exp(-1.0*(custo_sol_nova - custo_sol_corrente )/temperatura) > num_aleatorio ){
	// 			tmp = sol_corrente;
	// 			sol_corrente =  sol_nova;
	// 			sol_nova = tmp;
	// 			custo_sol_corrente = custo_sol_nova;
	// 		}
    //
	// 	}
    //
	// 	//escalono a temperatura
	// 	temperatura = 0.99991*temperatura;
    //
	// }
    //
	// printf("\nA melhor solução tem custo: %1.2e\n",custo_sol_melhor);
    //
    //
   return 0;
}  /* main */
