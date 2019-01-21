#include "header.h"

int main(int argc, char **argv) {

    check_opt(argc, argv);

    srand(time(NULL));

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    int ok = 5;
    MPI_Comm_rank(MPI_COMM_WORLD, &ok);
    MPI_Comm_rank(MPI_COMM_WORLD, &node_id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    rc = pthread_create(&recv_msg_thread, NULL, recv_msg, NULL);
    if (rc) {
        print_debug_message("unable to create thread");
        MPI_Finalize();
    }

    receiver = (node_id + 1) % size;
    cout << "size is " << size << " current is " << ok << endl;

    MPI_Barrier(MPI_COMM_WORLD);

    if (node_id == 0) {
        send_ping(ping, false);
        send_pong(pong, false);
    }

    while (true) {
        if (critical_section) {
            print_debug_message("entered critical section");
            usleep(CRITICAL_SECTION_SLEEP_TIME);
            print_debug_message("left critical section");
            data_mutex.lock();
            critical_section = false;

            if (ping_flag && !pong_flag && (rand() % 100 > 100 - PING_LOSS_CHANCE)) {
                print_debug_message("PING lost", ping);
            } else {
                send_ping(ping, true);
            }

            data_mutex.unlock();
        } else {
            cond_var.wait(lk);
        }
    }

    MPI_Finalize();
}

void regenerate(int val) {
    ping = abs(val);
    pong = -ping;
}

void incarnate(int val) {
    ping = abs(val) + 1;
    pong = -ping;
}

void print_debug_message(const char *message, int value) {
    if (value == INT_MIN)
        cout << "Node [" << node_id << "]: " << message << endl;
    else
        cout << "Node [" << node_id << "]: " << message << " [" << value << "]" << endl;
}

void *recv_msg(void *arg) {
    int msg;
    MPI_Status status;

    while (true) {
        MPI_Recv(&msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == MSG_PING) {
            if (msg < abs(m)) {
                print_debug_message("OLD PING has arrived (delete action)", msg);
            } else {
                print_debug_message("PING received", msg);

                data_mutex.lock();

                if (m == msg) {
                    regenerate(msg);
                    print_debug_message("REGENERATE PONG", pong);
                    send_pong(pong, false);
                } else {
                    if (m < msg)
                        regenerate(msg);

                    critical_section = true;
                }

                data_mutex.unlock();
                cond_var.notify_one();
            }
        } else if (status.MPI_TAG == MSG_PONG) {
            if (abs(msg) < abs(m)) {
                print_debug_message("OLD PONG has arrived (delete action)", msg);
            } else {
                print_debug_message("PONG received", msg);
                bool inc = false;

                data_mutex.lock();

                if (critical_section) {
                    incarnate(msg);
                    inc = true;
                } else if (m == msg) {
                    regenerate(msg);
                    print_debug_message("REGENERATE PING", ping);
                    send_ping(ping, false);
                } else if (abs(m) < abs(msg)) {
                    regenerate(msg);
                }

                data_mutex.unlock();
                cond_var.notify_one();

                if (pong_flag && !ping_flag && (rand() % 100 > 100 - PONG_LOSS_CHANCE)) {
                    print_debug_message("PONG lost", ping);
                } else {
                    if (inc)
                        send_pong(pong, false);
                    else
                        send_pong(pong, true);
                }
            }
        }
    }

    pthread_exit(NULL);
}

void check_opt(int argc, char **argv) {
    char c;

    while (true) {
        int option_index = 0;

        c = getopt_long(argc, argv, ":", long_options, &option_index);

        if (c == -1)
            break;
    }

    if (ping_flag && pong_flag) {
        cout << "Use only one option switch: --ping or --pong." << endl;
        exit(-1);
    }
}

void send_pong(int pong, bool save_state) {
    usleep(PONG_SEND_DELAY);
    print_debug_message("PONG sent", pong);

    if (save_state)
        m = pong;

    MPI_Send(&pong, MSG_SIZE, MPI_INT, receiver, MSG_PONG, MPI_COMM_WORLD);
}

void send_ping(int ping, bool save_state) {
    print_debug_message("PING sent", ping);

    if (save_state)
        m = ping;

    MPI_Send(&ping, MSG_SIZE, MPI_INT, receiver, MSG_PING, MPI_COMM_WORLD);
}
