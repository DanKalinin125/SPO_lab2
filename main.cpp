#include <iostream>
#include <sstream> 
#include "producer_consumer.h"
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {

  // Определяем наличие флага -debug
  bool debug_flag = false;
  for (int i = 1; i < argc; i++){
    if (string(argv[i]) == "-debug") debug_flag = true;
  }

  // Считываем числа
  string input_string;
  vector<int> numbers;
  
  getline(cin, input_string);
  stringstream raw_string_stream(input_string);

  string token;
  while (getline(raw_string_stream, token, ' ')){
    numbers.push_back(stoi(token));
  }

  // Считаем количество чисел (N)
  size_t n = numbers.size();

  // Выводим полученные данные
  // cout << "Debug: " << debug_flag << endl;
  // cout << "N: " << n << endl;

  // cout << "Numbers: ";
  // for (size_t i = 0; i < numbers.size(); i++){
  //   cout << numbers[i] << " ";
  // }
  // cout << endl;

  // Запускаем основную часть программы
  cout << run_threads(n, numbers, debug_flag) << endl;
  return 0;
}
