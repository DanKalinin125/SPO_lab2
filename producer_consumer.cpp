#include <pthread.h>
#include <iostream>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

#define ERROR_CREATE_THREAD -11
#define ERROR_JOIN_THREAD -12

using namespace std;

// Разделяемая переменная
struct shared_variable {
  // Значение разделяемой переменной
  int value;
  // Флаг наличия значения в разделяемой переменной
  bool flag_presence_value;
}

int get_tid() {
  // 1 to 3+N thread ID
  static atomic<int> unique_tid{0};
  thread_local static int *tid;
  if (tid == 0) {
    unique_tid++;
    tid = new int(unique_tid);
  }
  return *tid;
}


// Данные, передаваемые в producer
struct producer_data{
  int* shared_variable;
  int* numbers;
  size_t numbers_size;
}

// Поток producer
void* producer_routine(void* arg) {
  (void)arg;
  printf("Producer (tid) = %d\n", get_tid());
  // read data, loop through each value and update the value, notify consumer,
  // wait for consumer to process
  return nullptr;
}


// Данные передаваемые в consumer
struct consumer_data{
  int* shared_variable;
  bool debug_flag;
  int max_sleep_time;
}

// Поток consumer
void* consumer_routine(void* arg) {
  (void)arg;
  printf("Consumer (tid) = %d\n", get_tid());
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
  return nullptr;
}


void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  // interrupt random consumer while producer is running
  return nullptr;
}


// Запуск потоков
int run_threads(int n, int max_sleep_time, vector<int> numbers, bool debug_flag) {
  // Пока для тестов выводим полученные переменные
  cout << "Debug: " << debug_flag << endl;
  cout << "N: " << n << endl;
  cout << "Max_sleep_time: " << max_sleep_time << endl;
  cout << "Numbers: ";
  for (size_t i = 0; i < numbers.size(); i++){
    cout << numbers[i] << " ";
  }
  cout << endl;

  // Инициализируем разделяемую переменную
  // и заполняем её значениями по умолчанию
  struct shared_variable shared_variable;
  shared_variable.value = 0;
  shared_variable.flag_presence_value = false;

  // Запускаем поток producer
  struct producer_data producer_data;
  producer_data.shared_variable = &shared_variable;
  producer_data.numbers = &numbers[0];
  producer_data.numbers_size = numbers.size();
  
  pthread_t producer;
  int producer_create_status = pthread_create(&producer, NULL, &producer_routine, producer_data);
  if (producer_create_status != 0) {
    printf("Can't create thread producer, status = %d\n", producer_create_status);
    exit(ERROR_CREATE_THREAD);
  }

  // Запускаем N потоков consumer
  struct consumer_data consumer_data;
  consumer_data.shared_variable = &shared_variable;
  consumer_data.max_sleep_time = max_sleep_time;
  consumer_data.debug_flag = debug_flag;
  
  pthread_t consumer_threads[n];
  for (int i = 0; i < n; i++){
    int consumer_create_status = pthread_create(&consumer_threads[i], NULL, &consumer_routine, consumer_data);
    if (consumer_create_status != 0) {
      printf("Can't create thread consumer, status = %d\n", consumer_create_status);
      exit(ERROR_CREATE_THREAD);
    }
  }

  // Ожидаем завершения потока producer
  int producer_join_status = pthread_join(producer, NULL);
  if (producer_join_status != 0) {
    printf("Can't join thread producer, status = %d\n", producer_join_status);
    exit(ERROR_JOIN_THREAD);
  }

  // Ожидаем завершения всех потоков consumer
  for (int i = 0; i < n; i++){
    int consumer_join_status = pthread_join(consumer_threads[i], NULL);
    if (consumer_join_status != 0) {
      printf("Can't join thread consumer, status = %d\n", consumer_join_status);
      exit(ERROR_JOIN_THREAD);
    }
  }

  cout << "Все потоки завершены" << endl;

  return 0;
}
