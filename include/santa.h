#ifndef SANTA_H
#define SANTA_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#ifdef ENABLE_VERBOSE
    #define PRINT_MSG(...) printf(__VA_ARGS__)
    #define SIM_SLEEP(x) sleep(x)
#else
    #define PRINT(MSG...) do {} while(0)
    #define SIM_SLEEP(x) do{} while(0)
#endif

#define TOTAL_REINDEER 9
#define TOTAL_GNOMES 10
#define NEED_TO_WAKE_GNOMES 3

#define MAX_DELIVERIES 5
#define MAX_CONSULTS 10

typedef struct {
    int count_reindeer;
    int count_gnome;

    int total_deliveries_made;
    int total_consults_made;

    bool is_santa_sleeping;
    bool is_terminate;

    pthread_mutex_t mutex_santa;

    pthread_cond_t cv_santa;
    pthread_cond_t cv_reindeer;
    pthread_cond_t cv_gnome;
    pthread_cond_t cv_gnome_queue;
    pthread_cond_t cv_main_wait;
} state_shared_t;

void project_zso(void);

#endif
