#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
/*
	Arquivo de teste que verifica as funções:
		create2()
		open2()
		write2()
		read2()
		close2()
		seek()
		delete2()
*/
int main(){
	char string1[] = "/arq1";
	char buffer1_write[] = "Um texto de teste para escrita dentro do arquivo";
	char* buffer1_read = (char*) malloc(strlen(buffer1_write));
	FILE2 handle1;
	
	/* Cria arquivos */
	if(!create2(string1))
		printf("Arquivo de path %s criado.\n", string1);
	
	/* Abre arquivo */
	handle1 = open2(string1);
	if(handle1 != -1)
		printf("Arquivo de path %s aberto. Handle: %d.\n", string1, handle1);
	else
		printf("Arquivo de path %s nao foi aberto. Handle: %d.\n", string1, handle1);

	/* Escreve no arquivo */
	if(write2(handle1, buffer1_write, strlen(buffer1_write)) > 0)
		printf("Escrita no arquivo de handle %d efetuada.\n", handle1);
	else
		printf("Escrita no arquivo de handle %d nao efetuada.\n", handle1);

	/* Fecha arquivo para atualiza-lo*/
	if(!close2(handle1))
		printf("Arquivo de handle %d fechado.\n", handle1);
	else
		printf("Arquivo de handle %d nao foi fechado.\n", handle1);
	
	/* Abre arquivo para leitura */
	handle1 = open2(string1);
	if(handle1 != -1)
		printf("Arquivo de path %s aberto. Handle: %d.\n", string1, handle1);
	else
		printf("Arquivo de path %s nao foi aberto. Handle: %d.\n", string1, handle1);
	
	/* Reposiciona ponteiro*/
	if(!seek2(handle1, 0))
		printf("Ponteiro do arquivo de handle %d foi reposicionado.\n", handle1);
	else
		printf("Ponteiro do arquivo de handle %d nao foi reposicionado.\n", handle1);

	/* Lê arquivo */
	if(read2(handle1, buffer1_read, strlen(buffer1_write)) > 0)
		printf("Leitura no arquivo de handle %d: %s.\n", handle1, buffer1_read);
	else
		printf("Leitura no arquivo de handle %d nao efetuada.\n", handle1);
	
	/* Reposiciona o ponteiro para a partir do byte 5 do texto */
	if(!seek2(handle1, 5))
		printf("Ponteiro do arquivo de handle %d foi reposicionado.\n", handle1);
	else
		printf("Ponteiro do arquivo de handle %d nao foi reposicionado.\n", handle1);
	
	/* Lê arquivo a partir do byte 5*/
	if(read2(handle1, buffer1_read, strlen(buffer1_write)) > 0)
		printf("Leitura no arquivo de handle %d: %s.\n", handle1, buffer1_read);
	else
		printf("Leitura no arquivo de handle %d nao efetuada.\n", handle1);

	/* Fecha arquivo */
	if(!close2(handle1))
		printf("Arquivo de handle %d fechado.\n", handle1);
	else
		printf("Arquivo de handle %d nao foi fechado.\n", handle1);

	/* Deleta arquivo */
	if(!delete2(string1))
		printf("Arquivo de path %s deletado.\n", string1);

	return 0;
}




