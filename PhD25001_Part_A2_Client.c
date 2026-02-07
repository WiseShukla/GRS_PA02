/* Roll No: PhD25001 */
/* Part A2: One-Copy Implementation - Client Component */

#include "PhD25001_Part_A_common.h"

static volatile int g_running = 1;
void handle_alarm(int sig) { (void)sig; g_running = 0; }

void *client_thread_func(void *arg) {
    thread_arg_t *ta = (thread_arg_t *)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(ta->server_port) };
    inet_pton(AF_INET, ta->server_ip, &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect"); return NULL;
    }

    char *buf = malloc(ta->msg_size);
    double start_time = get_time_sec();

    while (g_running) {
        double msg_start = get_time_us();
        /* Sink for the data sent via scatter-gather sendmsg() */
        if (recv_all(fd, buf, ta->msg_size, 0) <= 0) break;
        
        ta->total_latency_us += (get_time_us() - msg_start);
        ta->bytes_transferred += ta->msg_size;
        ta->messages_transferred++;
    }

    ta->elapsed_sec = get_time_sec() - start_time;
    free(buf);
    close(fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) return 1;
    const char *ip = argv[1];
    int port = atoi(argv[2]), msg_size = atoi(argv[3]), num_t = atoi(argv[4]);
    int duration = (argc > 5) ? atoi(argv[5]) : DURATION_SEC;

    signal(SIGALRM, handle_alarm);
    alarm(duration);

    pthread_t threads[num_t]; thread_arg_t args[num_t];
    for (int i = 0; i < num_t; i++) {
        /* Correct initialization matching thread_arg_t */
        args[i] = (thread_arg_t){ .server_ip = ip, .server_port = port, .msg_size = msg_size, 
                                  .thread_id = i, .running = &g_running, .bytes_transferred = 0,
                                  .messages_transferred = 0, .total_latency_us = 0 };
        pthread_create(&threads[i], NULL, client_thread_func, &args[i]);
    }
    
    double total_lat = 0; unsigned long long total_bytes = 0, total_msgs = 0;
    for (int i = 0; i < num_t; i++) {
        pthread_join(threads[i], NULL);
        total_bytes += args[i].bytes_transferred;
        total_msgs += args[i].messages_transferred;
        total_lat += args[i].total_latency_us;
    }

    double throughput = (total_bytes * 8.0) / (duration * 1e9);
    printf("RESULT,onecopy,%d,%d,%.6f,%.2f,%llu\n", msg_size, num_t, throughput, total_lat/total_msgs, total_bytes);
    return 0;
}