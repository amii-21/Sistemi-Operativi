#include "common.h"

extern void load_config(const char *filename, Config *cfg);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <config_file>\n", argv[0]);
        exit(1);
    }

    Config cfg;
    load_config(argv[1], &cfg);

    //Inizializzazione IPC
    key_t shm_key = ftok(FTOK_PATH, SHM_KEY_ID);
    key_t sem_key = ftok(FTOK_PATH, SEM_KEY_ID);
    
    // Creazione Code Messaggi
    int msgids[4];
    int q_keys[4] = {MSG_PRIMI_ID, MSG_SECONDI_ID, MSG_COFFEE_ID, MSG_CASSA_ID};
    for(int i=0; i<4; i++) {
        msgids[i] = msgget(ftok(FTOK_PATH, q_keys[i]), IPC_CREAT | 0666);
        msgctl(msgids[i], IPC_RMID, NULL);
        msgids[i] = msgget(ftok(FTOK_PATH, q_keys[i]), IPC_CREAT | 0666);
    }

    // Creazione SHM
    int shmid = shmget(shm_key, sizeof(SharedMemory), IPC_CREAT | 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    memset(shm, 0, sizeof(SharedMemory));
    
    // Creazione Semafori
    int semid = semget(sem_key, SEM_TOTAL_NUM, IPC_CREAT | 0666);
    semctl(semid, SEM_MUTEX_SHM, SETVAL, (union semun){.val = 1});
    semctl(semid, SEM_TABLES, SETVAL, (union semun){.val = cfg.NOF_TABLE_SEATS});

    int wk[4] = {1, 1, 1, 1}; 
    int available = cfg.NOF_WORKERS - 4;
    for(int i=0; i<available; i++) wk[i%4]++;

    semctl(semid, SEM_SEATS_PRIMI, SETVAL, (union semun){.val = wk[0]});
    semctl(semid, SEM_SEATS_SECONDI, SETVAL, (union semun){.val = wk[1]});
    semctl(semid, SEM_SEATS_COFFEE, SETVAL, (union semun){.val = wk[2]});
    semctl(semid, SEM_SEATS_CASSA, SETVAL, (union semun){.val = wk[3]});

    printf("[RESPONSABILE] Configurazione: %d Operatori (P:%d S:%d C:%d $: %d)\n", 
           cfg.NOF_WORKERS, wk[0], wk[1], wk[2], wk[3]);

    //Operatori 
    for(int type=0; type<4; type++) {
        for(int k=0; k < wk[type]; k++) {
            if (fork() == 0) {
                char type_str[2]; sprintf(type_str, "%d", type);
                execl("./operator", "operator", type_str, argv[1], NULL);
                perror("Exec operator failed"); exit(1);
            }
        }
    }

    //Utenti 
    for(int i=0; i < cfg.NOF_USERS; i++) {
        if (fork() == 0) {
            execl("./user", "user", argv[1], NULL);
            perror("Exec user failed"); exit(1);
        }
    }

    printf("[RESPONSABILE] Attesa avvio processi...\n");
    int total_procs = cfg.NOF_WORKERS + cfg.NOF_USERS;
    while(1) {
        sem_wait_op(semid, SEM_MUTEX_SHM);
        if (shm->ready_count >= total_procs) {
            sem_signal_op(semid, SEM_MUTEX_SHM);
            break;
        }
        sem_signal_op(semid, SEM_MUTEX_SHM);
        usleep(10000);
    }
    printf("[RESPONSABILE] Tutti pronti. Inizio Simulazione per %d giorni.\n", cfg.SIM_DURATION);

    int totale_pause_accumulate = 0;
    // CICLO GIORNALIERO 
    for (int day = 0; day < cfg.SIM_DURATION; day++) {
        // Inizio Giorno: Reset e Refill
        sem_wait_op(semid, SEM_MUTEX_SHM);
        shm->current_day = day + 1;
        shm->current_minute = 0;
        
        // Reset contatori giornalieri
        shm->users_served_today = 0;
        shm->users_rejected_today = 0;
        shm->revenue_today = 0;
        shm->pause_today = 0; 
        for(int i=0; i<3; i++) shm->plates_distributed_today[i] = 0;
        for(int i=0; i<4; i++) {
            shm->wait_time_today_stazione[i] = 0;
            shm->service_count_today_stazione[i] = 0;
        }

        // Refill Iniziale
        shm->porzioni_primi[0] = cfg.AVG_REFILL_PRIMI / 2;
        shm->porzioni_primi[1] = cfg.AVG_REFILL_PRIMI / 2;
        shm->porzioni_secondi[0] = cfg.AVG_REFILL_SECONDI / 2;
        shm->porzioni_secondi[1] = cfg.AVG_REFILL_SECONDI / 2;
        sem_signal_op(semid, SEM_MUTEX_SHM);

        // Simulazione Minuti
        for (int min = 0; min < 600; min++) { // 10 ore fittizie
            struct timespec req = {0, cfg.N_NANO_SECS}; 
            nanosleep(&req, NULL); 

            sem_wait_op(semid, SEM_MUTEX_SHM);
            shm->current_minute = min;
            
            // Refill periodico (ogni 10 min)
            if (min % 10 == 0) {
                for(int i=0; i<2; i++) {
                    if (shm->porzioni_primi[i] < cfg.MAX_PORZIONI_PRIMI) shm->porzioni_primi[i]++;
                    if (shm->porzioni_secondi[i] < cfg.MAX_PORZIONI_SECONDI) shm->porzioni_secondi[i]++;
                }
            }
            sem_signal_op(semid, SEM_MUTEX_SHM);
        }

        // FINE GIORNATA
        sem_wait_op(semid, SEM_MUTEX_SHM);
        
        int pause_di_oggi = shm->pause_today;
        totale_pause_accumulate += pause_di_oggi;

        // Calcolo avanzi
        shm->leftover_today[0] = shm->porzioni_primi[0] + shm->porzioni_primi[1];
        shm->leftover_today[1] = shm->porzioni_secondi[0] + shm->porzioni_secondi[1];
        
        printf("\n================ REPORT GIORNO %d ================\n", day+1);
        printf("| UTENTI\n");
        printf("| - Serviti: %d\n", shm->users_served_today);
        printf("| - Rifiutati: %d (Soglia Overload: %d)\n", shm->users_rejected_today, cfg.OVERLOAD_THRESHOLD);
        
        printf("| ECONOMIA\n");
        printf("| - Incasso: %d EUR\n", shm->revenue_today);
        
        printf("| CIBO\n");
        printf("| - Distribuiti: Primi %d, Secondi %d, Caffe %d\n", 
               shm->plates_distributed_today[0], shm->plates_distributed_today[1], shm->plates_distributed_today[2]);
        printf("| - Avanzati (Leftover): Primi %d, Secondi %d\n", 
               shm->leftover_today[0], shm->leftover_today[1]);

        printf("| TEMPI DI ATTESA MEDI (ms)\n");
        const char *labels[4] = {"Primi", "Secondi", "Caffe", "Cassa"};
        for(int i=0; i<4; i++) {
            double avg = (shm->service_count_today_stazione[i] > 0) ? 
                         (shm->wait_time_today_stazione[i] / shm->service_count_today_stazione[i]) / 1000000.0 : 0.0;
            printf("| - %s: %.2f ms (su %ld servizi)\n", labels[i], avg, shm->service_count_today_stazione[i]);
        }
        
        printf("| OPERATORI\n");
        printf("| - Pause effettuate oggi: %d\n", pause_di_oggi);

        // Aggiornamento Totali per Statistiche Finali
        shm->users_served_total += shm->users_served_today;
        shm->users_rejected_total += shm->users_rejected_today;
        shm->revenue_total += shm->revenue_today;
        shm->plates_distributed_total[0] += shm->plates_distributed_today[0];
        shm->plates_distributed_total[1] += shm->plates_distributed_today[1];
        shm->plates_distributed_total[2] += shm->plates_distributed_today[2];
        shm->leftover_total[0] += shm->leftover_today[0];
        shm->leftover_total[1] += shm->leftover_today[1];
        shm->pause_today= 0;

        // Controllo Overload
        if (shm->users_rejected_today > cfg.OVERLOAD_THRESHOLD) {
            printf("\n!!! ALLARME: OVERLOAD RILEVATO (Rifiutati %d > %d) !!!\n", 
                   shm->users_rejected_today, cfg.OVERLOAD_THRESHOLD);
            printf("Terminazione anticipata della simulazione.\n");
            shm->sim_ended = 1;       
            sem_signal_op(semid, SEM_MUTEX_SHM); 
            break;                  
        }
        sem_signal_op(semid, SEM_MUTEX_SHM);
    }     

    // STATISTICHE FINALI
    sem_wait_op(semid, SEM_MUTEX_SHM);
    printf("\n\n################ STATISTICHE FINALI ################\n");
    printf("Giorni simulati: %d\n", shm->current_day);
    printf("Totale Utenti Serviti: %d (Media: %.1f/giorno)\n", 
           shm->users_served_total, (float)shm->users_served_total/shm->current_day);
    printf("Totale Utenti Rifiutati: %d\n", shm->users_rejected_total);
    printf("Totale Incasso: %d EUR\n", shm->revenue_total);
    printf("Totale Piatti Distribuiti: P:%d S:%d C:%d\n", 
           shm->plates_distributed_total[0], shm->plates_distributed_total[1], shm->plates_distributed_total[2]);
    printf("Totale Pause Operatori: %d\n", totale_pause_accumulate);
    
    printf("Tempi di Attesa Medi Complessivi:\n");
    const char *labels[4] = {"Primi", "Secondi", "Caffe", "Cassa"};
    for(int i=0; i<4; i++) {
        double avg = (shm->service_count_total_stazione[i] > 0) ? 
                     (shm->wait_time_total_stazione[i] / shm->service_count_total_stazione[i]) / 1000000.0 : 0.0;
        printf(" - %s: %.2f ms\n", labels[i], avg);
    }
    printf("####################################################\n");
    shm->sim_ended = 1;
    sem_signal_op(semid, SEM_MUTEX_SHM);

    // Pulizia
    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
    for(int i=0; i<4; i++) msgctl(msgids[i], IPC_RMID, NULL);

    kill(0, SIGTERM); 
    return 0;
}