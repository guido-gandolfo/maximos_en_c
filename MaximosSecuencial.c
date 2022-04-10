//N -> Largo del arreglo	T -> Hilos por máquina

#include <stdlib.h>
#include <stdio.h>
#define MAX_RANGE 1000000

int *arreglo, *numeros, N;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}


int main(int argc, char* argv[]){
	//Variables	
	int i, j;
	
	double timetick;
	
	N = atoi(argv[1]);
	
	numeros=(int*)malloc(sizeof(int)*MAX_RANGE*2);	
	arreglo=(int*)malloc(sizeof(int)*N);
	srand (time (NULL));
	for (i = 0;i < N; i++) {
		arreglo[i] = rand() % MAX_RANGE;
	}
	for (i = 0; i < MAX_RANGE; i++){
		numeros[i] = 0;						//Fila 1 tiene cantidades
		numeros[MAX_RANGE+i] = i;			//Fila 2 tiene números
	}
	
	timetick = dwalltime();
	
	//Cuento los elementos en el arreglo
	for (i = 0; i < N; i++) {
		numeros[arreglo[i]]++;
	}
	
	//Mergesort de los resultados
	int largo = MAX_RANGE;
	int tam = 1; //Comienzo desde abajo
	int izq, med, der;
	while (tam <= largo) {
		izq = 0;
		while (izq + tam <= largo) {
			med = izq + tam - 1;
			der = med + tam;
			if (der >= largo) der = largo-1;
			merge(izq, med, der);
			izq = der + 1;
		}
		tam *= 2;
	}
	
	int* maxLocal = (int*)malloc(sizeof(int)*100*2);
	for (i = 0; i < 100; i++) {
		maxLocal[i] = numeros[i];				//Fila 1 tiene cantidades
		maxLocal[100+i] = numeros[MAX_RANGE+i];	//Fila 2 tiene numeros
	}
	
	printf("Tiempo en segundos %f\n", dwalltime() - timetick);
	
	for (i = 0; i < 100; i++) {
		printf("Número : %d   Cantidad : %d \n",maxLocal[100+i],maxLocal[i]);
	}
	/*
	//Busco los 100 elementos más comunes
	int maximo = -1;
	int pos;
	int anterior = MAX_RANGE + 1;
	printf("Los 100 números más comunes son: ");
	printf("\n");
	for (j = 0; j < 100; j++) {
		for (i = 0; i < MAX_RANGE; i++) {
			if (numeros[i] > maximo) {
				maximo = numeros[i];
				pos = i;
			}
		}		
		printf("Número : %d   Cantidad : %d \n",pos,maximo);
		maximo = -1;
		numeros[pos] = 0;
	}
	*/
	
	

	return(0);
}

void merge (int izq, int med, int der) {
	int i = 0, j = 0, k = 0;
	int largoI = med - izq + 1;	
	int largoD = der - med;
	
	//Declaro matrices temporales
	int *arreglo_izq, *arreglo_der;
	arreglo_izq=(int*)malloc(sizeof(int)*largoI*2);
	arreglo_der=(int*)malloc(sizeof(int)*largoD*2);
	
	//Copio valores a las matrices temporales
	for (i = 0; i < largoI; i++) {
		arreglo_izq[i] = numeros[izq + i];						//Copio fila 1
		arreglo_izq[largoI + i] = numeros[izq + i + MAX_RANGE];	//Copio fila 2
	}
	for (i = 0; i < largoD; i++) {
		arreglo_der[i] = numeros[med + i + 1];
		arreglo_der[largoD + i] = numeros[med + i + 1 +MAX_RANGE];
	}
	
	i = 0;
	int aux;
	
	//Merge
	while ((i < largoI) && (j < largoD)) {
		if (arreglo_izq[i] >= arreglo_der[j]) {			
			numeros[izq + k] = arreglo_izq[i];
			numeros[izq + k + MAX_RANGE] = arreglo_izq[i + largoI];
			
			i++;
		}
		else {			
			numeros[izq + k] = arreglo_der[j];
			numeros[izq + k + MAX_RANGE] = arreglo_der[j + largoD];
			j++;
		}
		k++;
	}
	
	//Copio valores restantes
	while (i < largoI) {
		numeros[izq + k] = arreglo_izq[i];
		numeros[izq + k + MAX_RANGE] = arreglo_izq[i + largoI];
		i++;
		k++;
	}
	while (j < largoD) {
		numeros[izq + k] = arreglo_der[j];
		numeros[izq + k + MAX_RANGE] = arreglo_der[j + largoD];
		j++;
		k++;
	}
	
	free(arreglo_izq);
	free(arreglo_der);
}









//513165165
