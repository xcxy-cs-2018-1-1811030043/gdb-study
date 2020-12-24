#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
pthread_t tid[2];
pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER; /* 静态初始化互斥量 */
pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;
void *t1(void *arg)
{
    pthread_mutex_lock(&mutexA); /* 线程 1 获取 mutexA */
    printf("t1 get mutexA\n");
    usleep(1000);
    pthread_mutex_lock(&mutexB); /* 线程 1 获取 mutexB */
    printf("t1 get mutexB\n");
    pthread_mutex_unlock(&mutexB); /* 线程 1 释放 mutexB */
    printf("t1 release mutexB\n");
    pthread_mutex_unlock(&mutexA); /* 线程 1 释放 mutexA */
    printf("t1 release mutexA\n");
    return NULL;
}
void *t2(void *arg)
{
    pthread_mutex_lock(&mutexB);
    printf("t2 get mutexB\n");

    usleep(1000);
    pthread_mutex_lock(&mutexA);
    printf("t2 get mutexA\n");
    pthread_mutex_unlock(&mutexA);
    printf("t2 release mutexA\n");
    pthread_mutex_unlock(&mutexB);
    printf("t2 release mutexB\n");
    return NULL;
}
int main(void)
{
    int err;
    err = pthread_create(&(tid[0]), NULL, &t1, NULL); /* 创建线程 1 */
    if (err != 0)
        printf("Can't create thread :[%s]", strerror(err));
    err = pthread_create(&(tid[1]), NULL, &t2, NULL); /* 创建线程 2 */
    if (err != 0)
        printf("Can't create thread :[%s]", strerror(err));
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    return 0;
}