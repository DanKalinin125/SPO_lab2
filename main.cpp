#include <iostream>
#include <sstream> 
#include "producer_consumer.h"
#include <vector>

using namespace std;


// На вход подается
// 1. Флаг -debug (в argv)
// 2. Число потоков consumer
// 3. Максимальное время сна для consumer
// 4. Строка чисел через пробел для producer
int main(int argc, char* argv[]) {

  // Определяем наличие флага -debug
  bool debug_flag = false;
  for (int i = 1; i < argc; i++){
    if (string(argv[i]) == "-debug") debug_flag = true;
  }

  // Число потоков consumer (N)
  int n;
  string n_string;
  cout << "Введите число потоков consumer (N): ";
  getline(cin, n_string);
  n = stoi(n_string);

  // Максимальное время сна для consumer
  int max_sleep_time;
  string max_sleep_time_string;
  cout << "Введите максимальное время сна для consumer: ";
  getline(cin, max_sleep_time_string);
  max_sleep_time = stoi(max_sleep_time_string);

  // Список чисел
  vector<int> numbers;
  string input_string;
  cout << "Введите числа через пробел: ";
  getline(cin, input_string);

  stringstream raw_string_stream(input_string);
  string token;
  while (getline(raw_string_stream, token, ' ')){
    numbers.push_back(stoi(token));
  }

  // Выводим полученные данные
  // cout << "Debug: " << debug_flag << endl;
  // cout << "N: " << n << endl;
  // cout << "Max_sleep_time: " << max_sleep_time << endl;
  // cout << "Numbers: ";
  // for (size_t i = 0; i < numbers.size(); i++){
  //   cout << numbers[i] << " ";
  // }
  // cout << endl;

  // Запускаем основную часть программы
  cout << run_threads(n, max_sleep_time, numbers, debug_flag) << endl;
  return 0;
}
