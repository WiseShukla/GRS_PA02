/* PhD25001 */
/* Roll No: PhD25001 */
/* common header for Part A multithreaded implementations */

#define _GNU_SOURCE
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/errqueue.h>

#define DEFAULT_PORT 5000
#define NUM_FIELDS 8
#define DURATION_SEC 10

/* Requirement: The message structure comprising 8 dynamically allocated string fields. */
typedef struct {
    char *fields[NUM_FIELDS]; 
    uint32_t field_sizes[NUM_FIELDS];
    uint32_t total_payload_size;
} msg_struct_t;

/* Unified thread argument for both client and server profiling */
typedef struct {
    int client_fd;
    const char *server_ip;
    int server_port;
    int msg_size;
    int thread_id;
    int duration_sec;
    volatile int *running;
    /* Quantitative metrics for Part B */
    unsigned long long bytes_transferred;
    unsigned long long messages_transferred;
    double elapsed_sec;
    double total_latency_us;
} thread_arg_t;

/* --- Utility Functions --- */

static inline double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

static inline double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Requirement: Heap-allocated buffers (malloc) for EACH message field. */
static inline char** alloc_message_fields(int field_size) {
    char **fields = (char **)malloc(NUM_FIELDS * sizeof(char *));
    if (!fields) { perror("malloc fields"); exit(1); }
    for (int i = 0; i < NUM_FIELDS; i++) {
        fields[i] = (char *)malloc(field_size);
        if (!fields[i]) { perror("malloc field"); exit(1); }
        memset(fields[i], 'A' + (i % 26), field_size);
    }
    return fields;
}

static inline void free_message_fields(char **fields) {
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(fields[i]);
    }
    free(fields);
}

/* * Helper to set socket timeouts (Prevent Deadlocks) */
static inline void set_socket_timeout(int fd, int sec) {
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) perror("setsockopt RCVTIMEO");
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) perror("setsockopt SNDTIMEO");
}

/* Reliable Transmission with Deadlock & Crash Prevention */
static inline ssize_t send_all(int fd, const void *buf, size_t len, int flags) {
    size_t total = 0;
    while (total < len) {
        /* MSG_NOSIGNAL: Ensure this call never generates SIGPIPE */
        ssize_t n = send(fd, (const char *)buf + total, len - total, flags | MSG_NOSIGNAL);
        
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                usleep(1000); 
                continue;
            }
            return -1;
        }
        total += n;
    }
    return total;
}

static inline ssize_t recv_all(int fd, void *buf, size_t len, int flags) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(fd, (char *)buf + total, len - total, flags);
        if (n <= 0) {
            if (n == 0) return 0;
            if (n < 0 && errno == EINTR) continue;
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                usleep(1000);
                continue;
            }
            return -1;
        }
        total += n;
    }
    return total;
}

#endif