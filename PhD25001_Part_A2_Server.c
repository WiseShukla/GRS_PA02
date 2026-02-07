/* PhD25001 */
/* Roll No: PhD25001 */
/* Part A2: One-Copy Implementation - Server Component */

#include "PhD25001_Part_A_common.h"

static volatile int g_running = 1;

void handle_sigint(int sig) { 
    (void)sig; 
    g_running = 0; 
}

void *server_thread_func(void *arg) {
    thread_arg_t *ta = (thread_arg_t *)arg;
    int field_size = ta->msg_size / NUM_FIELDS;
    
    /* Requirement: 8 dynamically allocated fields */
    char **fields = alloc_message_fields(field_size);

    /* Use iovec to eliminate the manual aggregation copy */
    struct iovec iov[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = fields[i];
        iov[i].iov_len = field_size;
    }
    struct msghdr msg = { .msg_iov = iov, .msg_iovlen = NUM_FIELDS };

    while (g_running) {
        
        
        if (sendmsg(ta->client_fd, &msg, MSG_NOSIGNAL) <= 0) {
            break;
        }
        
        ta->bytes_transferred += ta->msg_size;
        ta->messages_transferred++;
    }

    free_message_fields(fields);
    close(ta->client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
   
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);

    if (argc < 4) return 1;
    int port = atoi(argv[1]), msg_size = atoi(argv[2]), max_threads = atoi(argv[3]);
    
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = INADDR_ANY };
    
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    listen(sfd, max_threads);

    pthread_t threads[max_threads];
    thread_arg_t args[max_threads];
    int count = 0;

    while (g_running && count < max_threads) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) { 
            if (errno == EINTR) continue; 
            break; 
        }
        
        args[count] = (thread_arg_t){
            .client_fd = cfd, 
            .server_port = port, 
            .msg_size = msg_size,
            .thread_id = count, 
            .duration_sec = DURATION_SEC,
            .running = &g_running, 
            .bytes_transferred = 0,
            .messages_transferred = 0, 
            .elapsed_sec = 0.0, 
            .total_latency_us = 0.0
        };
        pthread_create(&threads[count], NULL, server_thread_func, &args[count]);
        count++;
    }

    for (int i = 0; i < count; i++) pthread_join(threads[i], NULL);
    close(sfd);
    return 0;
}