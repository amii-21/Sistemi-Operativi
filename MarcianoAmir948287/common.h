#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define TYPE_PRIMI 0
#define TYPE_SECONDI 1
#define TYPE_COFFEE 2
#define TYPE_CASSA 3

#define FTOK_PATH "Makefile"
#define SHM_KEY_ID 65
#define SEM_KEY_ID 66
#define MSG_PRIMI_ID 67
#define MSG_SECONDI_ID 68
#define MSG_COFFEE_ID 69
#define MSG_CASSA_ID 70

typedef struct {
    int SIM_DURATION;
    long N_NANO_SECS;
    int NOF_WORKERS;
    int NOF_USERS;
    int NOF_TABLE_SEATS;
    int MAX_PORZIONI_PRIMI;
    int MAX_PORZIONI_SECONDI;
    int AVG_REFILL_PRIMI;
    int AVG_REFILL_SECONDI;
    int OVERLOAD_THRESHOLD;
    int AVG_SRVC_PRIMI;
    int AVG_SRVC_SECONDI;
    int AVG_SRVC_COFFEE;
    int AVG_SRVC_CASSA;
    int PRICE_PRIMI;
    int PRICE_SECONDI;
    int PRICE_COFFEE;
} Config;

typedef struct {
    int current_day;
    int current_minute; 
    int sim_ended;
    int ready_count;

    int porzioni_primi[2];
    int porzioni_secondi[2];
    
    // Statistiche Giornaliere
    int users_served_today;
    int users_rejected_today;
    int plates_distributed_today[3];
    int leftover_today[2];
    int revenue_today;
    int pause_today;
    double wait_time_today_stazione[4];
    long service_count_today_stazione[4];
    
    // Statistiche Totali
    int users_served_total;
    int users_rejected_total;
    int plates_distributed_total[3];
    int leftover_total[2];
    int revenue_total;
    int pauses_total;
    double wait_time_total_stazione[4];
    long service_count_total_stazione[4];
    int active_operators[4]; 
} SharedMemory;

typedef struct {
    long mtype; 
    int pid_sender;
    int food_type;
    int quantity;
    struct timespec timestamp;
} Message;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void sem_wait_op(int semid, int sem_num);
void sem_signal_op(int semid, int sem_num);

#define SEM_MUTEX_SHM 0  
#define SEM_TABLES 1      
#define SEM_SEATS_PRIMI 2 
#define SEM_SEATS_SECONDI 3
#define SEM_SEATS_COFFEE 4
#define SEM_SEATS_CASSA 5
#define SEM_TOTAL_NUM 6

#endif