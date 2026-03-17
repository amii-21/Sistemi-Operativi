#include "common.h"

extern void load_config(const char *filename, Config *cfg);

int main(int argc, char *argv[]) {
    int type = atoi(argv[1]);
    Config cfg; load_config(argv[2], &cfg);
    int shmid = shmget(ftok(FTOK_PATH, SHM_KEY_ID), sizeof(SharedMemory), 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    int semid = semget(ftok(FTOK_PATH, SEM_KEY_ID), SEM_TOTAL_NUM, 0666);
    
    int q_key_ids[4] = {MSG_PRIMI_ID, MSG_SECONDI_ID, MSG_COFFEE_ID, MSG_CASSA_ID};
    int base_times[4] = {cfg.AVG_SRVC_PRIMI, cfg.AVG_SRVC_SECONDI, cfg.AVG_SRVC_COFFEE, cfg.AVG_SRVC_CASSA};
    int var_pcts[4] = {50, 50, 80, 20};
    int sem_seats[4] = {SEM_SEATS_PRIMI, SEM_SEATS_SECONDI, SEM_SEATS_COFFEE, SEM_SEATS_CASSA};

    int msgid = msgget(ftok(FTOK_PATH, q_key_ids[type]), 0666);
    srand(getpid() ^ time(NULL));

    int pause_fatte = 0;
    const int MAX_PAUSE = 3;
    int last_day= 0;

    sem_wait_op(semid, SEM_MUTEX_SHM);
    shm->ready_count++;
    sem_signal_op(semid, SEM_MUTEX_SHM);

    sem_wait_op(semid, sem_seats[type]);
    sem_wait_op(semid, SEM_MUTEX_SHM); 
    shm->active_operators[type]++; 
    sem_signal_op(semid, SEM_MUTEX_SHM);

    

    Message msg;
    while(!shm->sim_ended) {
        // Reset Pause Giornaliero
        if(shm->current_day> last_day) {
            last_day = shm->current_day;
            pause_fatte = 0;
        }

        // Pausa
        if(pause_fatte < MAX_PAUSE && rand()%100 < 5) {
            sem_wait_op(semid, SEM_MUTEX_SHM);
            //Pausa solo se rimane almento un operatore attivo
            if(shm->active_operators[type] > 1) {
                shm->active_operators[type]--; 
                shm->pause_today++;
                //shm->pauses_total++; 
                sem_signal_op(semid, SEM_MUTEX_SHM);
                //Esegue pausa
                sem_signal_op(semid, sem_seats[type]);
                usleep(cfg.N_NANO_SECS/1000 * 20); 
                sem_wait_op(semid, sem_seats[type]);
                //Riprende lavoro
                sem_wait_op(semid, SEM_MUTEX_SHM); 
                shm->active_operators[type]++;
                sem_signal_op(semid, SEM_MUTEX_SHM);
                pause_fatte++;
            } else sem_signal_op(semid, SEM_MUTEX_SHM);
        }

        if(msgrcv(msgid, &msg, sizeof(Message)-sizeof(long), 1, IPC_NOWAIT) != -1) {
            struct timespec now; 
            clock_gettime(CLOCK_MONOTONIC, &now);
            double wait = (now.tv_sec - msg.timestamp.tv_sec) * 1e9 + (now.tv_nsec - msg.timestamp.tv_nsec);
            
            sem_wait_op(semid, SEM_MUTEX_SHM);
            shm->wait_time_today_stazione[type] += wait; 
            shm->service_count_today_stazione[type]++;
            shm->wait_time_total_stazione[type] += wait;
            shm->service_count_total_stazione[type]++;
            sem_signal_op(semid, SEM_MUTEX_SHM);

            int var = (base_times[type] * var_pcts[type]) / 100; // varianza assoluta
            int random_part=(var >0) ? (rand()%(var*2+1)) - var : 0; // parte casuale tra -var e +var
            int act = base_times[type] + random_part;// tempo di servizio attuale
            if(act < 0) act = 0;
            usleep(act * (cfg.N_NANO_SECS/1000000));
            msg.mtype = msg.pid_sender; 
            msgsnd(msgid, &msg, sizeof(Message)-sizeof(long), 0);
        } else usleep(1000);
    }
    shmdt(shm);
    return 0;
}