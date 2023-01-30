#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

// https://www.modernescpp.com/index.php/performance-of-the-parallel-stl-algorithmn
// parallelSTLPerformance.cpp
#include <execution>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <vector>

constexpr long long size = 500000000;

const double pi = std::acos(-1);

template <typename Func>
void getExecutionTime(const std::string& title, Func func){                   // (4)

  const auto sta = std::chrono::steady_clock::now();
  func();                                                                     // (5)
  const std::chrono::duration<double> dur = std::chrono::steady_clock::now() - sta;
  std::cout << title << ": " << dur.count() << " sec. " << std::endl;

}


void print_front(const std::vector<double> &v)
{
  for (auto i=0; i!=4 && i<v.size(); ++i)
    std::cout << v[i] << " ";
  std::cout << std::endl;
}



int main(){

  std::cout << '\n';

  std::vector<double> randValues;
  randValues.reserve(size);

  std::mt19937 engine;
  std::uniform_real_distribution<> uniformDist(0, pi / 2);
  for (long long i = 0 ; i < size ; ++i) randValues.push_back(uniformDist(engine));

  std::vector<double> workVec(randValues);

  //print_front(workVec);

  getExecutionTime("std::tan() / std::execution::seq", [workVec]() mutable {   // (6)
    std::transform(std::execution::seq, workVec.begin(), workVec.end(),        // (1)
		   workVec.begin(),
                   [](double arg){ return std::tan(arg); }
                  );
    });

  //print_front(workVec);

  getExecutionTime("std::tan() / std::execution::par", [workVec]() mutable {   // (7)
    std::transform(std::execution::par, workVec.begin(), workVec.end(),        // (2)
		   workVec.begin(),
                   [](double arg){ return std::tan(arg); }
                  );
  });

  //print_front(workVec);

  getExecutionTime("std::tan() / std::execution::par_unseq", [workVec]() mutable { // (8)
    std::transform(std::execution::par_unseq, workVec.begin(), workVec.end(),      // (3)
		   workVec.begin(),
                   [](double arg){ return std::tan(arg); }
                  );
  });

  //print_front(workVec);


  std::cout << '\n';

}
