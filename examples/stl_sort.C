#ifdef HAVE_CONFIG_H
#  include "programming_paradigms_config.h"
#endif

#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

//constexpr long long size = 500000000;
constexpr long long size = 50000000;

const double pi = std::acos(-1);

template <typename Func>
void getExecutionTime(const std::string& title, Func func){                   // (4)

  const auto sta = std::chrono::steady_clock::now();
  func();                                                                     // (5)
  const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - sta;
  std::cout << title << ": " << dur.count() << " sec. " << std::endl;

}


template <typename T>
void print_front(const std::vector<T> &v)
{
  std::cout << "v.size()=" << v.size() << " : ";
  for (auto i=0; i!=11 && i<v.size(); ++i)
    std::cout << v[i] << " ";
  std::cout << std::endl;
}



int main(){


  typedef unsigned int size_type;
  std::vector<size_type> randValues, workVec;
  randValues.reserve(size); /**/ workVec.reserve(size);

  std::mt19937 engine;
  std::uniform_int_distribution<size_type> uniformDist(0, std::numeric_limits<size_type>::max());
  for (auto i = 0 ; i < size ; ++i) randValues.push_back(uniformDist(engine));

  getExecutionTime("std::copy() / std::sort() / std::unique()", [ randValues, workVec]() mutable {
      workVec.resize(randValues.size());
      std::copy(randValues.begin(), randValues.end(), workVec.begin());
      print_front(workVec);
      std::sort(workVec.begin(), workVec.end());
      print_front(workVec);
      auto new_end = std::unique(workVec.begin(), workVec.end());
      workVec.erase(new_end, workVec.end());
      print_front(workVec);
    }); std::cout << '\n';

  std::cout << '\n';
}
