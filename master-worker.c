// master-worker.c
// Program to simulate a classic producer consumer problem using condition variables and locks
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

// item_to_produce : counter for the current number we are producing and putting in buffer
// curr_buf_size : counter for num items in the buffer
int item_to_produce, curr_buf_size;
// total_items : how many numbers we want to "produce"
// max_buf_size : the max ammount of "items" we can have in our buffer
// num_workers : number of consumers
// num_masters : number of producers
int total_items, max_buf_size, num_workers, num_masters;
// lock
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// condition variables
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
// buffer
int *buffer;

void print_produced(int num, int master) {

  printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {

  printf("Consumed %d by worker %d\n", num, worker);
  
}

// Producer

// modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{// produce items and place in buffer
  int thread_id = *((int *)data);
  
  // will go until 
  while(1){
    pthread_mutex_lock(&mutex);

    // wait if the buffer is currently full
    while (curr_buf_size >= max_buf_size){
      pthread_cond_wait(&not_full, &mutex); // wait for when the buffer is "not_full", then proceed with producer activities
    }
    // if we have produced all of the items we wanted to (item_to_produce is out of bounds)
    if(item_to_produce >= total_items) {
      pthread_mutex_unlock(&mutex);
      break; // we end the producer's process
    }
      
      // Produce item and put it in the buffer
      buffer[curr_buf_size++] = item_to_produce;
      print_produced(item_to_produce, thread_id);
      // make us move on to the next item
      item_to_produce++;

      
      pthread_cond_signal(&not_empty);
      pthread_mutex_unlock(&mutex);
      }
  return 0;
}




// Consumer
// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void *generate_work_loop(void *data){
// consume items out of the buffer
  int thread_id = *((int *)data);
  

  while(1){
    pthread_mutex_lock(&mutex);

    // be sure the 
    while (curr_buf_size == 0 && item_to_produce < total_items){
      pthread_cond_wait(&not_empty, &mutex);
    }

    // If buffer if empty and producers are done
    if (curr_buf_size == 0 && item_to_produce >= total_items){
      pthread_mutex_unlock(&mutex);
      break;
    }

    curr_buf_size--;
    print_consumed(buffer[curr_buf_size], thread_id);


    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);
    }
  return 0;
}

int main(int argc, char *argv[]){

  // producers
  int *master_thread_id;
  pthread_t *master_thread;
  // consumers
  int *worker_thread_id;
  pthread_t *worker_thread;
  // initialize counters
  item_to_produce = 0;
  curr_buf_size = 0;
  // 
  int i;

  if (argc < 5) {
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else {
    num_masters = atoi(argv[4]);
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  } 

   buffer = (int *)malloc (sizeof(int) * max_buf_size);

  //create master producer threads
   master_thread_id = (int *)malloc(sizeof(int) * num_masters);
   master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  
  for (i = 0; i < num_masters; i++)
    master_thread_id[i] = i;

  for (i = 0; i < num_masters; i++)
    pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);
  
  //create worker consumer threads

  worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);

  for (i = 0; i < num_workers; i++)
    worker_thread_id[i] = i;

  for (i = 0; i < num_workers; i++)
    pthread_create(&worker_thread[i], NULL, generate_work_loop, (void *)&worker_thread_id[i]);


  //wait for all threads to complete
  for (i = 0; i < num_masters; i++)
    {
      pthread_join(master_thread[i], NULL);
      printf("master %d joined\n", i);
    }
  for (i = 0; i < num_workers; i++)
  {
    pthread_join(worker_thread[i], NULL);
    printf("worker %d joined\n", i);
  }
  
  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);
  //
  free(worker_thread_id);
  free(worker_thread);
  
  return 0;
}
