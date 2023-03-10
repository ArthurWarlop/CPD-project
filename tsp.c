#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "queue.h"

#define MAX 100000000

typedef struct{
    int* tour;
    double cost;
    double bound;
    int length;
    int current_city;
} Element;

double **distances;
double **min_distances;
priority_queue_t *queue;

char compare(void* node1, void* node2) {
    Element* element1 = (Element*)node1;
    Element* element2 = (Element*)node2;
    if (element1->bound < element2->bound)
        return 0;
    else if (element1->bound == element2->bound && element1->current_city < element2->current_city)
        return 0;
    return 1;
}

void printSolution(int* bestTour, int bestTourLength, double bestTourCost) {
    printf("%0.1f\n", bestTourCost);
    for(int j = 0; j < bestTourLength; j++)
        printf("%d ", bestTour[j]);
    printf("0\n");
}

int isElementInTour (int element, int length, int* tour) {
    for (int j = 1; j < length; j++)
        if (tour[j] == element)
            return 1;
    return 0;
}

double getMinDistance (double distance, double value1, double value2) {
    if(distance >= value2)
        return value2;
    return value1;
}

double lowerbound_estimate(int n_cities){

    double lowerbound=0;
    int i;
    for(i=0;i<n_cities;i++){
        lowerbound+=min_distances[i][0]+min_distances[i][1];

    }

    return lowerbound/(double)2;
}

int free_alocated_space(int n_cities){

    int i;
    for(i=0;i<n_cities;i++){
        free(distances[i]);
        free(min_distances[i]);
    }
    free(distances);
    free(min_distances);

    while(1){
        Element *a=queue_pop(queue);
        if(a!=NULL){
            free(a->tour);
            free(a);
        }else{
            break;
        }
    }

    queue_delete(queue);
    free(queue);

    return 0;
}

int get_min_distances(int n_cities){
    int i,j;
    double min1,min2,temp;
    for(i=0;i<n_cities;i++){
        min1=MAX;
        min2=MAX;
        for(j=0;j<n_cities;j++){
            if(distances[i][j]<min2 && distances[i][j]!=0){
                min2=distances[i][j];
                if(min2<min1){
                    temp=min1;
                    min1=min2;
                    min2=temp;
                }
            }
        }
        min_distances[i][0]=min1;
        min_distances[i][1]=min2;
    }
    return 0;
}

int tsp(int n_cities, double max_value){
    
    int i;
    double lowerbound=0;
    double bestTourCost = max_value;
    int bestTourLength=0;
    double distance=0;
    double newBound=0;
    double newCost=0;
    double cf=0,ct=0, min1f = 0, min2f = 0, min1t = 0, min2t = 0;
    int* bestTour = (int*)malloc(n_cities * sizeof(int));

    lowerbound = lowerbound_estimate(n_cities);

    Element* element1;
    element1 = malloc(sizeof(Element));
    element1->tour = (int*)malloc(1 * sizeof(int));
    element1->tour[0] = 0;
    element1->bound = lowerbound;
    element1->current_city = 0;
    element1->length = 1;
    element1->cost = 0;

    queue = queue_create(compare);
    queue_push(queue, element1);
    
    while(queue->size != 0){
        Element* element = queue_pop(queue);
        if(element->bound >= bestTourCost){
            if (bestTourCost >= max_value){
                printf("NO SOLUTION\n");
                free(bestTour);
                return 1;
            }
            else{
                printSolution(bestTour, bestTourLength, bestTourCost);
                free(bestTour);
                return 0;
            }
        }
        if(element->length == n_cities){
            if(distances[0][element->current_city] != 0 && element->cost + distances[0][element->current_city] < bestTourCost){
                memcpy(bestTour,element->tour,element->length*sizeof(int));
                bestTourCost = element->cost + distances[0][element->current_city];
                bestTourLength = element->length;
            }
        }
        else{
            for (i = 1; i < n_cities; i++){
                int inTour = isElementInTour(i, element->length, element->tour);  
                if(distances[element->current_city][i] != 0 && inTour != 1){
                    cf=0,ct=0, min1f = 0, min2f = 0, min1t = 0, min2t = 0;
                    distance = distances[element->current_city][i];
                                     
                    min1f=min_distances[element->current_city][0];
                    min2f=min_distances[element->current_city][1];
                    min1t=min_distances[i][0];
                    min2t=min_distances[i][1];

                    //First city
                    cf = getMinDistance(distance, min1f, min2f);
                    //Second city
                    ct = getMinDistance(distance, min1t, min2t);
                    
                    newBound = element->bound + distance - ((cf + ct)/(double)2);

                    int newLength = element->length + 1;
                    if(newBound <= bestTourCost){
                        int* newTour = (int*)malloc(newLength * sizeof(int));
                        
                        memcpy (newTour, element->tour, element->length*sizeof(int));
                        newTour[element->length] = i;
                        
                        newCost = element->cost + distances[element->current_city][i];
                        Element *newElement = malloc(sizeof(Element));
                        newElement->tour = (int*)malloc(newLength * sizeof(int));
                        
                        memcpy(newElement->tour,newTour,newLength*sizeof(int));
                        free(newTour);
                        newElement->bound = newBound;
                        newElement->current_city = i;
                        newElement->length = newLength;
                        newElement->cost = newCost;

                        queue_push(queue, newElement);
                    }
                }
            }
        }
        free(element->tour);
        free(element);
    }

    if (bestTourCost >= max_value){
        printf("NO SOLUTION\n");
        free(bestTour);
        return 1;
    }
    else{
        printSolution(bestTour,bestTourLength, bestTourCost);
        free(bestTour);
        return 0;
    }

}

int main(int argc, char*argv[]){
    int i, n_cities, city_1, city_2; 
    double distance_value;
    double max_value=atof(argv[2]);
    
    FILE* ptr;
    char str[50];
    ptr = fopen(argv[1], "r");

    if (NULL == ptr) {
        printf("file can't be opened \n");
    }

    if(fgets(str, 50, ptr)==NULL){
        printf("file read wen't wrong somewhere!\n");
    }
    n_cities=atoi(strtok(str," "));

    distances = (double**)malloc(n_cities * sizeof(double*));
    min_distances=(double**)malloc(n_cities * sizeof(double*));

    for (i = 0; i < n_cities; i++){
        distances[i] = (double*)malloc(n_cities * sizeof(double));
        memset(distances[i],0,n_cities*sizeof(double));
        min_distances[i] = (double*)malloc(2 * sizeof(double));
        memset(min_distances[i],0,2*sizeof(double));
    }

    while (fgets(str, 50, ptr) != NULL) {
        city_1=atoi(strtok(str," "));
        city_2=atoi(strtok(NULL," "));
        distance_value=atof(strtok(NULL," "));
        distances[city_1][city_2]=distance_value;
        distances[city_2][city_1]=distance_value;
    }
    fclose(ptr);

    get_min_distances(n_cities);

    double exec_time;
    exec_time = -omp_get_wtime();

    tsp(n_cities,max_value);
    
    free_alocated_space(n_cities);

    exec_time += omp_get_wtime();  
    fprintf(stderr, "%.1fs\n", exec_time);

    return 0;
}