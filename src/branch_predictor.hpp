#pragma once

#include <cstdint>
#include <cstdio>

class BranchPredictor {
private:
  static const int SIZE = 1024;
  static const int SIZE_MASK = SIZE - 1;
  bool visited[SIZE], result[SIZE];
  int total, correct;

public:
  BranchPredictor() {
    total = correct = 0;
    for (int i = 0; i < SIZE; i++) {
      visited[i] = result[i] = false;
    }
  }
  bool predict(uint32_t pc) {
    int idx = (pc >> 2) & SIZE_MASK;
    total++;
    if (visited[idx] == false) {
      return false;
    }
    return result[idx];
  }
  void update(uint32_t pc, bool feedback) {
    int idx = (pc >> 2) & SIZE_MASK;
    bool predict = visited[idx] ? result[idx] : false;
    correct += (predict == feedback);
    visited[idx] = true;
    result[idx] = feedback;
  }
  void debug() {
    if (total == 0) {
      puts("Effective prediction has not been made yet");
      return ;
    }
    printf("total predict: %d\n", total);
    printf("correct predict: %d\n", correct);
    printf("prediction accuracy: %.4lf\n",1.0 * correct / total);
  }
};