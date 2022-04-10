//N -> Largo del arreglo	T -> Hilos por máquina

#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <stdio.h>

#define MAX_RANGE 1000000

void merge (int izq, int med, int der);
void merge_secciones(int hilos, int secciones_agregadas);


pthread_mutex_t Lock1;
pthread_mutex_t Lock2;



int *arreglo, *arreglo_local, *numeros, *posiciones, N, T, Nlocal, rank;

//Para calcular tiempo
double dwalltime(){
        double sec;
        struct timeval tv;

        gettimeofday(&tv,NULL);
        sec = tv.tv_sec + tv.tv_usec/1000000.0;
        return sec;
}

//Hilos
void* sumar(void *arg){
	int tid=*(int*)arg;	
	int inicio = tid * (Nlocal/T);
	int final = tid * (Nlocal/T) + (Nlocal/T);
	printf("Hello MPI rank %d thread %d\n",rank,tid);
	/*
	//Cada hilo cuenta un determinado rango de números: división por datos de salida
	for (int i = 0; i < Nlocal; i++) {
		if ((arreglo_local[i] >= inicio) && (arreglo_local[i] < final)) numeros[arreglo_local[i]]++;
	}	
	*/
	int *cuenta = (int*)malloc(sizeof(int)*MAX_RANGE);
	for (int i = 0; i < MAX_RANGE; i++){
		cuenta[i] = 0;
	}
	
	//Cada hilo cuenta un determinado segmento del arreglo local: división por datos de entrada
	for (int i = inicio; i < final; i++) {
		cuenta[arreglo_local[i]]++;
	}
	//hilos pares suman desde 0
	if (tid % 2 == 0) {
		for (int i = 0; i < MAX_RANGE/2; i++) {
			if (cuenta[i] != 0) {
				pthread_mutex_lock(&Lock1);
				numeros[i] += cuenta[i];
				pthread_mutex_unlock(&Lock1);
			}
		}
		for (int i = MAX_RANGE/2; i < MAX_RANGE; i++) {
			if (cuenta[i] != 0) {
				pthread_mutex_lock(&Lock2);
				numeros[i] += cuenta[i];
				pthread_mutex_unlock(&Lock2);
			}
		}
	}
	
	//hilos impares suman desde maxrange/2
	else {
		for (int i = MAX_RANGE/2; i < MAX_RANGE; i++) {
			if (cuenta[i] != 0) {
				pthread_mutex_lock(&Lock2);
				numeros[i] += cuenta[i];
				pthread_mutex_unlock(&Lock2);
			}
		}
		for (int i = 0; i < MAX_RANGE/2; i++) {
			if (cuenta[i] != 0) {
				pthread_mutex_lock(&Lock1);
				numeros[i] += cuenta[i];
				pthread_mutex_unlock(&Lock1);
			}
		}
	}
	
	//Mergesort de resultados locales
	int largo = (tid * (MAX_RANGE/T) + (MAX_RANGE/T)) - 1;
	int tam = 1; //Comienzo desde abajo
	int izq, med, der;
	while (tam <= largo) {
		izq = tid * (MAX_RANGE/T);
		while (izq + tam <= largo) {
			med = izq + tam - 1;
			der = med + tam;
			if (der >= largo) der = largo-1;
			merge(izq, med, der);
			izq = der + 1;
		}
		tam *= 2;
	}
	pthread_exit(NULL);
}


int main(int argc, char* argv[]){
	//Variables	
	int i, j, k;
	
	double timetick, test1, test2;
	
	N = atoi(argv[1]);
	T = atoi(argv[2]);
	
	//Matriz de dos filas
	numeros=(int*)malloc(sizeof(int)*MAX_RANGE*2);
	
	for (i = 0; i < MAX_RANGE; i++){
		numeros[i] = 0;						//Fila 1 tiene cantidades
		numeros[MAX_RANGE+i] = i;			//Fila 2 tiene números
	}
	
	pthread_mutex_init(&Lock1, NULL);
	pthread_mutex_init(&Lock2, NULL);
	
	MPI_Init(&argc, &argv);
	int id;
	int M;
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
	MPI_Comm_size(MPI_COMM_WORLD,&M);
	rank = id;
	
	Nlocal = N/M;
	arreglo_local=(int*)malloc(sizeof(int)*Nlocal);

	if(id == 0) {
		
		printf("Elementos Totales : %d   Cantidad local : %d \n",N,Nlocal);
		//Inicializo el arreglo
		arreglo=(int*)malloc(sizeof(int)*N);
		srand (time (NULL));
		for (i=0;i<N;i++){
			arreglo[i] = rand() % MAX_RANGE;
		}
		
//Comienzo el conteo
		timetick = dwalltime();
		test1 = dwalltime();
	}
	
	
	//Envío las secciones del arreglo a las otras máquinas	
	MPI_Scatter(&arreglo[0], Nlocal, MPI_INT, &arreglo_local[0], Nlocal, MPI_INT, 0, MPI_COMM_WORLD);
	
	
	if (id == 0) printf("Tiempo de envío %f\n", dwalltime() - test1);
	if (id == 0) test1 = dwalltime();
	
	//Creo los hilos
	pthread_t misThreads[T];
	int threads_ids[T];
	for(int id=0;id<T;id++){
		threads_ids[id]=id;
		pthread_create(&misThreads[id],NULL,&sumar,(void*)&threads_ids[id]);
	}
	if (id == 0) printf("Tiempo de creación de hilos %f\n", dwalltime() - test1);
	if (id == 0) test1 = dwalltime();
	for(int id=0;id<T;id++){
		pthread_join(misThreads[id],NULL);
	}

	//free(arreglo_local);
	
	//Merge secciones de los hilos y obtener resultado local
	merge_secciones(T, 1);
	
	//Tomo los 100 máximos
	//free(numeros+100);
	int* maxLocal = (int*)malloc(sizeof(int)*100*2);
	for (i = 0; i < 100; i++) {
		maxLocal[i] = numeros[i];				//Fila 1 tiene cantidades
		maxLocal[100+i] = numeros[MAX_RANGE+i];	//Fila 2 tiene numeros
	}
	
	free(numeros);
	
	if (id == 0) printf("Tiempo de ejecución de hilos %f\n", dwalltime() - test1);
	if (id == 0) test1 = dwalltime();
	
	if ((id%2) == 0) {
		//Recibo los resultados de las otras máquinas y los sumo
		int* resultado = (int*)malloc(sizeof(int)*200);
		int* maximos = (int*)malloc(sizeof(int)*200);
		
		//Tag 2
		for (i = 1; i <= M/2; i=i*2) {
			if ((id%(i*2)) == 0) {
				MPI_Recv(&resultado[0], 200, MPI_INT, (id+i), 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			
				//Calculo los 100 mayores del arreglo local y el recibido
				j = 0;
				k = 0;
				for (i = 0; i < 100; i++) {
					if (maxLocal[k] >= resultado[j]) {						
						maximos[i] = maxLocal[k];
						maximos[100+i] = maxLocal[k+100];
						k++;
					}
					else {
						maximos[i] = resultado[j];
						maximos[i+100] = resultado[j+100];
						j++;
					}
					//printf("i = %d   j = %d   k = %d\n",i,j,k);
				}
				for (i = 0; i < 100; i++) {
					maxLocal[i] = maximos[i];				//Fila 1 tiene cantidades
					maxLocal[100+i] = maximos[100+i];	//Fila 2 tiene numeros
				}
				
			}			
		}
		
		
		free(resultado);
		free(maxLocal);
		
		/*		
		for (int i = 0; i < MAX_RANGE; i++) {
			printf("%d  ",numeros[i]);
		}
	
		printf("\n");	
	
		for (int i = 0; i < N; i++) {
			printf("%d  ",arreglo[i]);
		}
		printf("\n");		
		*/
		if (id == 0) printf("Tiempo de recepción %f\n", dwalltime() - test1);
		/*
		//Busco y muestro los 100 elementos más comunes
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
		}*/
		printf("Tiempo en segundos %f\n", dwalltime() - timetick);
		
		for (i = 0; i < 100; i++) {
			printf("Número : %d   Cantidad : %d \n",maximos[100+i],maximos[i]);
		}
		
		
	}
	else {
		//Envío los resultados al vecino
		MPI_Send(&maxLocal[0], 200, MPI_INT, id-1, 2, MPI_COMM_WORLD);
		
	}
	printf("Fin %d \n",id);
	pthread_mutex_destroy(&Lock1);
	pthread_mutex_destroy(&Lock2);
	free(arreglo_local);
	free(arreglo);
	MPI_Finalize();
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

void merge_secciones(int hilos, int secciones_agregadas) {
	while (hilos / 2 >= 1) {
		for (int i = 0; i < hilos; i = i + 2) {
			int izq = i * ((MAX_RANGE/T) * secciones_agregadas);
			int der = ((i + 2) * (MAX_RANGE/T) * secciones_agregadas) - 1;
			int med = izq + ((MAX_RANGE/T) * secciones_agregadas) - 1;
			merge(izq, med, der);
		}
		hilos = hilos / 2;
		secciones_agregadas = secciones_agregadas * 2;
	}
}









//513165165
