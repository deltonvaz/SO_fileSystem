#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

/*
	Arquivo que verifica a correta criação de arquivos dentro de diretórios, exclusão deles 
	e verifica se o nome de criação está de acordo com o esperado
	Funções testadas:
		mkdir2()
		create2()
		open2()
		close2()
		delete2()	
*/

int main(){
	char string1[] = "/dir1";
	char string2[] = "/dir1/dir2";
	char string3[] = "/dir1/nFile1";
	char string4[] = "/dir1/nFile2";
	char string5[] = "/dir1/dir2/nFile1/";
	
	FILE2 handle1, handle2, handle3;
	
	/* Cria diretórios */
	if(!mkdir2(string1))
		printf("Diretorio de path %s criado.\n", string1);
	if(!mkdir2(string2))
		printf("Diretorio de path %s criado.\n", string2);
		
	/* Cria arquivos */
	if(!create2(string3))
		printf("Arquivo de path %s criado.\n", string3);
	if(!create2(string4))
		printf("Arquivo de path %s criado.\n", string4);
	
	/* Abre arquivos */
	handle1 = open2(string3);
	if(handle1 != -1)
		printf("Arquivo de path %s aberto. Handle: %d.\n", string3, handle1);
	else
		printf("Arquivo de path %s nao foi aberto. Handle: %d.\n", string3, handle1);
		
	handle2 = open2(string4);
	if(handle2 != -1)
		printf("Arquivo de path %s aberto. Handle: %d.\n", string4, handle2);
	else
		printf("Arquivo de path %s nao foi aberto. Handle: %d.\n", string4, handle2);
		
	handle3 = open2(string5);
	if(handle3 != -1)
		printf("Arquivo de path %s aberto. Handle: %d.\n", string5, handle3);
	else
		printf("Arquivo de path %s nao foi aberto. Handle: %d.\n", string5, handle3);
	
	/* Fecha arquivos */
	if(!close2(handle1))
		printf("Arquivo de handle %d fechado.\n", handle1);
	else
		printf("Arquivo de handle %d nao foi fechado.\n", handle1);
		
	if(!close2(handle2))
		printf("Arquivo de handle %d fechado.\n", handle2);
	else
		printf("Arquivo de handle %d nao foi fechado.\n", handle2);
	
	if(!close2(handle3))
		printf("Arquivo de handle %d fechado.\n", handle3);
	else
		printf("Arquivo de handle %d nao foi fechado.\n", handle3);
		
	/* Exclui arquivos*/
	if(!delete2(string4))
		printf("Arquivo de path %s deletado.\n", string4);
	if(!delete2(string3))
		printf("Arquivo de path %s deletado.\n", string3);
	if(!delete2(string5))
		printf("Arquivo de path %s deletado.\n", string5);

	return 0;
}


