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
  
  cin >> input_string;
  stringstream raw_string_stream(input_string);

  string token;
  while (getline(raw_string_stream, token, ' ')){
    numbers.push_back(stoi(token));
  }

  for (size_t i = 0; i < numbers.size(); i++){
    cout << numbers[i] << endl;
  }

  cout << "Debug_flag " << debug_flag << endl;

  cout << run_threads() << endl;
  return 0;
}
