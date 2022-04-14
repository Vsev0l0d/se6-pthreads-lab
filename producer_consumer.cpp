#include <pthread.h>
#include <unistd.h>
#include <atomic>
#include <iostream>

typedef struct {
  const int sleep_millisecond_limit;
  const bool is_debug;
  const int* number;
} consumerData;

typedef struct {
  const pthread_t* consumers;
  const int length;
} interruptorData;

pthread_cond_t cond_produced;
pthread_cond_t cond_processed;
pthread_mutex_t mutex_produced;
pthread_mutex_t mutex_processed;
bool finish = false;

int get_tid() {
  thread_local static int tid = 0;
  static std::atomic_int count = 0;
  if (tid == 0) tid = ++count;
  return tid;
}

void* producer_routine(void* arg) {
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  int* number = (int*)arg;

  while (scanf("%d", number) == 1) {
    pthread_mutex_lock(&mutex_processed);
    printf("%d\n", *number);
    pthread_cond_signal(&cond_produced);
    pthread_cond_wait(&cond_processed, &mutex_processed);
    pthread_mutex_unlock(&mutex_processed);
  }

  *number = 0;
  finish = true;
  pthread_cond_broadcast(&cond_produced);
  return nullptr;
}

void* consumer_routine(void* arg) {
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  consumerData* data = (consumerData*)arg;
  int* psum = (int*)malloc(sizeof(int));
  *psum = 0;

  while (!finish) {
    pthread_mutex_lock(&mutex_produced);
    pthread_cond_wait(&cond_produced, &mutex_produced);
      *psum += *(data->number);
    pthread_cond_signal(&cond_processed);
    pthread_mutex_unlock(&mutex_produced);

    if (finish) break;
    if (data->is_debug) printf("(%d, %d)\n", get_tid(), *psum);
    srand(time(0));
    usleep(1000 * (rand() % (data->sleep_millisecond_limit + 1)));
  }

  return psum;
}

void* consumer_interruptor_routine(void* arg) {
  // interrupt random consumer while producer is running
  interruptorData* data = (interruptorData*)arg;
  while (!finish) {
    srand(time(0));
    pthread_cancel(data->consumers[rand() % data->length]);
  }
  return nullptr;
}

// the declaration of run threads can be changed as you like
int run_threads(int N, int sleep_millisecond_limit, bool is_debug = false) {
  // start N threads and wait until they're done
  // return aggregated sum of values
  int aggregated_sum = 0, number = 0;
  pthread_mutex_init(&mutex_produced, NULL);
  pthread_mutex_init(&mutex_processed, NULL);
  pthread_cond_init(&cond_produced, NULL);
  pthread_cond_init(&cond_processed, NULL);

  pthread_t* consumers = (pthread_t*)malloc(N * sizeof(pthread_t));
  consumerData consumersData = {sleep_millisecond_limit, is_debug, &number};
  for (int i = 0; i < N; i++) {
    pthread_create(&(consumers[i]), NULL, consumer_routine, &consumersData);
  }

  pthread_t producer;
  pthread_create(&producer, NULL, producer_routine, &number);

  pthread_t interruptor;
  interruptorData interruptorData = {consumers, N};
  pthread_create(&interruptor, NULL, consumer_interruptor_routine,
                 &interruptorData);

  pthread_join(producer, NULL);
  pthread_join(interruptor, NULL);
  for (int i = 0; i < N; i++) {
    int* psum;
    pthread_join(consumers[i], (void**)&psum);
    aggregated_sum += *psum;
    free(psum);
  }

  free(consumers);
  pthread_cond_destroy(&cond_processed);
  pthread_cond_destroy(&cond_produced);
  pthread_mutex_destroy(&mutex_processed);
  pthread_mutex_destroy(&mutex_produced);
  return aggregated_sum;
}
