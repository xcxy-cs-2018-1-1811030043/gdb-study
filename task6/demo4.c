#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t tid[2];
pthread_mutex_t lock;

void *doPrint(void *arg)
{
    int id = (long)arg;
    int i = 0;
    pthread_mutex_lock(&lock); /* 使用互斥量保护临界区 */
    printf("Job %d started\n", id);
    for (i = 0; i < 5; i++)
    {
        printf("Job %d printing\n", id);
        usleep(10);
    }
    printf("Job %d finished\n", id);
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(void)
{
    long i = 0;
    int err;

    if (pthread_mutex_init(&lock, NULL) != 0) /* 动态初始化互斥量 */
    {
        printf("\n Mutex init failed\n");
        return 1;
    }
    while (i < 2)
    {
        err = pthread_create(&(tid[i]), NULL, &doPrint, (void *)i);
        if (err != 0)
            printf("Can't create thread :[%s]", strerror(err));
        i++;
    }
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_mutex_destroy(&lock);

    return 0;
}