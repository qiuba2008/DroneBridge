//
// Created by Wolfgang Christl on 30.11.17.
// This file is part of DroneBridge
// https://github.com/seeul8er/DroneBridge
//

#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include "parameter.h"
#include "../common/db_protocol.h"
#include "i6S.h"
#include "../common/db_raw_send.h"
#include "tx.h"

int detect_RC(int new_Joy_IF) {
    int fd;
    char interface_joystick[500];
    char path[] = "/dev/input/js";
    sprintf(interface_joystick, "%s%d", path, new_Joy_IF);
    printf("DB_CONTROL_GROUND: Waiting for a RC to be detected on: %s\n", interface_joystick);
    do {
        usleep(250);
        fd = open(interface_joystick, O_RDONLY | O_NONBLOCK);
    } while (fd < 0);
    return fd;
}

int main(int argc, char *argv[]) {
    atexit(close_socket_send);
    char ifName[IFNAMSIZ], RC_name[128];
    char calibrate_comm[500];
    char comm_id[10];
    unsigned char comm_id_bytes[4];
    int Joy_IF, c, bitrate_op, frame_type, rc_protocol;
    char db_mode = 'm';

    // Command Line processing
    Joy_IF = JOY_INTERFACE;
    frame_type = 1;
    rc_protocol = 2;
    bitrate_op = DEFAULT_BITRATE_OPTION;
    strcpy(comm_id, DEFAULT_COMMID);
    strcpy(calibrate_comm, DEFAULT_i6S_CALIBRATION);
    strcpy(ifName, DEFAULT_IF);
    opterr = 0;
    while ((c = getopt(argc, argv, "n:j:m:b:g:v:c:a:")) != -1) {
        switch (c) {
            case 'n':
                strncpy(ifName, optarg, IFNAMSIZ);
                break;
            case 'j':
                Joy_IF = (int) strtol(optarg, NULL, 10);
                break;
            case 'm':
                db_mode = *optarg;
                break;
            case 'b':
                bitrate_op = (int) strtol(optarg, NULL, 10);
                break;
            case 'g':
                strcpy(calibrate_comm, optarg);
                break;
            case 'v':
                rc_protocol = (int) strtol(optarg, NULL, 10);
                break;
            case 'c':
                strncpy(comm_id, optarg, 10);
                break;
            case 'a':
                frame_type = (int) strtol(optarg, NULL, 10);
                break;
            case '?':
                printf("Use following commandline arguments.\n");
                printf("-n network interface for long range \n"
                               "-j number of joystick interface of RC \n"
                               "-m mode: <w|m> for wifi or monitor\n"
                               "-g a command to calibrate the joystick. Gets executed on initialisation\n"
                               "-v Protocol [1|2|3]: 1 = Betaflight/Cleanflight [MSPv1]; 2 = iNAV (default) [MSPv2]; 3 = MAVLink\n"
                               "-a frame type [1|2] <1> for Ralink und <2> for Atheros chipsets\n"
                               "-c the communication ID (same on drone and groundstation)\n"
                               "-b bitrate: \n\t1 = 2.5Mbit\n\t2 = 4.5Mbit\n\t3 = 6Mbit\n\t4 = 12Mbit (default)\n\t"
                               "5 = 18Mbit\n(bitrate option only supported with Ralink chipsets)\n");
                return -1;
            default:
                abort();
        }
    }
    sscanf(comm_id, "%2hhx%2hhx%2hhx%2hhx", &comm_id_bytes[0], &comm_id_bytes[1], &comm_id_bytes[2], &comm_id_bytes[3]);
    printf("DB_CONTROL_GROUND: Interface: %s Communication ID: %02x %02x %02x %02x MSP: v%i\n", ifName, comm_id_bytes[0],
           comm_id_bytes[1], comm_id_bytes[2], comm_id_bytes[3], rc_protocol);

    if (open_socket_sending(ifName, comm_id_bytes, db_mode, bitrate_op, frame_type, DB_DIREC_DRONE) < 0) {
        printf("DB_CONTROL_GROUND: Could not open socket\n");
        exit(-1);
    }
    conf_rc_protocol(rc_protocol);
    int sock_fd = detect_RC(Joy_IF);
    if (ioctl(sock_fd, JSIOCGNAME(sizeof(RC_name)), RC_name) < 0)
        strncpy(RC_name, "Unknown", sizeof(RC_name));
    close(sock_fd); // We reopen in the RC specific file. Only opened in here to get the name of the controller
    if (strcmp(i6S_descriptor, RC_name) == 0){
        printf("DB_CONTROL_GROUND: Choosing i6S-Config\n");
        i6S(Joy_IF, calibrate_comm);
    } else {
        printf("DB_CONTROL_GROUND: Your RC \"%s\" is currently not supported. Closing.\n", RC_name);
    }
    return 0;
}