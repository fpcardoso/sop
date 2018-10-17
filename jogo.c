#include <stdio.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//Numero de linhas e colunas
#define nLinhas 32
#define nColunas 82 

char  *entrada[nLinhas];



int adjacente(char **matriz, int i, int j){
 
     int x,y, initX, initY, limitX, limitY;
     int vizinhos = 0;
 
     if(i == 0)
         initX = 0;
     else
         initX = -1;
 
         if(i == (nLinhas-1))
         limitX = 1;
     else
         limitX = 2;
 
 
     if(y == 0)
         initY = 0;
     else
         initY = -1;
 
         if(y == (nColunas-3))
         limitY = 1;
     else
         limitY = 2;
 
     for(x = initX ; x < limitX; x++){
         for(y = initY; y < limitY; y++){
             if(matriz[i+x][j+y] == '#')
                 vizinhos++;
         }
     }
 
 
     if(matriz[i][j] == '#')
         return (vizinhos-1);
     else
         return vizinhos;

}


void calculoGeracao(char **matriz, int ger) {
 
     int i, j, a;
     char novaGeracao[nLinhas][nColunas];
 
     /* Aplicando as regras do jogo da vida */
     for (i=0; i < nLinhas; i++){
         for (j=0; j < nColunas; j++) {
 
             a = adjacente(matriz, i, j);
 
             if (a == 2) novaGeracao[i][j] = matriz[i][j];
             if (a == 3) novaGeracao[i][j] = '#';
             if (a < 2)  novaGeracao[i][j] = ' ';
             if (a > 3)  novaGeracao[i][j] = ' ';
 
             if (j == 0)
                 novaGeracao[i][j] = '"';
                 }
 
         novaGeracao[i][nColunas-3] = '"';
         novaGeracao[i][nColunas-1] = '\n';
 

    }
	char string[nColunas*nLinhas];
	
	strcpy(string,novaGeracao[0]);

	int li = 0;
	int col = 0;
	for(int k = 0;k < (nLinhas*nColunas);k++){
		if(string[k] == '\n'){		
			col = 0;
			li++;
		}	
		else{
			matriz[li][col] = string[k];
			col++;
		}
	}		

    /* Passando o resultado da nova geração para a matriz de entrada */
//    for (i=0; i < nLinhas; i++){
  //      for (j=0; j < nColunas; j++)
    //        matriz[i][j] = novaGeracao[i][j];
   // }
}




int main(int argc,char **argv){
	int rank, size;
    int tag, destination, count;
    int buffer; //value to send

    tag = 1234;
    destination = 1; //destination process
    count = 1; //number of elements in buffer    

    MPI_Request request,request2;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size); //number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //rank of current process

    if (destination >= size) {
        MPI_Abort(MPI_COMM_WORLD, 1); // destination process must be under the number of processes created, otherwise abort
    }

    if (rank == 0) {
       
      	char  *matriz[nLinhas];
		int i;
    	
		//Alocar espaço em memória para as matrizes
     	for (i = 0; i < nLinhas; i++ )
        	matriz[i] = (char *) malloc((nColunas) * sizeof( char ));
				
		FILE *matrizEntrada;
		matrizEntrada = fopen(argv[1], "r");
     
     	if (matrizEntrada == NULL)
        	printf ("Não posso abrir o arquivo \"%s\"\n", argv[1]);
         
     	char str[nColunas];
      	int linha = 0;

        // Lendo o estado inicial do jogo a partir do arquivo
	 	while((fgets(str, nColunas, matrizEntrada) != NULL)&&(linha < nLinhas)){
  	 	 	strcpy(matriz[linha], str);
    	   	linha++;    
    	}
   		 
		printf("%c[2J",27);  // Esta linha serve para limpar a tela antes de imprimir o resultado de uma geração    
 
       	//Lendo o estado do jogo e imprime na tela 
		for(i = 0; i < nLinhas; i++)
	    	printf("%s", matriz[i]);
		usleep(500000);

		int gens = 0;
		while(gens < atoi(argv[2])/2){
			for(int j = 0;j < nLinhas;j++){		
	        	MPI_Isend(matriz[j],nColunas , MPI_CHAR, destination, tag, MPI_COMM_WORLD,&request);
       			MPI_Wait(&request, &status);
			}				
			for(int k = 0;k < nLinhas;k++){
				MPI_Irecv(matriz[k], nColunas, MPI_CHAR, destination, tag,MPI_COMM_WORLD , &request2); 
			}
			MPI_Wait(&request2, &status); //bloks and waits for destination process to receive data
			
			calculoGeracao(matriz,1);
			printf("%c[2J",27);  // Esta linha serve para limpar a tela antes de imprimir o resultado de uma geração    
 
         	//Lendo o estado do jogo e imprime na tela 
			for(i = 0; i < nLinhas; i++)
	    		printf("%s\n", matriz[i]);
			printf("\nGeração:%d, Processo:%d\n",gens,rank);
			usleep(500000);
			gens++;
			
		}
		for(int i = 0; i < nLinhas; i++)		
			free(matriz[i]);
		
    }
	
    if (rank == destination){

		char  *matriz2[nLinhas];
        int i;
		 
        //Alocar espaço em memória para as matrizes
        for (i = 0; i < nLinhas; i++ )
        	matriz2[i] = (char *) malloc((nColunas) * sizeof( char ));

		int gen = 0;
				
		while(gen < atoi(argv[2])/2){
			for(int k = 0;k < nLinhas;k++){
				MPI_Irecv(matriz2[k], nColunas, MPI_CHAR, 0, tag,MPI_COMM_WORLD , &request); 
			}
			MPI_Wait(&request, &status); //bloks and waits for destination process to receive data
			
			calculoGeracao(matriz2,1);

				
	
	 	    printf("%c[2J",27);  // Esta linha serve para limpar a tela antes de imprimir o resultado de uma geração    
    
	     	//Lendo o estado do jogo e imprime na tela 
 	      	for(i = 0; i < nLinhas; i++)
		    	printf("%s\n", matriz2[i]);
			printf("\nGeração:%d, Processo:%d\n",gen,rank);
			usleep(500000);
			gen++;
			for(int j = 0;j < nLinhas;j++){		
	        	MPI_Isend(matriz2[j],nColunas , MPI_CHAR, 0, tag, MPI_COMM_WORLD,&request2);
       			MPI_Wait(&request2, &status); 
			}
					
			
		}
		for(int i = 0; i < nLinhas; i++)		
			free(matriz2[i]);
    }

	MPI_Finalize();

	return 0;
}
