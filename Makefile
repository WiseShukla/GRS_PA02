# PhD25001
# Makefile for PA02: Analysis of Network I/O Primitives
# Roll No: PhD25001

CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread -lrt

# Compliant Binary Names [cite: 111, 112, 113]
SERVERS = PhD25001_Part_A1_Server PhD25001_Part_A2_Server PhD25001_Part_A3_Server
CLIENTS = PhD25001_Part_A1_Client PhD25001_Part_A2_Client PhD25001_Part_A3_Client
ALL = $(SERVERS) $(CLIENTS)

.PHONY: all clean servers clients

all: $(ALL)

servers: $(SERVERS)

clients: $(CLIENTS)

# A1: Two-copy Implementation (Baseline) [cite: 38]
PhD25001_Part_A1_Server: PhD25001_Part_A1_Server.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

PhD25001_Part_A1_Client: PhD25001_Part_A1_Client.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# A2: One-copy Implementation [cite: 44]
PhD25001_Part_A2_Server: PhD25001_Part_A2_Server.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

PhD25001_Part_A2_Client: PhD25001_Part_A2_Client.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# A3: Zero-copy Implementation [cite: 49]
PhD25001_Part_A3_Server: PhD25001_Part_A3_Server.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

PhD25001_Part_A3_Client: PhD25001_Part_A3_Client.c PhD25001_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(ALL)