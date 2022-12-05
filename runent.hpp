#pragma once

class RunEnt {
public:
  bool isReachable;

public:
  RunEnt() : isReachable(0) {}

  void unmarkSelf() { isReachable = 0; }

};
