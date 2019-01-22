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

using namespace std;

int MSG_PING = 100; // MPI communication variables
int MSG_PONG = 101;
int MSG_SIZE = 1;

int ping = 1; // algorithm variables
int pong = -1;
int m = 0;

int initThread; // MPI variables
int size;
int processId;
int receiver;

int msgReceiver; // thread descriptors
int keyListener;

int pingPressed = false;
int pongPressed = false;

int pingPressedTimes = 0; // key listeners
int pongPressedTimes = 0;

pthread_t receiveMessageThread; // threads
pthread_t listenerThread;

mutex conditionalVariableMutex; // mutual exclusion variables
mutex dataMutex;
condition_variable conditionVariable;

bool isInCriticalSection = false;

unique_lock<mutex> uniqueLock(conditionalVariableMutex);

void *receiveMsg(void *arg);

void *listenKey(void *arg);

void incarnate(int value);

void regenerate(int value);

void printMessage(const char *message, int value, bool printValue);

void sendPong(int pong, bool saveState);

void sendPing(int ping, bool saveState);

#endif //PING_PONG_HEADER_H
