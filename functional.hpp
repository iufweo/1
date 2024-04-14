#pragma once
#include <list>
#include <memory>

#include "stmt_fwd.hpp"
#include "token.hpp"

struct Functional {
  const std::list<Token> params;
  const std::unique_ptr<const StmtList> listp;

  Functional() = delete;
  Functional(std::list<Token> params, std::unique_ptr<const StmtList>&& listp);
  ~Functional();
};
