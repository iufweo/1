#include <list>
#include <memory>

#include "stmt.hpp"

#include "functional.hpp"

Functional::Functional(std::list<Token> params,
                       std::unique_ptr<const StmtList>&& listp)
    : params(params), listp(std::move(listp)) {}

Functional::~Functional() = default;
