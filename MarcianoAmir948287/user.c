#include "common.h"

extern void load_config(const char *filename, Config *cfg);

// Struttura memorizzare il menu letto da file
typedef struct {
    char primi[2][64];
    int n_primi;
    char secondi[2][64];
    int n_secondi;
} Menu;

// leggere il menu
void load_menu(const char *filename, Menu *m) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Errore apertura menu.txt, uso default");
        strcpy(m->primi[0], "Primo_Standard_A"); strcpy(m->primi[1], "Primo_Standard_B");
        strcpy(m->secondi[0], "Secondo_Standard_A"); strcpy(m->secondi[1], "Secondo_Standard_B");
        return;
    }
    
    char line[128], type[32], name[64];
    m->n_primi = 0; m->n_secondi = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%s %s", type, name) == 2) {
            if (strcmp(type, "PRIMO") == 0 && m->n_primi < 2) {
                strncpy(m->primi[m->n_primi], name, 63);
                m->n_primi++;
            } else if (strcmp(type, "SECONDO") == 0 && m->n_secondi < 2) {
                strncpy(m->secondi[m->n_secondi], name, 63);
                m->n_secondi++;
            }
        }
    }
    fclose(f);
}

void req(int mid, int type, struct timespec *ts) {
    Message m;
    m.mtype = 1;
    m.pid_sender = getpid();
    m.food_type = type;
    m.quantity = 1;
    clock_gettime(CLOCK_MONOTONIC, &m.timestamp);
    
    // Invio richiesta
    if (msgsnd(mid, &m, sizeof(Message)-sizeof(long), 0) == -1) return;
    // Attesa risposta
    msgrcv(mid, &m, sizeof(Message)-sizeof(long), getpid(), 0);
    if(ts) *ts = m.timestamp; 
}

int main(int argc, char *argv[]) {
    if (argc < 2) exit(1);
    
    Config cfg; 
    load_config(argv[1], &cfg);
    
    // Caricamento Menu
    Menu my_menu;
    load_menu("menu.txt", &my_menu);

    // IPC Setup
    int shmid = shmget(ftok(FTOK_PATH, SHM_KEY_ID), sizeof(SharedMemory), 0666);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    int semid = semget(ftok(FTOK_PATH, SEM_KEY_ID), SEM_TOTAL_NUM, 0666);
    int qs[4] = { 
        msgget(ftok(FTOK_PATH, MSG_PRIMI_ID), 0666), 
        msgget(ftok(FTOK_PATH, MSG_SECONDI_ID), 0666),
        msgget(ftok(FTOK_PATH, MSG_COFFEE_ID), 0666), 
        msgget(ftok(FTOK_PATH, MSG_CASSA_ID), 0666) 
    };

    srand(getpid() ^ time(NULL));

    sem_wait_op(semid, SEM_MUTEX_SHM); 
    shm->ready_count++; 
    sem_signal_op(semid, SEM_MUTEX_SHM);

    int last_day = 0;
    while(!shm->sim_ended) {
        if(shm->current_day > last_day) {
            last_day = shm->current_day;
            int dishes = 0;
            int cost = 0;

            // SCELTA MENU
            int chosen_primo_idx = rand() % 2;   // 0 o 1
            int chosen_secondo_idx = rand() % 2; // 0 o 1
            
            //  PRIMI
            int got_primo = 0;
            sem_wait_op(semid, SEM_MUTEX_SHM);
            if (shm->porzioni_primi[chosen_primo_idx] > 0) {
                shm->porzioni_primi[chosen_primo_idx]--;
                got_primo = 1;
            } else if (shm->porzioni_primi[1 - chosen_primo_idx] > 0) {
                chosen_primo_idx = 1 - chosen_primo_idx;
                shm->porzioni_primi[chosen_primo_idx]--;
                got_primo = 1;
            }
            sem_signal_op(semid, SEM_MUTEX_SHM);

            if (got_primo) {
                req(qs[TYPE_PRIMI], chosen_primo_idx, NULL);
                cost += cfg.PRICE_PRIMI;
                dishes++;
                sem_wait_op(semid, SEM_MUTEX_SHM); 
                shm->plates_distributed_today[0]++; 
                sem_signal_op(semid, SEM_MUTEX_SHM);
            }

            // SECONDI
            int got_secondo = 0;
            sem_wait_op(semid, SEM_MUTEX_SHM);
            if (shm->porzioni_secondi[chosen_secondo_idx] > 0) {
                shm->porzioni_secondi[chosen_secondo_idx]--;
                got_secondo = 1;
            } else if (shm->porzioni_secondi[1 - chosen_secondo_idx] > 0) {
                chosen_secondo_idx = 1 - chosen_secondo_idx;
                shm->porzioni_secondi[chosen_secondo_idx]--;
                got_secondo = 1;
            }
            sem_signal_op(semid, SEM_MUTEX_SHM);

            if (got_secondo) {
                req(qs[TYPE_SECONDI], chosen_secondo_idx, NULL);
                cost += cfg.PRICE_SECONDI;
                dishes++;
                sem_wait_op(semid, SEM_MUTEX_SHM); 
                shm->plates_distributed_today[1]++; 
                sem_signal_op(semid, SEM_MUTEX_SHM);
            }

            if (dishes == 0) { 
                sem_wait_op(semid, SEM_MUTEX_SHM); 
                shm->users_rejected_today++; 
                sem_signal_op(semid, SEM_MUTEX_SHM); 
                continue; 
            }

            // Caffè e Cassa
            if(rand() % 2) { 
                req(qs[TYPE_COFFEE], 0, NULL); 
                cost += cfg.PRICE_COFFEE; 

                sem_wait_op(semid, SEM_MUTEX_SHM); 
                shm->plates_distributed_today[2]++; 
                sem_signal_op(semid, SEM_MUTEX_SHM);
            }
            req(qs[TYPE_CASSA], 0, NULL); // Paga costo totale

            // Aggiorna incasso
            sem_wait_op(semid, SEM_MUTEX_SHM); 
            shm->revenue_today += cost; 
            sem_signal_op(semid, SEM_MUTEX_SHM);

            // Mangia
            sem_wait_op(semid, SEM_TABLES);
            usleep(cfg.N_NANO_SECS / 1000 * 10 * dishes); 
            sem_signal_op(semid, SEM_TABLES);

            sem_wait_op(semid, SEM_MUTEX_SHM); 
            shm->users_served_today++; 
            sem_signal_op(semid, SEM_MUTEX_SHM);
        }
        usleep(10000);
    }
    return 0;
}