#pragma once

#include <cstdint>
#include <cstdio>

class BranchPredictor {
private:
  static const int SIZE = 1024;
  static const int SIZE_MASK = SIZE - 1;
  // bool visited[SIZE], result[SIZE];
  uint8_t count[SIZE];
  int total, correct;

public:
  BranchPredictor() {
    total = correct = 0;
    for (int i = 0; i < SIZE; i++) {
      // visited[i] = result[i] = false;
      count[i] = 2;
    }
  }
  bool predict(uint32_t pc) {
    int idx = (pc >> 2) & SIZE_MASK;
    return (count[idx] >= 2);
    // if (visited[idx] == false) {
    //   return true;
    // }
    // return result[idx];
  }
  void update(uint32_t pc, bool feedback, bool predict) {
    int idx = (pc >> 2) & SIZE_MASK;
    total++;
    correct += (predict == feedback);
    if (feedback && count[idx] < 3) {
      count[idx]++;
    } else if (!feedback && count[idx]) {
      count[idx]--;
    }
    // visited[idx] = true;
    // result[idx] = feedback;
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