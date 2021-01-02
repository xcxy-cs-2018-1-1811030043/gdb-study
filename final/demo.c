#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define MYFIFO            "myfifo"
#define BUFFER_SIZE        3
#define UNIT_SIZE          5
#define RUN_TIME           30
#define DELAY_TIME_LEVELS  5

void *producer(void *arg);//生产者
void *customer(void *arg);//消费者

int fd;//管道描述符
time_t end_time;//存放线程的起始时间
sem_t mutex, full, avail;//信号量描述符（互斥、full、avail）

int main(int argc, const char *argv[])
{
    int ret;
    pthread_t thrd_prd_id,thrd_cst_id;

    srand(time(NULL));//随机数发生器初始化s
    end_time = time(NULL) + RUN_TIME;

    //创建管道
    if ((mkfifo(MYFIFO, 0644) < 0) && (errno != EEXIST)) {
        perror("mkfifo error!");
        exit(-1);
    }

    //打开管道
    fd = open(MYFIFO, O_RDWR);
    if (fd == -1) {
        perror("open fifo error");
        exit(-1);
    }

    //初始化信号量
    ret = sem_init(&mutex, 0, 1);
    ret += sem_init(&avail, 0, BUFFER_SIZE);
    ret += sem_init(&full, 0, 0);
    if (ret != 0) {
        perror("sem_init error");
        exit(-1);
    }

    //创建两个线程
    ret = pthread_create(&thrd_prd_id, NULL, producer, NULL);
    if (ret != 0) {
        perror("producer pthread_create error");
        exit(-1);
    }
    ret = pthread_create(&thrd_cst_id, NULL, customer, NULL);
    if (ret != 0) {
        perror("customer pthread_create error");
        exit(-1);
    }
    pthread_join(thrd_prd_id, NULL);
    pthread_join(thrd_cst_id, NULL);
    sem_destroy(&mutex);
    sem_destroy(&avail);
    sem_destroy(&full);
    close(fd);
    unlink(MYFIFO);//结束删除
    return 0;
}

void *producer(void *arg)
{
    int real_write;
    int delay_time;

    while (time(NULL) < end_time) {
        delay_time = rand()%DELAY_TIME_LEVELS;
        sleep(delay_time);
        
        //p操作
        sem_wait(&avail);
        sem_wait(&mutex);
        printf("\nproducer have delayed %d seconds\n", delay_time);

        //生产者写入数据  执行的操作
        if (-1 == (real_write = write(fd, "hello", UNIT_SIZE))) {
            if (errno == EAGAIN) {
                printf("The buffer is full, please wait for reading!\n");
            }
        } else {
            printf("producer writes %d bytes to the FIFO\n", real_write);
        }
        //v操作
        sem_post(&full);
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}

void *customer(void *arg)
{
    unsigned char read_buffer[UNIT_SIZE];
    int real_read;
    int delay_time;

    while (time(NULL) < end_time) {
        delay_time = rand()%DELAY_TIME_LEVELS;
        sleep(delay_time);
        //p操作
        sem_wait(&full);
        sem_wait(&mutex);
        memset(read_buffer, 0, UNIT_SIZE);
        printf("\nCustomer have delayed %d seconds\n", delay_time);

        //消费 操作
        if (-1 == (real_read = read(fd, read_buffer, UNIT_SIZE))) {
            if (errno == EAGAIN) {
                printf("The buffer is empty, please wait for writing!\n");
            }
        } else {
            printf("customer reads %d bytes from the FIFO\n", real_read);
        }
        //v操作
        sem_post(&avail);
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}