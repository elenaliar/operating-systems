#include "header.h"


int main(int argc, char* argv[]){

    FILE *fptr;
    int count_lines=0, K, N, i, pr, y, total_requests;
    char chr;
    char*  X;//onoma tou text file
    key_t shm_key;
    int shm_id;
    void *shrd_value;
    sem_t *child, *parent, *child2;
    clock_t start_t, end_t;
    double total_t;

    //oi arithmoi K, N, X perniountai os arguments sto  command line me auti tin seira
    K = atoi(argv[1]);
    N = atoi(argv[2]);
    X = argv[3];
    total_requests = K*N;//sinolo aitimaton pou tha ikanopoiisei o pateras

    fptr = fopen(X,"r");
    pid_t par= getpid();//se auti tin metavliti apothikevetai to pid tou parent process

    if (fptr == NULL){
        printf("Error opening file");   
        exit(EXIT_FAILURE);             
    }

    //ipologismos poson grammon exei to arxeio lyrics.txt
    for (chr = getc(fptr); chr != EOF; chr = getc(fptr))
        if (chr == '\n') // Increment count if this character is newline
            count_lines = count_lines + 1;
            
    //ektiposeis pou dinoun plirofories gia tis metavlites apo tin grammi entolon
    printf("%d child processes will be created.\n", K);
    printf("Each child will make %d requests.\n", N);
    printf("The name of the text file is %s.\n", X);
    printf("There are %d lines in the given text file.\n", count_lines);

    //create shared memory
    shm_key = ftok("main.c", 64);
    if(shm_key == -1){
        printf("error in ftok\n");
        exit(1);
    }

    shm_id = shmget(shm_key, sizeof(struct message), 0644 | IPC_CREAT);
    if (shm_id == -1){
        printf("shmget failed\n");
        exit(EXIT_FAILURE);
    }


    shrd_value = shmat(shm_id, NULL, 0);
    if (shrd_value == (void *) -1){
        printf("shmat failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Process %d attached the segment %d\n", getpid(), shm_id);

    printf("Shared memory segment with id %d attached at %p\n", shm_id, shrd_value);
    struct message* pmsg = (struct message *)shrd_value;

    //dimiourgia semaphore
    child = sem_open("/s1", O_CREAT, 0600, 1);//autos einai gia to paidi kai exei value 1 oste prota na arxizei to child process
    parent = sem_open("/s2", O_CREAT, 0600, 0);//autos einai gia ton parent kai exei value 0 oste na min arxizei to parent porcess proto
    child2  = sem_open("/s3", O_CREAT, 0600, 0);//kai autos o semaphore einai gia to paidi gia na elegxei pote tha kaleitai i sinartisi pou tha tiponei tin zitoumeni seira
    
    //elegxos gia to an dimioyrgithikan sosta oi semaphores
    if (child != SEM_FAILED) {
		printf("created new semaphore!\n");
	}
    else{
        perror("error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (child2 != SEM_FAILED) {
		printf("created new semaphore!\n");
	}
    else{
        perror("error creating semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (parent != SEM_FAILED) {
		printf("created new semaphore!\n");
	}
    else{
        perror("error creating semaphore\n");
        exit(EXIT_FAILURE);
    }


    printf("\n");
    
    //dimiourgia K child processes me to idio parent process
    for(i=0; i< K; i++){
        pid_t p = fork();
        if(p < 0 ){
            total_requests = total_requests-N;//meiono ta total requests afou tha dimiourgithike ena ligpotero paidi
            printf("fork failed, one less child\n");
            continue;
        }
        if(p == 0){
            pr = i+1;//stin metavliti pr apothikevetai i seira tou child process
            printf("Child process %d with pid %d forked from parent process with pid %d\n", pr, getpid(), getppid());
            break;
        }
    }

    pid_t curr= getpid();//stin curr apothikevetai to pid tis trexousas diergasias

    //edo ginetai o diaxorismos child kai parent
    if(curr != par){
            requestline(pmsg, count_lines, N, pr);
    }
    else if (curr == par){
            findline(pmsg, fptr, total_requests); 
    }
    //perimenei na termatisoun ola ta paidia
    for(int i=0; i<K; i++){
        wait(NULL);
    }
    //apodesmeusi mnimis kai semaphores
    if (shmdt(shrd_value) == -1) {
    	perror("shmdt failed\n");
	    exit(EXIT_FAILURE);
    }
    else
        printf("Process %d detached the segment %d\n", getpid(), shm_id);

    if (shmctl(shm_id,IPC_RMID,NULL) == -1){
        perror("shmctl returned -1\n");
        exit(EXIT_FAILURE);
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
    if (sem_unlink("/s1") < 0)
        perror("sem_unlink(3) failed");
    if (sem_unlink("/s2") < 0)
        perror("sem_unlink(3) failed");
    if (sem_unlink("/s3") < 0)
        perror("sem_unlink(3) failed");

    fclose(fptr);
    return 0;
}

