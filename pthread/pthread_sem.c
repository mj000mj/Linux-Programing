#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>

#define SEM_MAX  6

sem_t product_num, blank_num;
bool producer_live = false;

struct msg{
    int num;
    struct msg *next;
};

struct msg *head;
int count = 0;

/*生产者*/
void *producer(void *arg)
{
    struct msg *mg;
    int i,ret;
    while(1)
    {
        ret = sem_wait(&blank_num);
        mg = malloc(sizeof(struct msg));
        memset(mg, 0, sizeof(struct msg));

        count++;
        if(count >= 20)
        {
            producer_live = false;
            pthread_exit(NULL);
        }
        mg->num = rand()%100;
        mg->next = head;
        head = mg;
        printf("-----Producer (LWID = %lu) produce a num: %d------------\n", pthread_self(), mg->num);    

        sem_post(&product_num); 
        i = rand()%3;
        sleep(i);
    }
}

/*消费者*/
void *consumer(void *arg)
{
    struct msg *mg;
    int ret;
    int id;
    id = (int)arg;
    while(1)
    {
        if (head != NULL)
        {
            sem_wait(&product_num);
            mg = head;
            head = mg->next;

            sem_post(&blank_num);
            printf("**********%dth (LWID = %lu) Consume a num: %d**********\n", id, pthread_self(), mg->num);

            free(mg);
            if (count >= 20)
            {
                pthread_exit(NULL);
            }
        }
        else if (producer_live == false)
        {
            pthread_exit(NULL);
        }
    }
}


int main(int args, char* argv[])
{
    int ret;
    int i;

    pthread_t pth_pro_id;
    pthread_t pth_con_id[3];

    ret = sem_init(&product_num, 0, SEM_MAX);
    if (ret == -1)
    {
        fprintf(stderr, "sem_init error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully create a product sem\n");
    }

    ret = sem_init(&blank_num, 0, SEM_MAX);
    if (ret == -1)
    {
        fprintf(stderr, "sem_init error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully create a blank sem\n");
    }

    ret = pthread_create(&pth_pro_id, NULL, producer, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_create producer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        producer_live = true;
        printf("Successfully create a producer pthread\n");
    }

    for(i = 0; i < 3; i++)
    {
        ret = pthread_create(&pth_con_id[i], NULL, consumer, (void *)i);
        if (ret == -1)
        {
            fprintf(stderr, "pthread_create consumer error : %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Successfully create a %dth consumer pthread\n", i);
        }
    }

    ret = pthread_join(pth_pro_id, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_join producer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < 3; i++)
    {
        pthread_join(pth_con_id[i], NULL);
    }
    if (ret == -1)
    {
        fprintf(stderr, "pthread_join consumer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    sem_destroy(&product_num);
    sem_destroy(&blank_num);
    return 0;
}
