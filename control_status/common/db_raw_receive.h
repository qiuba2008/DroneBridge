//
// Created by Wolfgang Christl on 30.11.17.
// This file is part of DroneBridge
// https://github.com/seeul8er/DroneBridge
//

#ifndef STATUS_DB_RECEIVE_H
#define STATUS_DB_RECEIVE_H

#include <stdint.h>
#include <net/if.h>

int set_socket_nonblocking(int the_socket);
int set_socket_timeout(int the_socketfd, int time_out_s ,int time_out_us);
int open_receive_socket(char newifName[IFNAMSIZ], char new_mode, uint8_t comm_id[6], uint8_t new_direction,
                        uint8_t new_port);

#endif //STATUS_DB_RECEIVE_H
