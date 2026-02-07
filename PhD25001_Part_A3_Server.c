/* PhD25001 */
/* Roll No: PhD25001 */
/* Part A3: Zero-Copy Implementation (MSG_ZEROCOPY) - Server Component */

#include "PhD25001_Part_A_common.h"

static volatile int g_running = 1;

void handle_sigint(int sig) { 
    (void)sig; 
    g_running = 0; 
}

/**
 * read_errqueue:
 * Checks the ERRQUEUE. Returns 1 if a notification was found, 0 if empty/EAGAIN.
 */
static int read_errqueue(int fd) {
    char msg_control[512];
    struct iovec iov;
    char buf[1]; // Dummy buffer
    struct msghdr msg = {0};
    struct sock_extended_err *serr;
    struct cmsghdr *cm;

    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = msg_control;
    msg.msg_controllen = sizeof(msg_control);

    if (recvmsg(fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT) < 0) {
        return 0; /* Queue is empty, return immediately */
    }

    cm = CMSG_FIRSTHDR(&msg);
    if (!cm || cm->cmsg_level != SOL_IP || cm->cmsg_type != IP_RECVERR) {
        return 0;
    }

    serr = (struct sock_extended_err *)CMSG_DATA(cm);
    if (serr->ee_errno != 0 && serr->ee_origin == SO_EE_ORIGIN_ZEROCOPY) {
        return 1; /* Successfully read a notification */
    }
    return 0;
}

void *server_thread_func(void *arg) {
    thread_arg_t *ta = (thread_arg_t *)arg;
    int client_fd = ta->client_fd;
    int msg_size = ta->msg_size;

    /* Timeout to ensure thread eventually dies if client disappears */
    set_socket_timeout(client_fd, 1);

    char *contiguous_buf = (char *)malloc(msg_size);
    memset(contiguous_buf, 'Z', msg_size); 

    struct iovec iov;
    struct msghdr msg = {0};

    iov.iov_base = contiguous_buf;
    iov.iov_len = msg_size;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    int one = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_ZEROCOPY, &one, sizeof(one))) {
        perror("setsockopt SO_ZEROCOPY");
        free(contiguous_buf);
        close(client_fd);
        return NULL;
    }

    while (g_running) {
        /* FIX: Send with MSG_NOSIGNAL to prevent crashes */
        ssize_t sent = sendmsg(client_fd, &msg, MSG_ZEROCOPY | MSG_NOSIGNAL);
        
        if (sent <= 0) {
            /* Case A: TCP Send Buffer Full -> Wait slightly and retry */
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100); 
                /* While waiting for space, try to drain the error queue */
                read_errqueue(client_fd);
                continue;
            }
            
            /* Case B: Error Queue Full -> Must drain it to proceed */
            if (errno == ENOBUFS) {
                /* Spin-wait until we clear at least one notification */
                while (read_errqueue(client_fd) == 0 && g_running) {
                    usleep(10); 
                }
                continue;
            }

            /* Case C: Real Error (Broken Pipe) -> Exit */
            break;
        }

        /* FIX: Opportunistic Polling - Do NOT loop here. Just check once. */
        read_errqueue(client_fd);

        ta->bytes_transferred += msg_size;
        ta->messages_transferred++;
    }

    free(contiguous_buf);
    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);

    if (argc < 4) {
        printf("Usage: %s <port> <msg_size> <threads>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    int msg_size = atoi(argv[2]);
    int max_threads = atoi(argv[3]);

    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { 
        .sin_family = AF_INET, 
        .sin_port = htons(port), 
        .sin_addr.s_addr = INADDR_ANY 
    };
    
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { 
        perror("bind"); return 1; 
    }
    listen(sfd, max_threads);
    printf("[Server] A3 (Zero-Copy) Listening on port %d\n", port);

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
            .client_fd = cfd, .server_port = port, .msg_size = msg_size, 
            .thread_id = count, .running = &g_running,
            .bytes_transferred = 0, .messages_transferred = 0
        };

        pthread_create(&threads[count], NULL, server_thread_func, &args[count]);
        count++;
    }

    for (int i = 0; i < count; i++) pthread_join(threads[i], NULL);
    close(sfd);
    return 0;
}