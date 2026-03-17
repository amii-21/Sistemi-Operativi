#include "common.h"

/// Funzione per caricare la configurazione da un file
void load_config(const char *filename, Config *cfg) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("Errore apertura config"); exit(1); }
    
    char line[256], key[128];
    int val;
    
    memset(cfg, 0, sizeof(Config));

    while (fgets(line, sizeof(line), f)) {
        if(line[0] == '#' || line[0] == '\n') continue;
        if(sscanf(line, "%s %d", key, &val) == 2) {
            if (strcmp(key, "SIM_DURATION") == 0) cfg->SIM_DURATION = val;
            else if (strcmp(key, "N_NANO_SECS") == 0) cfg->N_NANO_SECS = val;
            else if (strcmp(key, "NOF_WORKERS") == 0) cfg->NOF_WORKERS = val;
            else if (strcmp(key, "NOF_USERS") == 0) cfg->NOF_USERS = val;
            else if (strcmp(key, "NOF_TABLE_SEATS") == 0) cfg->NOF_TABLE_SEATS = val;
            else if (strcmp(key, "MAX_PORZIONI_PRIMI") == 0) cfg->MAX_PORZIONI_PRIMI = val;
            else if (strcmp(key, "MAX_PORZIONI_SECONDI") == 0) cfg->MAX_PORZIONI_SECONDI = val;
            else if (strcmp(key, "AVG_REFILL_PRIMI") == 0) cfg->AVG_REFILL_PRIMI = val;
            else if (strcmp(key, "AVG_REFILL_SECONDI") == 0) cfg->AVG_REFILL_SECONDI = val;
            else if (strcmp(key, "OVERLOAD_THRESHOLD") == 0) cfg->OVERLOAD_THRESHOLD = val;
            else if (strcmp(key, "AVG_SRVC_PRIMI") == 0) cfg->AVG_SRVC_PRIMI = val;
            else if (strcmp(key, "AVG_SRVC_SECONDI") == 0) cfg->AVG_SRVC_SECONDI = val;
            else if (strcmp(key, "AVG_SRVC_COFFEE") == 0) cfg->AVG_SRVC_COFFEE = val;
            else if (strcmp(key, "AVG_SRVC_CASSA") == 0) cfg->AVG_SRVC_CASSA = val;
            else if (strcmp(key, "PRICE_PRIMI") == 0) cfg->PRICE_PRIMI = val;
            else if (strcmp(key, "PRICE_SECONDI") == 0) cfg->PRICE_SECONDI = val;
            else if (strcmp(key, "PRICE_COFFEE") == 0) cfg->PRICE_COFFEE = val;
        }
    }
    fclose(f);
}

void sem_wait_op(int semid, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}

void sem_signal_op(int semid, int sem_num) {
    struct sembuf sb;
    sb.sem_num = sem_num;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    semop(semid, &sb, 1);
}