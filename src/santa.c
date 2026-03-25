#include "santa.h"

void* santa_routine(void* arg) {
  state_shared_t *sim_state = (state_shared_t*)arg;
  while(1) {
    pthread_mutex_lock(&sim_state->mutex_santa);
    while(
      !sim_state->is_terminate 
      && sim_state->count_reindeer < 9 
      //&& sim_state->count_gnome < 3
      && !sim_state->is_gnome_ready
    ){
      pthread_cond_wait(&sim_state->cv_santa, &sim_state->mutex_santa);
    }

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);
      return NULL;
    }

    PRINT_MSG("Santa has been awaken!\n");
    
    if(sim_state->count_reindeer >= 9) {
      sim_state->total_deliveries_made += 1;
      sim_state->count_reindeer = 0;
      PRINT_MSG("Reindeer have been served. This is delivery number %d, and now %d left to go!\n", sim_state->total_deliveries_made, MAX_DELIVERIES - sim_state->total_deliveries_made);
      pthread_mutex_unlock(&sim_state->mutex_santa);
      SIM_SLEEP(3);
      pthread_mutex_lock(&sim_state->mutex_santa);

      if(sim_state->total_deliveries_made >= MAX_DELIVERIES) {
        pthread_cond_signal(&sim_state->cv_main_wait);
      }

      pthread_cond_broadcast(&sim_state->cv_reindeer);
    }else if(
      sim_state->is_gnome_ready
      //sim_state->count_gnome >= 3
    ) {
      sim_state->total_consults_made += 1;
      PRINT_MSG("Consultation number %d for gnomes is held! There's only %d remain till X-mas!\n", sim_state->total_consults_made, MAX_CONSULTS - sim_state->total_consults_made);
      
      pthread_mutex_unlock(&sim_state->mutex_santa);
      SIM_SLEEP(5);
      pthread_mutex_lock(&sim_state->mutex_santa);

      if(sim_state->total_consults_made >= MAX_CONSULTS) {
        pthread_cond_signal(&sim_state->cv_main_wait);
      }

      sim_state->is_gnome_ready = false;

      pthread_cond_broadcast(&sim_state->cv_gnome);
    }

    PRINT_MSG("Santa goes back to sleep!\n");
    pthread_mutex_unlock(&sim_state->mutex_santa);
  }
}

void* reindeer_routine(void* arg) {
  state_shared_t *sim_state = (state_shared_t*)arg;

  while(1) {
    SIM_SLEEP(10);

    pthread_mutex_lock(&sim_state->mutex_santa);

    PRINT_MSG("Reindeer has been awaken!\n");
    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);

      return NULL;
    }

    sim_state->count_reindeer += 1;

    PRINT_MSG("Now there are %d reindeer gathered!\n", sim_state->count_reindeer);

    if(sim_state->count_reindeer >= 9) {
      PRINT_MSG("9 Reindeer have gathered and heading to wake up Santa!\n");
      pthread_cond_signal(&sim_state->cv_santa);
    }

    while(sim_state->count_reindeer != 0 && !sim_state->is_terminate) {
      pthread_cond_wait(&sim_state->cv_reindeer, &sim_state->mutex_santa);
    }

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);

      return NULL;
    }

    pthread_mutex_unlock(&sim_state->mutex_santa);
    PRINT_MSG("Reindeer are going back to their sleep state!\n");
  }
}

void* gnome_routine(void* arg) {
  state_shared_t *sim_state = (state_shared_t*)arg;

  while(1) {
    SIM_SLEEP(10);
    PRINT_MSG("Gnomes have been awaken!\n");

    pthread_mutex_lock(&sim_state->mutex_santa);

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);

      return NULL;
    }

    while(
      (
        sim_state->count_gnome >= 3
        || 
        sim_state->is_consulting
      ) 
      && !sim_state->is_terminate
    ) {
      pthread_cond_wait(&sim_state->cv_gnome_queue, &sim_state->mutex_santa);
    }

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);

      return NULL;
    }

    sim_state->count_gnome += 1;

    PRINT_MSG("New gnome is heading towards Santa! Now we're %d gnomes up!\n", sim_state->count_gnome);

    if(sim_state->count_gnome == 3) {
      sim_state->is_consulting = true;
      sim_state->is_gnome_ready = true;
      pthread_cond_signal(&sim_state->cv_santa);
    }

    int current_consult = sim_state->total_consults_made;

    while(sim_state->total_consults_made == current_consult && !sim_state->is_terminate) {
      pthread_cond_wait(&sim_state->cv_gnome, &sim_state->mutex_santa);
    }

    if(sim_state->is_terminate) {
      pthread_mutex_unlock(&sim_state->mutex_santa);

      return NULL;
    }
   
    PRINT_MSG("Consultation with Santa has ended, now it's time to release our fellow gnome!\n");
    sim_state->count_gnome -= 1;

    PRINT_MSG("Now we're %d gnomes gathered!\n", sim_state->count_gnome);

    if(sim_state->count_gnome == 0) {
      sim_state->is_consulting = false;
      pthread_cond_broadcast(&sim_state->cv_gnome_queue);
    }

    pthread_mutex_unlock(&sim_state->mutex_santa);
    PRINT_MSG("Gnomes are going back to sleep!\n");
  }
}

void project_zso(void) {
  state_shared_t sim_state;

  PRINT_MSG("Welcome to Santa's North Pole House!\n");

  sim_state.count_reindeer = 0;
  sim_state.count_gnome = 0;
  sim_state.total_deliveries_made = 0;
  sim_state.total_consults_made = 0;

  sim_state.is_santa_sleeping = true;
  sim_state.is_consulting = false;
  sim_state.is_gnome_ready = false;
  sim_state.is_terminate = false;

  pthread_mutex_init(&sim_state.mutex_santa, NULL);

  PRINT_MSG("Santa mutex has been successfully initialized!\n");

  pthread_cond_init(&sim_state.cv_santa, NULL);
  pthread_cond_init(&sim_state.cv_reindeer, NULL);
  pthread_cond_init(&sim_state.cv_gnome, NULL);
  pthread_cond_init(&sim_state.cv_gnome_queue, NULL);
  pthread_cond_init(&sim_state.cv_main_wait, NULL);

  PRINT_MSG("All thread conditions initialized successfully!\n");

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

  PRINT_MSG("Simulation is being terminated!\n");

  sim_state.is_terminate = true;

  pthread_cond_broadcast(&sim_state.cv_santa);
  pthread_cond_broadcast(&sim_state.cv_reindeer);
  pthread_cond_broadcast(&sim_state.cv_gnome);
  pthread_cond_broadcast(&sim_state.cv_gnome_queue);

  pthread_mutex_unlock(&sim_state.mutex_santa);

  PRINT_MSG("Now starting to join all threads!\n");

  pthread_join(thread_santa, NULL);

  for(int i = 0; i < TOTAL_REINDEER; i++) {
    pthread_join(thread_reindeer[i], NULL);
  }

  for(int i = 0; i < TOTAL_GNOMES; i++) {
    pthread_join(thread_gnome[i], NULL);
  }

  //DON'T FORGET TO DESTROY ALL OBJECTS
  PRINT_MSG("Process of destruction has been initiated!\n");
  pthread_cond_destroy(&sim_state.cv_santa);
  pthread_cond_destroy(&sim_state.cv_reindeer);
  pthread_cond_destroy(&sim_state.cv_gnome);
  pthread_cond_destroy(&sim_state.cv_gnome_queue);
  pthread_cond_destroy(&sim_state.cv_main_wait);

  pthread_mutex_destroy(&sim_state.mutex_santa);

  PRINT_MSG("Thank you! See you next X-mas!\n");
}
