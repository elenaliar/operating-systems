#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define TEXT_SZ 100
#define s1 *child
#define s2 *parent
#define s3 *child2

struct message{
    int from; /*to index tou process apo opou erxetai to request, ta paidia exoun index apo 1,..,Ks*/
    int line; /*arithmos tis grammis pou zitithike*/
    char data[TEXT_SZ]; /*periexei tin pliroforia an prokeitai gia response*/
}msg;

void requestline(struct message*, int, int, int);
void findline(struct message*, FILE*, int);
