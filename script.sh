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

rm ~/csa-openmp/runtimes/tempo_de_exec_csa.txt
touch ~/csa-openmp/runtimes/tempo_de_exec_csa.txt

#Compila o código
gcc -g -Wall -fopenmp -o ~/csa-openmp/simulated_annealing ~/csa-openmp/src/simulated_annealing.c ~/csa-openmp/src/CSA_Problem.c -lm

#Loop principal de execuções. São 4 tentativas

tentativas=15 #Quantas vezes o código será executado

for function in 0 1 2 #numero da funcao a ser utilizada 0 = 2001, 1 = 2003, 2 = 2006
do
	echo  -e "========== FUNCAO $function ==========" >> "/home/vgdmenezes/csa-openmp/runtimes/tempo_de_exec_csa.txt"

	for dimensao in 5 10 50 100 #tamanho do problema
	do
		echo -e "DIMENSAO $dimensao" >> "/home/vgdmenezes/csa-openmp/runtimes/tempo_de_exec_csa.txt"

		for thread in 2 4 8 16 32 #números de threads utilizadas
		do
			echo -e "THREAD $thread" >> "/home/vgdmenezes/csa-openmp/runtimes/tempo_de_exec_csa.txt"
			for tentativa in $(seq $tentativas)
			do
				echo -e `/home/vgdmenezes/csa-openmp/simulated_annealing $thread $dimensao $function`
			done
			echo -e " " >> "/home/vgdmenezes/csa-openmp/runtimes/tempo_de_exec_csa.txt"
		done
	done


	echo -e "\n\n\n" >> "/home/vgdmenezes/csa-openmp/runtimes/tempo_de_exec_csa.txt"
done
exit
