#include <stdio.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

//jogo da vida


//Numero de linhas e colunas
#define nLinhas 32
#define nColunas 82 

int adjacente(char **matriz, int i, int j){
 
	int x,y, initX, initY, limitX, limitY;
	int vizinhos = 0;

	if(i == 0) initX = 0;
	else initX = -1;

	if(i == (nLinhas-1)) limitX = 1;
	else limitX = 2;
   
	if(y == 0) initY = 0;
	else initY = -1;

    if(y == (nColunas-3)) limitY = 1;
	else limitY = 2;

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

        	if (j == 0) novaGeracao[i][j] = '"';
        }
		novaGeracao[i][nColunas-3] = '"';
    	novaGeracao[i][nColunas-1] = '\n';
	}
	//Correção do código comentado abaixo
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

    /* Passando o resultado da nova geração para a matriz de entrada 
	    for (i=0; i < nLinhas; i++){
  	      for (j=0; j < nColunas; j++)
          	matriz[i][j] = novaGeracao[i][j];
 	}*/
}
		

int main(int argc,char **argv){
	int rank, size; 
    int tag, destino,origem;

    tag = 1234; 	//tag da comunicação
	origem = 0; 	//processo de origem (responsável por fazer a leitura do arquivo inicial)
    destino = 1;    //processo de destino    
	
	/* Inicialização do MPI */
    MPI_Request request,request2;	//recebe resultado quando processo receber mensagem 	
    MPI_Status status;				//recebe status da comunicação
    MPI_Init(&argc, &argv);			//inicia processos conforme quantidade passada por parametro

    MPI_Comm_size(MPI_COMM_WORLD, &size); //size recebe o número de processos iniciado
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //rank recebe o número (identificador) do processo 

    if (destino >= size) {
        MPI_Abort(MPI_COMM_WORLD, 1); //verifica se existem somente 2 processos 
    }

	/* Trecho do código que será executado pelo processo de origem (processo 0) */
    if (rank == origem) {
       
      	char  *matriz[nLinhas];
		int i;
    	
		//Alocar espaço em memória para a matriz que armazenara os dados para o processo 0
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
 
       	//Imprimindo o estado do jogo após leitura do arquivo
		for(i = 0; i < nLinhas; i++)
	    	printf("%s", matriz[i]);
		usleep(400000); //tempo para visualização

		int gens = 2;  //indica geração do processo 0
		
		/* Loop do processo 0*/
		while(gens <= atoi(argv[2])){      

			/* trecho que envia a matriz do jogo para o processo 1 linha po linha */
			for(int j = 0;j < nLinhas;j++){		
	        	MPI_Isend(matriz[j],nColunas , MPI_CHAR, destino, tag, MPI_COMM_WORLD,&request);  //send  não bloqueante, necessário wait para aguardar processo 1 receber
       			MPI_Wait(&request, &status);  //aguarda processo 1 receber linha antes de enviar a próxima
			}				
			
			/* trecho que recebe linha por linha da matriz do jogo enviada pelo processo 1 */
			for(int k = 0;k < nLinhas;k++){
				MPI_Irecv(matriz[k], nColunas, MPI_CHAR, destino, tag,MPI_COMM_WORLD , &request2); //recv não bloqueante, avisa na variavel request2 quando receber
			}
			MPI_Wait(&request2, &status); //-----
			


			calculoGeracao(matriz,1); //calcula geração
	
			printf("%c[2J",27);  // Esta linha serve para limpar a tela antes de imprimir o resultado de uma geração    
 
         	//Imprime estado do jogo após cada calculo de geração
			for(i = 0; i < nLinhas; i++)
	    		printf("%s\n", matriz[i]);
			printf("\nGeração:%d, Processo:%d\n",gens,rank);    //imprime numero da geração e identificador processo que a gerou
			usleep(400000);  
			gens += 2;
			
		}

		//limpa da memória matriz do processo 0 
		for(int i = 0; i < nLinhas; i++)		
			free(matriz[i]);
		
    }
	
    
	/* Trecho do código que será executado pelo processo de destino (processo 1) */
	if (rank == destino){

		char  *matriz2[nLinhas];
        int i;
		 
        //Alocar espaço em memória para as matrizes
        for (i = 0; i < nLinhas; i++ )
        	matriz2[i] = (char *) malloc((nColunas) * sizeof( char ));

		int gen = 1; // indica geração do processo 1
		
		/* loop do processo 1 */
		while(gen <= atoi(argv[2])){

			/* trecho que recebe a matriz do jogo enviada pelo processo 0 linha por linha*/
			for(int k = 0;k < nLinhas;k++){
				MPI_Irecv(matriz2[k], nColunas, MPI_CHAR, origem, tag,MPI_COMM_WORLD , &request);   //recv não bloqueante, informa através de request quando receber  
			}
			MPI_Wait(&request, &status); //------------------
			
			calculoGeracao(matriz2,1);  //calcula geração 

				
	
	 	    printf("%c[2J",27);  // Esta linha serve para limpar a tela antes de imprimir o resultado de uma geração    
    
	     	//Imprime estado do jogo após calcular geração
 	      	for(i = 0; i < nLinhas; i++)
		    	printf("%s\n", matriz2[i]);
			printf("\nGeração:%d, Processo:%d\n",gen,rank);
			usleep(500000);
			gen += 2;

			/* trecho que envia matriz do jogo para processo 0 linha por linha após calcular sua geração e imprimir na tela */
			for(int j = 0;j < nLinhas;j++){		
	        	MPI_Isend(matriz2[j],nColunas , MPI_CHAR, origem, tag, MPI_COMM_WORLD,&request2);  // send não bloqueante, utiliza wait para esperar processos 0 receber antes de enviar a linha seguinte 
       			MPI_Wait(&request2, &status);  //espera processo 0 receber linha antes de enviar outra
			}
					
			
		}
		
		// limpa matriz da memória
		for(int i = 0; i < nLinhas; i++)		
			free(matriz2[i]);
    }

	MPI_Finalize();  // finaliza processo iniciados

	return 0;
}
