#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main(){
	char name1[30];
	char name2[43];
	char name3[100];
	//A primeira chamada deve retornar erro pois o string é maior que 30
	if(identify2(name1, sizeof(name1)) == -1) 
		puts("[E] - Primeira chamada retornou erro - CORRETO");
	else 
		puts(name1);
	//A segunda chamada deve retornar erro pois o string é maior que 43
	if(identify2(name2, sizeof(name2)) == -1) 
		puts("[E] - Segunda chamada retornou erro - CORRETO");
	else 
		puts(name2);
	//A terceira chamada deve retornar correto pois a string é menos que 100
	if(identify2(name3, sizeof(name3)) == -1) 
		puts("[E] - Terceira chamada retornou erro - ERRADO");
	else 
		puts(name3);
	return 0;
}
