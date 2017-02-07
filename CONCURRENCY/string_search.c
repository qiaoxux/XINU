#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <string.h>
#include <sys/time.h>

#define THREAD_NUM 500

struct thread_session {
    pthread_t tid;

    int t_num;
    long size;
    char *start;
    char *pattern;
};

unsigned long get_current_time()
{
    struct timeval t_val;
    gettimeofday(&t_val, NULL);
    return (unsigned long) (t_val.tv_sec * 1000000 + t_val.tv_usec);
}

void *search_string_function(void *arg) {
    struct thread_session *t_session = (struct thread_session *) arg;

    int size = t_session->size;
    char *start = t_session->start, *pattern = t_session->pattern;

    int count = 0, searchlen = strlen(pattern);
    char *end = start + size, *cur = start;

    while ((cur = strstr(cur, pattern))) {
        if (cur >= end)
            break;
        cur += searchlen;
        count++;
    }
    return (void *) count;

}

int main(int argc, char *argv[])
{
    char *buffer;
    long file_size, block_size;

    char *search_string = argv[1], *target_file = argv[2], *output_file = "string_search.out";

    FILE *fp;
    fp = fopen(target_file, "r");
    if(fp) {
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        buffer = (char*) calloc(file_size, sizeof(char));
        if(buffer == NULL) {
            printf("Out of Memory");
            return -1;
        }
        fread(buffer, sizeof(char), file_size, fp);
    }
    fclose(fp);

    int i, j;
    void *res;
    unsigned int count;
    unsigned long start, end;

    pthread_attr_t attr;
    struct thread_session *t_session;

    fp = fopen(output_file, "w+");
    for(i = 1; i <= THREAD_NUM; ++i) {
        count = 0;
        block_size = file_size / i + 1;

        pthread_attr_init(&attr);
        t_session = calloc(i, sizeof(struct thread_session));
        if (t_session == NULL) {
            printf("Out of Memory");
            return -1;
        }
        for(j = 0; j < i; ++j) {
            t_session[j].t_num = j + 1;
            t_session[j].size = (buffer + block_size * j) > (buffer + file_size) ? (buffer + file_size) - (buffer + block_size * j) : block_size;
            t_session[j].start = strndup(buffer + block_size*j, block_size + 80);
            t_session[j].pattern = search_string;
        }

        start = get_current_time();

        for(j = 0; j < i; ++j) {
            if (pthread_create(&t_session[j].tid, &attr, &search_string_function, &t_session[j])) {
                printf("Thread Creation Error");
                return -1;
            }
        }

        pthread_attr_destroy(&attr);
        for(j = 0; j < i; ++j) {
            if (pthread_join(t_session[j].tid, &res)) {
                printf("Thread Join Error");
                return -1;
            }
            free(t_session[j].start);
            count += (unsigned int) res;
        }

        free(t_session);
        end = get_current_time();

        printf("blockSize: %8lu  numOfThread: %4d  matchCount: %4d  runningTime: %8lu\n", block_size, i, count, end - start);
        fprintf(fp, "blockSize: %8lu  numOfThread: %4d  matchCount: %4d  runningTime: %8lu\n", block_size, i, count, end - start);
    }
    free(buffer);
    fclose(fp);
    return 0;
}