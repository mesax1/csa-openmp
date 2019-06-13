/*
* COMPILE: g++ -g -Wall -fopenmp -o simulated_annealing simulated_annealing.cpp
* RUN: ./simulated_annealing
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include "CSA_Problem.h"

#ifndef PI
    #define PI 3.14159265358979323846264338327
#endif

// funcao auxiliar para pegar o maior/pior custo atual de todos os otimizadores
double maxValue(double myArray[], int size) {
    assert(myArray && size);
    int i;
    double maxValue = myArray[0];

    for (i = 1; i < size; ++i) {
        if ( myArray[i] > maxValue ) {
            maxValue = myArray[i];
        }
    }
    return maxValue;
}

double minValue(double myArray[], int size) {
    assert(myArray && size);
    int i;
    double minValue = myArray[0];

    for (i = 1; i < size; i++) {
        if ( myArray[i] < minValue ) {
            minValue = myArray[i];
        }
    }
    return minValue;
}

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {


    double t_gen = 100; // temperatura de geracao
    double t_ac = 100; // temperatura de aceitacao
    double num_aleatorio = 0.0; // numero aleatorio
    int dim; // dimensao do problema
    int k = 1;
    int i, j;
    double num_otimizadores = 0; // numero de threads/num_otimizadores
    int num_function_choices[3] = {2001,2003,2006}; // opcoes de funcao
    int num_function; // funcao selecionada
    int avaliacoes = 0;
    int menor_custo_total;

    /* atribuicoes iniciais */
    num_otimizadores = (double) atoi(argv[1]); // pega a numero de threads do programa
    dim = atoi(argv[2]); // pega a dimensao do arg da linha de comando
    num_function = num_function_choices[atoi(argv[3])]; // pega a funcao a ser usada

    double var_desejada = 0.99 * ((num_otimizadores - 1)/(num_otimizadores * num_otimizadores )); // calculo da variancia desejada
    double *tmp = NULL; // usado para trocar valores
    double termo_acoplamento; // termo de acoplamento
    double sigma;
    double total_time; // tempo total de execucao
    double start; // tempo de inicio da execucao
    double *custo_sol_corrente = (double *)malloc(num_otimizadores * sizeof(double));
    double *custo_sol_nova = (double *)malloc(num_otimizadores * sizeof(double));
    double *melhor_custo = (double *)malloc(num_otimizadores * sizeof(double)); // energias corrente e nova
    double *vetor_func_prob = (double *)malloc(num_otimizadores * sizeof(double)); // solucao nova
    double *atuais_custos = (double *)malloc(num_otimizadores * sizeof(double)); // custos correntes de cada otimizador

    double **sol_correntes = (double **)malloc(num_otimizadores * sizeof(double *)); // solucoes correntes de todos os otimizadores
    double **sol_novas = (double **)malloc(num_otimizadores * sizeof(double *)); // novas solucoes de todos os otimizadores

    for (i = 0; i < (int) num_otimizadores; i++) {
        sol_correntes[i] = (double *)malloc(dim * sizeof(double));
        sol_novas[i] = (double *)malloc(dim * sizeof(double));
    }

    struct drand48_data buffer; // semente
    srand48_r(time(NULL),&buffer);

    //Gera soluções iniciais para cada otimizador
    for (i = 0; i < (int) num_otimizadores; i++){
        for (j = 0; j < dim; j++) {
            drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
            sol_correntes[i][j] = 2.0*num_aleatorio-1.0;
        }
        custo_sol_corrente[i] = CSA_EvalCost(sol_correntes[i], dim, num_function);
        atuais_custos[i] = custo_sol_corrente[i];
        melhor_custo[i] = custo_sol_corrente[i];
        avaliacoes++;
    }

    // for (i = 0; i < num_otimizadores; i++) {
    //     for (j = 0; j < dim; j++) {
    //         printf("%f ", sol_correntes[i][j]);
    //     }
    //     printf("\t\t custo = %f\n", atuais_custos[i]);
    // }

    termo_acoplamento = 0;
    for (i = 0; i < (int) num_otimizadores; i++) {
        termo_acoplamento += pow(EULLER, ((atuais_custos[i] - maxValue(atuais_custos, (int)num_otimizadores))/t_ac));
    }

    printf("%1.20e\n", termo_acoplamento);

    // funcao de probabilidade de aceitacao
    for (i = 0; i < (int) num_otimizadores; i++) {
        double func_prob = pow(EULLER, ((custo_sol_corrente[i] - maxValue(atuais_custos, (int)num_otimizadores))/t_ac))/termo_acoplamento;
        if (func_prob < 0 || func_prob > 1){
            printf("Limite errado da função de probabilidade\n");
            exit(1);
        }
        //adiciona o valor da funcao do otimizador N na sua posicao correspondente no vetor de funcoes
        vetor_func_prob[i] = func_prob;
    }

    double soma = 0;
    for (i = 0; i < num_otimizadores; i++) {
        soma += vetor_func_prob[i];
    }
    printf("%.30f\n", soma);

	//Loop Principal - Critério de Parada
	while (avaliacoes < 1000000){
		//Gerar nova solução
        for (i = 0; i < (int) num_otimizadores; i++){
            for (j = 0; j < dim; j++) {
                drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
                sol_novas[i][j] = fmod(sol_correntes[i][j] + t_gen*tan(PI*(num_aleatorio-0.5)) ,1.0 );
            }
            custo_sol_nova[i] = CSA_EvalCost(sol_correntes[i], dim, num_function);
            avaliacoes++;
        }

		//Avaliar nova solução
        for (i = 0; i < num_otimizadores; i++) {
            drand48_r(&buffer, &num_aleatorio);
            if (num_aleatorio > 1 || num_aleatorio < 0) {
                printf("ERRO NO LIMITE DE R = %f\n", num_aleatorio);
                exit(1);
            }
            if (custo_sol_nova[i] <= custo_sol_corrente[i] || vetor_func_prob[i] > num_aleatorio){
                //copio sol_nova para sol_corrente
                tmp = sol_correntes[i];
                sol_correntes[i] =  sol_novas[i];
                sol_novas[i] = tmp;
                custo_sol_corrente[i] = custo_sol_nova[i];
                atuais_custos[i] = custo_sol_nova[i];

                if (custo_sol_nova[i] < melhor_custo[i]){
                    melhor_custo[i] = custo_sol_nova[i];
                }
            }

        }

        termo_acoplamento = 0;
        for (i = 0; i < (int) num_otimizadores; i++) {
            termo_acoplamento += pow(EULLER, ((atuais_custos[i] - maxValue(atuais_custos, (int)num_otimizadores))/t_ac));
        }

        // funcao de probabilidade de aceitacao
        for (i = 0; i < (int) num_otimizadores; i++) {
            double func_prob = pow(EULLER, ((custo_sol_corrente[i] - maxValue(atuais_custos, (int)num_otimizadores))/t_ac))/termo_acoplamento;
            if (func_prob < 0 || func_prob > 1){
                printf("Limite errado da função de probabilidade\n");
                exit(1);
            }
            //adiciona o valor da funcao do otimizador N na sua posicao correspondente no vetor de funcoes
            vetor_func_prob[i] = func_prob;
        }

            double soma = 0;
            for (i = 0; i < num_otimizadores; i++) {
                soma += vetor_func_prob[i];
            }
            printf("%.30f\n", soma);

        sigma = 0;
        for (i = 0; i < (int) num_otimizadores; i++) {
            sigma += (double) pow(vetor_func_prob[i], 2);
        }
        sigma = ((1/num_otimizadores) * sigma) - 1/(num_otimizadores * num_otimizadores);

        double sigma_limit = ((num_otimizadores - 1)/(num_otimizadores * num_otimizadores));
        if (sigma < 0 || sigma > sigma_limit){
            printf("Limite errado de sigma. sigma = %f, iteracao = %d\n", sigma, k);
            exit(0);
        }

        // avaliacao e atualizacao da temperatura de aceitacao
        if (sigma < var_desejada){
            t_ac = t_ac * (1 - 0.01);
        } else if (sigma > var_desejada){
            t_ac = t_ac * (1 + 0.01);
        }

        // atualizacao da temperatura de geracao
        t_gen = 0.99992 * t_gen;

        // incrementa k
        k++;
	}

    for (i = 0; i < (int) num_otimizadores; i++) {
        atuais_custos[i] = melhor_custo[i];
    }

    double melhor_custo_total = minValue(atuais_custos, (int)num_otimizadores);

	printf("\nA melhor solução tem custo: %.20f\n",melhor_custo_total);

   return 0;
}  /* main */
