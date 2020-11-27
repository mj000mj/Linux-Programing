#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


pthread_mutex_t pmutex;

struct msg{
    int num;
    struct msg *next;
};

struct msg *head;
int count = 0;

void* producer(void* arg)
{
    struct msg *mg;
    int i,ret;
    while(1)
    {
        mg = malloc(sizeof(struct msg));
        memset(mg, 0, sizeof(struct msg));

        ret = pthread_mutex_lock(&pmutex);
        if(ret == 0)
        {
            count++;
            if(count >= 20)
            {
                pthread_mutex_unlock(&pmutex);
                pthread_exit(NULL);
            }
            mg->num = rand()%100;
            mg->next = head;
            head = mg;
            printf("-----Produce a num: %d------------\n", mg->num);    
            pthread_mutex_unlock(&pmutex);
    
            i = rand()%2;
            sleep(i);
        }
    }
}

void* consumer(void* arg)
{
    struct msg *mg;
    int i, ret;

    while(1)
    {
        ret = pthread_mutex_lock(&pmutex);
        if (ret == 0)
        {
            if(head != NULL)
            {
                mg = head;
                head = mg->next;

                printf("-----Consume a num: %d--------\n", mg->num);

                free(mg);
            }
            else
            {
                if (count >= 20)
                {
                    pthread_exit(NULL);
                    pthread_mutex_unlock(&pmutex);
                }
            }

            pthread_mutex_unlock(&pmutex);
            
        }                       
        
        i = rand()%2;
        sleep(i);
    }
}


int main(int args, char* argv[])
{
    int ret;
    
    pthread_t pth_pro_id;
    pthread_t pth_con_id;
    //pthread_cond_t pcond;

    ret = pthread_mutex_init(&pmutex, NULL);
    //ret = pthread_cond_init(&pcond, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_create producer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully create a mutex\n");
    }

    ret = pthread_create(&pth_pro_id, NULL, producer, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_create producer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully create a producer pthread\n");
    }

    ret = pthread_create(&pth_con_id, NULL, consumer, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_create consumer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Successfully create a consumer pthread\n");
    }

    ret = pthread_join(pth_pro_id, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_join producer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    ret = pthread_join(pth_con_id, NULL);
    if (ret == -1)
    {
        fprintf(stderr, "pthread_join consumer error : %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    ret = pthread_mutex_destroy(&pmutex);
    return 0;
}
