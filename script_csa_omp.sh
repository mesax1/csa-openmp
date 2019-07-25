#!/bin/bash
#SBATCH --partition=cluster
#SBATCH --job-name=csa
#SBATCH --output=out_matrix_calc_csa.out
#SBATCH --error=err_matrix_calc_csa.err
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=32
#SBATCH --time=0-1:00
#SBATCH --exclusive
#SBATCH --hint=compute_bound

#just to save the runtimes
#rm ~/csa-openmp/runtimes/tempo_de_exec_csa.txt
#touch ~/csa-openmp/runtimes/tempo_de_exec_csa.txt

#Compila o código
gcc -g -Wall -fopenmp -o simulated_annealing simulated_annealing.c CSA_Problem.c -lm

#Loop principal de execuções. São 4 tentativas

tentativas=15 #Quantas vezes o código será executado

for function in 0 1 2 #numero da funcao a ser utilizada 0 = 2001, 1 = 2003, 2 = 2006
do
	echo  -e "========== FUNCAO $function ==========" >> "" # ome file

	for dimensao in 5 10 50 100 #tamanho do problema
	do
		echo -e "DIMENSAO $dimensao" >> "" # some file

		for thread in 2 4 8 16 32 #números de threads utilizadas
		do
			echo -e "THREAD $thread" >> ""#some file
			for tentativa in $(seq $tentativas)
			do
				echo -e `simulated_annealing $thread $dimensao $function`
			done
			echo -e " " >> "" #some file
		done
	done
	echo -e "\n\n\n" >> "" #some file
done

exit
