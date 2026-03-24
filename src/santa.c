#include "santa.h"

void* santa_routine(void* arg) {
  state_shared_t *sim_state = (state_shared_t*)arg;
  while(1) {
    pthread_mutex_lock(&sim_state->mutex_santa);
    while(
      !sim_state->is_terminate 
      && sim_state->count_reindeer < 9 
      && sim_state->count_gnome < 3
    ){
      pthread_cond_wait(&sim_state->cv_santa, &sim_state->mutex_santa);
    }

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);
      return NULL;
    }

    if(sim_state->count_reindeer >= 9) {
      sim_state->total_deliveries_made += 1;
      PRINT_MSG("Reindeer have been served. This is delivery number %d, and now %d left to go!", sim_state->total_deliveries_made, MAX_DELIVERIES - sim_state->total_deliveries_made);
      SIM_SLEEP(3);

      if(sim_state->total_deliveries_made >= MAX_DELIVERIES) {
        pthread_cond_signal(&sim_state->cv_main_wait);
      }

      pthread_cond_broadcast(&sim_state->cv_reindeer);
      sim_state->count_reindeer = 0;
    }else if(sim_state->count_gnome >= 3) {
      sim_state->total_consults_made += 1;
      PRINT_MSG("Consultation number %d for gnomes is held! There's only %d remain till X-mas!", sim_state->total_consults_made, MAX_CONSULTS - sim_state->total_consults_made);
      SIM_SLEEP(5);

      if(sim_state->total_consults_made >= MAX_CONSULTS) {
        pthread_cond_signal(&sim_state->cv_main_wait);
      }

      pthread_cond_broadcast(&sim_state->cv_gnome);
    }
    pthread_mutex_unlock(&sim_state->mutex_santa);
  }
}

void* reindeer_routine(void* arg) {
  //DEFINE LATER
}

void* gnome_routine(void* arg) {
  //DEFINE LATER
}

void project_zso(void) {
  state_shared_t sim_state;

  sim_state.count_reindeer = 0;
  sim_state.count_gnome = 0;
  sim_state.total_deliveries_made = 0;
  sim_state.total_consults_made = 0;

  sim_state.is_santa_sleeping = true;
  sim_state.is_terminate = false;

  pthread_mutex_init(&sim_state.mutex_santa, NULL);

  pthread_cond_init(&sim_state.cv_santa, NULL);
  pthread_cond_init(&sim_state.cv_reindeer, NULL);
  pthread_cond_init(&sim_state.cv_gnome, NULL);
  pthread_cond_init(&sim_state.cv_gnome_queue, NULL);
  pthread_cond_init(&sim_state.cv_main_wait, NULL);

  pthread_t thread_santa;
  pthread_t thread_reindeer[TOTAL_REINDEER];
  pthread_t thread_gnome[TOTAL_GNOMES];

  pthread_create(&thread_santa, NULL, santa_routine, &sim_state);

  for(int i = 0; i < TOTAL_REINDEER; i++) {
    pthread_create(&thread_reindeer[i], NULL, reindeer_routine, &sim_state);
  }

  for(int i = 0; i < TOTAL_GNOMES; i++) {
    pthread_create(&thread_gnome[i], NULL, gnome_routine, &sim_state);
  }

  pthread_mutex_lock(&sim_state.mutex_santa);

  while (sim_state.total_deliveries_made < MAX_DELIVERIES && sim_state.total_consults_made < MAX_CONSULTS) {
    pthread_cond_wait(&sim_state.cv_main_wait, &sim_state.mutex_santa);
  }

  sim_state.is_terminate = true;

  pthread_cond_broadcast(&sim_state.cv_santa);
  pthread_cond_broadcast(&sim_state.cv_reindeer);
  pthread_cond_broadcast(&sim_state.cv_gnome);
  pthread_cond_broadcast(&sim_state.cv_gnome_queue);

  pthread_mutex_unlock(&sim_state.mutex_santa);

  pthread_join(thread_santa, NULL);

  for(int i = 0; i < TOTAL_REINDEER; i++) {
    pthread_join(thread_reindeer[i], NULL);
  }

  for(int i = 0; i < TOTAL_GNOMES; i++) {
    pthread_join(thread_gnome[i], NULL);
  }

  //DON'T FORGET TO DESTROY ALL OBJECTS
  pthread_cond_destroy(&sim_state.cv_santa);
  pthread_cond_destroy(&sim_state.cv_reindeer);
  pthread_cond_destroy(&sim_state.cv_gnome);
  pthread_cond_destroy(&sim_state.cv_gnome_queue);
  pthread_cond_destroy(&sim_state.cv_main_wait);

  pthread_mutex_destroy(&sim_state.mutex_santa);
}
