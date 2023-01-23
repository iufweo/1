#pragma once
#include <list>
#include <memory>

#include "token.hpp"

struct StmtList;

struct Functional {
  const std::list<Token> params;
  const std::unique_ptr<const StmtList> listp;

  Functional() = delete;
  Functional(std::list<Token> params, std::unique_ptr<const StmtList>&& listp);
  ~Functional();
};
