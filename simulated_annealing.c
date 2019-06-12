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
    int i, j = 0;
    double minValue = myArray[0];

    for (i = 1; i < size; i++) {
        if ( myArray[i] < minValue ) {
            minValue = myArray[i];
            j = i;
        }
    }
    return j;
}

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

    double t_gen = 100; // temperatura de geracao
    double t_ac = 100; // temperatura de aceitacao
    double num_aleatorio = 0.0; // numero aleatorio
    double custo_sol_corrente, custo_sol_nova, melhor_custo; // energias corrente e nova
    int dim; // dimensao do problema
    int k = 1;
    int i;
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
    double *sol_corrente; // solucao corrente
    double *sol_nova; // solucao nova
    double *melhor_solucao; // solucao nova
    double *tmp = NULL; // usado para trocar valores
    double *vetor_func_prob = (double *)malloc(num_otimizadores * sizeof(double)); // solucao nova
    double *atuais_custos = (double *)malloc(num_otimizadores * sizeof(double));
    double termo_acoplamento;
    double sigma;
    struct drand48_data buffer; // semente

    /* --------------------  inicia regiao paralela ------------------------- */
    # pragma omp parallel num_threads((int)num_otimizadores) \
    default(none) \
    shared(vetor_func_prob, sigma, menor_custo_total, k, termo_acoplamento, avaliacoes, t_gen, t_ac, atuais_custos, num_otimizadores, dim, num_function, var_desejada) \
    private(num_aleatorio, tmp, buffer, melhor_custo, custo_sol_corrente, sol_corrente, sol_nova, custo_sol_nova, melhor_solucao, i)
    {
        sol_corrente = (double *)malloc(dim * sizeof(double));
        sol_nova = (double *)malloc(dim * sizeof(double));
        melhor_solucao = (double *)malloc(dim * sizeof(double));

        int my_rank = omp_get_thread_num(); // rank da thread/otimizador
        srand48_r(time(NULL)+my_rank*my_rank,&buffer); // Gera semente

        //Gera soluções iniciais
        for (i = 0; i < dim; i++){
            drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
            sol_corrente[i] = 2.0*num_aleatorio-1.0;
            if (sol_corrente[i] < -1.0 || sol_corrente[i] > 1.0) {
                printf("Erro no limite da primeira solucao corrente: \n");
                exit(0);
            }
        }

        custo_sol_corrente = CSA_EvalCost(sol_corrente, dim, num_function); // nova energia corrente

        // incrementa +1 avaliacao na funcao objetivo
        # pragma omp atomic
            avaliacoes++;

        // coloca a atual energia num vetor auxiliar de custos/energias
        atuais_custos[my_rank] = custo_sol_corrente;

        // melhor custo do otimizador, de inicio, eh o primeiro
        melhor_custo = custo_sol_corrente;

        // sincronizacao
        # pragma omp barrier

        // calculo do termo de acoplamento
        # pragma omp single private(i)
        {
            termo_acoplamento = 0;
            for (i = 0; i < (int) num_otimizadores; i++) {
                termo_acoplamento += pow(EULLER, ((atuais_custos[i] - maxValue(atuais_custos, (int)num_otimizadores))/t_ac));
            }
        }

        # pragma omp barrier

        double func_prob = pow(EULLER, ((custo_sol_corrente - maxValue(atuais_custos, (int)num_otimizadores))/t_ac))/termo_acoplamento; // funcao de probabilidade de aceitacao
        printf("Thread (%d) --- func_prob = %1.2e\n", my_rank, func_prob);
        if (func_prob < 0 || func_prob > 1){
            printf("Limite errado da função de probabilidade\n");
            exit(0);
        }
        vetor_func_prob[my_rank] = func_prob;

        /*
            Geracoes de Yi a partir de Xi
            O laco so termina quando a funcao for avaliada 1 KK de vezes
        */
        while(avaliacoes < 1000000){
            // gera a nova solucao a partir da corrente
            for (i = 0; i < (int) num_otimizadores; i++) {
                drand48_r(&buffer, &num_aleatorio); //gera um número entre 0 e 1
                sol_nova[i] = fmod((sol_corrente[i] + t_gen * tan(PI*(num_aleatorio-0.5))), 1.0);
                if (sol_nova[i] > 1.0 || sol_nova[i] < -1.0) {
                    printf("Intervalo de soluções mal definido!\n");
                    exit(0);
                }
            }

            // nova energia
            custo_sol_nova = CSA_EvalCost(sol_nova, dim, num_function);

            // incrementa +1 avaliacao na funcao objetivo
            # pragma omp critical
            {
                avaliacoes++;
            }

            drand48_r(&buffer, &num_aleatorio); // novo numero aleatorio entre 0 e 1
            if (num_aleatorio > 1 || num_aleatorio < 0) {
                printf("ERRO NO LIMITE DE R = %f\n", num_aleatorio);
                exit(0);
            }

            // avaliacao dos atuais custos/energias
            if (custo_sol_nova <= custo_sol_corrente || func_prob > num_aleatorio){
                tmp = sol_corrente;
        		sol_corrente = sol_nova;
        		sol_nova = tmp;
                custo_sol_corrente = custo_sol_nova;
                atuais_custos[my_rank] = custo_sol_nova;

                if (melhor_custo > custo_sol_nova) {
                    melhor_custo = custo_sol_nova;
                    melhor_solucao = sol_corrente;
                }
            }

            // sincronizacao
            # pragma omp barrier

            # pragma omp single private(i)
            {
                double soma = 0;
                for (i = 0; i < num_otimizadores; i++) {
                    soma += vetor_func_prob[i];
                }
                // printf("SOMA DO VETOR DE FUNC = %f\n", soma);

                // calculo do termo de acoplamento
                termo_acoplamento = 0;
                for (i = 0; i < (int) num_otimizadores; i++) {
                    termo_acoplamento += pow(EULLER, ((atuais_custos[i] - maxValue(atuais_custos, (int) num_otimizadores))/t_ac));
                }
            }

            // recalcula a funcao de probabilidade
            // printf("CUSTO CORRENTE = %1.2e\tTERMO ACOPL = %1.2e\tT_AC = %1.2e\n", custo_sol_corrente, termo_acoplamento, t_ac);
            func_prob = pow(EULLER, ((custo_sol_corrente - maxValue(atuais_custos, (int) num_otimizadores))/t_ac))/termo_acoplamento;
            if (func_prob < 0 || func_prob > 1){
                printf("Limite errado da função de probabilidade\n");
                exit(0);
            }
            vetor_func_prob[my_rank] = func_prob;

            # pragma omp barrier

            # pragma omp single private(i)
            {
                // calculo da variancia da funcao de probabilidades de aceitacao
                sigma = 0;
                double partial_sigma = 0;
                for (i = 0; i < (int) num_otimizadores; i++) {
                    partial_sigma += (double) pow(vetor_func_prob[i], 2);
                }

                // printf("PARTIAL SIGMA = %1.20e\n", partial_sigma);
                sigma = ((1/num_otimizadores) * partial_sigma) - 1/(num_otimizadores * num_otimizadores);
                // printf("SIGMA = %1.20e\n", sigma);

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


        }

        // melhores custos gerais agora no vetor de atuais custos
        atuais_custos[my_rank] = melhor_custo;

        # pragma omp barrier // sincronizacao

        // recupera o menor custo total
        # pragma omp single private(i)
        {
            menor_custo_total = minValue(atuais_custos, (int) num_otimizadores);
        }

        // o otimizador com o menor custo o imprime
        if (menor_custo_total == my_rank) {
            printf("\n\n\n%f\n",melhor_custo);
            for (i = 0; i < dim; i++) {
                printf("%f ", melhor_solucao[i]);
            }
            printf("\n");
        }
    }

   return 0;
}  /* main */
