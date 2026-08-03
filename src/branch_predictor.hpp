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
    if (visited[idx] == false) {
      return true;
    }
    return result[idx];
  }
  void update(uint32_t pc, bool feedback, bool predict) {
    int idx = (pc >> 2) & SIZE_MASK;
    total++;
    correct += (predict == feedback);
    visited[idx] = true;
    result[idx] = feedback;
  }
  void debug() {
    if (total == 0) {
      fputs("Effective prediction has not been made yet\n", stderr);
      return ;
    }
    fprintf(stderr, "total predict: %d\n", total);
    fprintf(stderr, "correct predict: %d\n", correct);
    fprintf(stderr, "prediction accuracy: %.4lf\n",1.0 * correct / total);
  }
};