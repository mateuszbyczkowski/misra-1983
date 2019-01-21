#include "header.h"

int main(int argc, char **argv) {

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &initThread);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    msgReceiver = pthread_create(&receiveMessageThread, NULL, receiveMsg, NULL);
    if (msgReceiver) {
        printMessage("unable to create thread", 0, false);
        MPI_Finalize();
    }

    keyListener = pthread_create(&listenerThread, NULL, listenKey, NULL);
    if (keyListener) {
        printMessage("unable to create thread", 0, false);
        MPI_Finalize();
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // first process has token
    if (processId == 0) {
        sendPing(ping, false);
        sendPong(pong, false);
    }

    while (true) {
        if (criticalSection) {
            printMessage("in critical section", 0, false);
            usleep(1000000); // performing complex computations
            printMessage("out of critical section", 0, false);
            dataMutex.lock(); //lock and release
            criticalSection = false;

            if (pingPressed) {
                printMessage("PING lost", ping, true);
            } else {
                sendPing(ping, true);
            }

            dataMutex.unlock();
        } else {
            conditionVariable.wait(uniqueLock);
        }
    }
}

void regenerate(int value) {
    ping = abs(value);
    pong = -ping;
}

void incarnate(int value) {
    ping = abs(value) + 1;
    pong = -ping;
}

void sendPong(int pong, bool saveState) {
    usleep(500000);
    printMessage("PONG sent", pong, true);

    if (saveState)
        m = pong;

    MPI_Send(&pong, MSG_SIZE, MPI_INT, receiver, MSG_PONG, MPI_COMM_WORLD);
}

void sendPing(int ping, bool saveState) {
    printMessage("PING sent", ping, true);

    if (saveState)
        m = ping;

    MPI_Send(&ping, MSG_SIZE, MPI_INT, receiver, MSG_PING, MPI_COMM_WORLD);
}

void *receiveMsg(void *arg) {
    int msg;
    MPI_Status status;

    while (true) {
        MPI_Recv(&msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == MSG_PING) {
            if (msg < abs(m)) {
                printMessage("Old PING received", msg, true);
            } else {
                printMessage("PING received", msg, true);

                dataMutex.lock();

                if (m == msg) { // the same ping means that pong is lost
                    regenerate(msg);
                    printMessage("REGENERATE PONG", pong, true);
                    sendPong(pong, false);
                } else {
                    if (m < msg)
                        regenerate(msg);

                    criticalSection = true;
                }

                dataMutex.unlock();
                conditionVariable.notify_one();
            }
        } else if (status.MPI_TAG == MSG_PONG) {
            if (abs(msg) < abs(m)) {
                printMessage("OLD PONG has arrived (delete action)", msg, true);
            } else {
                printMessage("PONG received", msg, true);
                bool inc = false;

                dataMutex.lock();

                if (criticalSection) {
                    incarnate(msg);
                    inc = true;
                } else if (m == msg) {
                    regenerate(msg);
                    printMessage("REGENERATE PING", ping, true);
                    sendPing(ping, false);
                } else if (abs(m) < abs(msg)) {
                    regenerate(msg);
                }

                dataMutex.unlock();
                conditionVariable.notify_one();

                if (pongPressed) {
                    printMessage("PONG lost", ping, true);
                } else {
                    if (inc)
                        sendPong(pong, false);
                    else
                        sendPong(pong, true);
                }
            }
        }
    }
}

void *listenKey(void *arg) {
    char key;
    while (!pingPressed || !pongPressed) {
        cin >> key;
        if (key == 'p') {
            printMessage("ping pressed", 0, false);
            pingPressed = true;
        } else if (key == 'k') {
            printMessage("pong pressed", 0, false);
            pongPressed = true;
        }
    }
}

void printMessage(const char *message, int value, bool printValue) {
    if (!printValue)
        cout << "Proc number " << processId << ": " << message << endl;
    else
        cout << "Proc number " << processId << ": " << message << " |" << value << "|" << endl;
}
