//
// Created by ubuntu on 19.01.19.
//

#ifndef PING_PONG_HEADER_H
#define PING_PONG_HEADER_H

#include "mpi.h"
#include <pthread.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <climits>
#include <getopt.h>
#include "header.h"

#define MSG_PING 100
#define MSG_PONG 101
#define MSG_SIZE 1
#define CRITICAL_SECTION_SLEEP_TIME 500000
#define PONG_SEND_DELAY 500000
#define PING_LOSS_CHANCE 10
#define PONG_LOSS_CHANCE 10

using namespace std;

static int ping_flag = 0;
static int pong_flag = 0;

static struct option long_options[] = {
        {"ping", no_argument, &ping_flag, 1},
        {"pong", no_argument, &pong_flag, 1},
        {0, 0,                0,          0}
};

int receiver;
int ping = 1;
int pong = -1;
int size;
int m = 0;
int provided;
int node_id;
int rc;
pthread_t recv_msg_thread;
mutex cond_var_mut, data_mutex;
condition_variable cond_var;
bool critical_section = false;
unique_lock<mutex> lk(cond_var_mut);

void *recv_msg(void *arg);

void incarnate(int val);

void regenerate(int val);

void print_debug_message(const char *message, int value = INT_MIN);

void send_pong(int pong, bool save_state);

void send_ping(int ping, bool save_state);

void check_opt(int argc, char **argv);

#endif //PING_PONG_HEADER_H
