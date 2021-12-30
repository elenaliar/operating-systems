#include "header.h"

//periexei tis sinartiseis pou ilopoioun ta requests

//to paidi einai o client-consumer 
void requestline(struct message* pmsg, int max_lines, int N, int pr){
    sem_t *child = sem_open("/s1", O_RDWR);
    sem_t *parent = sem_open("/s2", O_RDWR);
    sem_t *child2 = sem_open("/s3", O_RDWR);
    clock_t start_t, end_t;
    double total_t=0.0;
    if (child == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    if (parent == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    if (child2 == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    for(int o=0; o<N; o++){
        if (sem_wait(&s1) <0){
            perror("sem_wait failed\n");
            exit(EXIT_FAILURE);
        }
        printf("\n");
        pmsg->from=pr;
        start_t = clock();//arxi xronometrisis tou request
        srand(time(NULL) + getpid() + rand());
        pmsg->line=(rand()%(max_lines))+1;
        printf("the child process %d requests line %d\n", pmsg->from, pmsg->line);
        if (sem_post(&s2) <0){
            perror("sem_post failed\n");
            exit(EXIT_FAILURE);
        }
        if (sem_wait(&s3) <0){
            perror("sem_wait failed\n");
            exit(EXIT_FAILURE);
        }
        end_t = clock();
        printf("the child process %d found this line in shared memory as the parent's response: %s", pmsg->from, pmsg->data);
        total_t += (double)(end_t - start_t) / CLOCKS_PER_SEC;
        if(o==N-1){
            total_t = total_t/N;
            printf("average time of child porcess %d with pid %d is %f\n", pmsg->from, getpid(), total_t);
        }
        if (sem_post(&s1) <0){
            perror("sem_post failed\n");
            exit(EXIT_FAILURE);
        }
    }
    if (sem_close(&s1) < 0){
        perror("sem_close(3) failed\n");
        sem_unlink("/s1");
        exit(EXIT_FAILURE);
    }
    if (sem_close(&s2) < 0){
        perror("sem_close(3) failed\n");
        sem_unlink("/s2");
        exit(EXIT_FAILURE);
    }
    if (sem_close(&s3) < 0){
        perror("sem_close(3) failed\n");
        sem_unlink("/s3");
        exit(EXIT_FAILURE);
    }
    exit(0);
}

//o parent einai o producer
void findline(struct message* pmsg, FILE* f, int tr){
    sem_t *child2 = sem_open("/s3", O_RDWR);
    sem_t *parent = sem_open("/s2", O_RDWR);
    if (child2 == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    if (parent == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
    for(int j=0; j<tr; j++){
        if (sem_wait(&s2) <0){
            perror("sem_wait failed\n");
            exit(EXIT_FAILURE);
        }
        char line[100];
        int i=0;
        rewind(f);
        while (fgets(line, sizeof(line), f)){
            i++;
            if(i==pmsg->line){
                strcpy(pmsg->data, line);
                //printf("requested line with number %d from child process %d is: %s", pmsg->line, pmsg->from, pmsg->data);
                break;
            }
        }
        if (sem_post(&s3) <0){
            perror("sem_post failed\n");
            exit(EXIT_FAILURE);
        }
    }
    if (sem_close(&s2) < 0){
        perror("sem_close(3) failed\n");
        sem_unlink("/s2");
        exit(EXIT_FAILURE);
    }
    if (sem_close(&s3) < 0){
        perror("sem_close(3) failed\n");
        sem_unlink("/s3");
        exit(EXIT_FAILURE);
    }
    return;
}