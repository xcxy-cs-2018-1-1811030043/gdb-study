#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/* 出错处理宏供返回错误码的函数使用 */
#define handle_error_en(en, msg) \
    do                           \
    {                            \
        errno = en;              \
        perror(msg);             \
        exit(EXIT_FAILURE);      \
    } while (0)

/* 出错处理宏 */
#define handle_error(msg)   \
    do                      \
    {                       \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    }                       \
    while (0)
    
struct thread_info
{
    pthread_t thread_id;
    int thread_num;
    char *argv_string;
};

static void *thread_start(void *arg)
{ /* 线程运行函数 */
    struct thread_info *tinfo = arg;
    char *uargv, *p;

    /* 通过 p 的地址来计算栈的起始地址*/
    printf("Thread %d: top of stack near %p; argv_string=%s\n", tinfo->thread_num, &p, tinfo->argv_string);
    uargv = strdup(tinfo->argv_string);
    if (uargv == NULL)
        handle_error("strdup");
    for (p = uargv; *p != '\0'; p++)
        *p = toupper(*p); /* 小写字符转换大写字符 */
    return uargv;         /* 将转换结果返回 */
}

int main(int argc, char *argv[])
{
    int s, tnum, opt, num_threads;
    struct thread_info *tinfo;
    pthread_attr_t attr;
    int stack_size;
    void *res;
    stack_size = -1;
    while ((opt = getopt(argc, argv, "s:")) != -1)
    { /* 处理参数-s 所指定的栈大小 */
        switch (opt)
        {
        case 's':
            stack_size = strtoul(optarg, NULL, 0);
            break;
        default:
            fprintf(stderr, "Usage: %s [-s stack-size] arg...\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    num_threads = argc - optind;
    s = pthread_attr_init(&attr); /* 初始化属性对象 */
    if (s != 0)
        handle_error_en(s, "pthread_attr_init");
    if (stack_size > 0)
    {
        s = pthread_attr_setstacksize(&attr, stack_size); /* 设置属性对象的栈大小 */
        if (s != 0)
            handle_error_en(s, "pthread_attr_setstacksize");
    }
    tinfo = calloc(num_threads, sizeof(struct thread_info));
    if (tinfo == NULL)
        handle_error("calloc");
    for (tnum = 0; tnum < num_threads; tnum++)
    {
        tinfo[tnum].thread_num = tnum + 1;
        tinfo[tnum].argv_string = argv[optind + tnum];
        /* 根据属性创建线程 */
        s = pthread_create(&tinfo[tnum].thread_id, &attr, &thread_start, &tinfo[tnum]);
        if (s != 0)
            handle_error_en(s, "pthread_create");
    }
    s = pthread_attr_destroy(&attr); /* 销毁属性对象 */
    if (s != 0)
        handle_error_en(s, "pthread_attr_destroy");
    for (tnum = 0; tnum < num_threads; tnum++)
    {
        s = pthread_join(tinfo[tnum].thread_id, &res); /* 等待线程终止，并获取返回值 */
        if (s != 0)
            handle_error_en(s, "pthread_join");
        printf("Joined with thread %d; returned value was %s\n",
            tinfo[tnum].thread_num, (char *)res);
        free(res);
    }
    free(tinfo);
    exit(EXIT_SUCCESS);
}