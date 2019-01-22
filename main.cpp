#include "header.h"

void handlePing(int msg);

void handlePong(int msg);

int main(int argc, char **argv) {

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &initThread);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    msgReceiver = pthread_create(&receiveMessageThread, NULL, receiveMsg, NULL);
    if (msgReceiver) {
        printMessage("can't create thread", 0, false);
        MPI_Finalize();
    }

    keyListener = pthread_create(&listenerThread, NULL, listenKey, NULL);
    if (keyListener) {
        printMessage("can't create thread", 0, false);
        MPI_Finalize();
    }

    receiver = (processId + 1) % size;

    MPI_Barrier(MPI_COMM_WORLD);

    // first process has token
    if (processId == 0) {
        sendPing(ping, false);
        sendPong(pong, false);
    }

    while (true) {
        if (isInCriticalSection) {
            printMessage("-> in critical section", 0, false);
            usleep(1000000); // performing complex computations
            printMessage("<- out of critical section", 0, false);
            dataMutex.lock(); //lock and release
            isInCriticalSection = false;

            if (pingPressed) {
                printMessage("PING LOST", ping, true);
                pingPressed = false;
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

    if (saveState) {
        m = pong;
    }

    MPI_Send(&pong, MSG_SIZE, MPI_INT, receiver, MSG_PONG, MPI_COMM_WORLD);
}

void sendPing(int ping, bool saveState) {
    printMessage("PING sent", ping, true);

    if (saveState) {
        m = ping;
    }

    MPI_Send(&ping, MSG_SIZE, MPI_INT, receiver, MSG_PING, MPI_COMM_WORLD);
}

void *receiveMsg(void *arg) {
    int msg;
    MPI_Status status;

    while (true) {
        MPI_Recv(&msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == MSG_PING) {
            handlePing(msg);
        } else if (status.MPI_TAG == MSG_PONG) {
            handlePong(msg);
        }
    }
}

void handlePing(int msg) {
    if (msg < abs(m)) {
        printMessage("Old PING received", msg, true);
        return;
    } else {
        printMessage("PING received", msg, true);

        dataMutex.lock();

        if (m == msg) { // the same ping means that pong is lost
            regenerate(msg);
            printMessage("REGENERATE PONG", pong, true);
            sendPong(pong, false);
        } else {
            if (m < msg) {
                regenerate(msg);
            }

            isInCriticalSection = true;
        }

        dataMutex.unlock();
        conditionVariable.notify_one();
    }
}

void handlePong(int msg) {
    if (abs(msg) < abs(m)) {
        printMessage("OLD PONG has arrived (delete action)", msg, true);
        return;
    }

    printMessage("PONG received", msg, true);
    bool incarnation = false;

    dataMutex.lock();

    if (isInCriticalSection) {
        incarnate(msg);
        printMessage("INCARNATION", 0, false);
        incarnation = true;
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
        printMessage("PONG LOST", ping, true);
        pongPressed = false;
    } else {
        if (incarnation) {
            sendPong(pong, false);
        } else {
            sendPong(pong, true);
        }
    }

}

void *listenKey(void *arg) {
    char key;
    while (true) {
        cin >> key;
        if (key == 'p') {
            if (pongPressedTimes > 0) {
                printMessage("you are in PONG lost only mode", 0, false);
            } else {
                printMessage("ping pressed", 0, false);
                pingPressedTimes++;
                pingPressed = true;
            }
        } else if (key == 'k') {
            if (pingPressedTimes > 0) {
                printMessage("you are in PING lost only mode", 0, false);
            } else {
                printMessage("pong pressed", 0, false);
                pongPressed = true;
                pongPressedTimes++;
            }
        }
    }
}

void printMessage(const char *message, int value, bool printValue) {
    if (!printValue) {
        cout << "Proc number " << processId << ": " << message << endl;
    } else {
        cout << "Proc number " << processId << ": " << message << " |" << value << "|" << endl;
    }
}
