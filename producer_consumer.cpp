#include <pthread.h>
#include <unistd.h>
#include <iostream>

typedef struct {
  int sleep_millisecond_limit;
  bool is_debug;
  const int* number;
} pthrData;

int get_tid() {
  // 1 to 3+N thread ID
  return 0;
}

void* producer_routine(void* arg) {
  (void)arg;
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  return nullptr;
}

void* consumer_routine(void* arg) {
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  pthrData* data = (pthrData*)arg;
  int* psum = (int*)malloc(sizeof(int));
  *psum = 0;

  *psum += *(data->number);
  srand(time(0));
  usleep(1000 * (rand() % (data->sleep_millisecond_limit + 1)));
  if (data->is_debug) printf("(%d, %d)\n", get_tid(), *psum);

  return psum;
}

void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  // interrupt random consumer while producer is running
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads(int N, int sleep_millisecond_limit, bool is_debug = false) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  int aggregated_sum = 0, number = 0;
  pthread_t* threads = (pthread_t*)malloc(N * sizeof(pthread_t));
  pthrData threadData = {sleep_millisecond_limit, is_debug, &number};

  for (int i = 0; i < N; i++) {
    pthread_create(&(threads[i]), NULL, consumer_routine, &threadData);
  }

  for (int i = 0; i < N; i++) {
    int* psum;
    pthread_join(threads[i], (void**)&psum);
    aggregated_sum += *psum;
    free(psum);
  }

  free(threads);
  return aggregated_sum;
}
