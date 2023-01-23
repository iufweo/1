#include <ctime>
#include <memory>
#include <optional>

#include "func.hpp"
#include "interp.hpp"
#include "ltype.hpp"
// constructor for Token so ctors can return 'this'
#include "functional.hpp"
#include "runent.hpp"
#include "token.hpp"

Func::Func(std::size_t arity) : arity(arity) {}

Clock::Clock() : Func(0) {}

Clock* Clock::get() {
  static Clock c;

  return &c;
}

Ltype Clock::call(Interp& interp, const std::list<Ltype>& args) {
  (void)interp;
  (void)args;
  return (double)std::clock() / CLOCKS_PER_SEC;
}

Lfunc::Lfunc(std::shared_ptr<const Functional> funp,
             Interp::Env* enclosing,
             bool isCtor)
    : Func(funp->params.size()),
      funp(funp),
      enclosing(enclosing),
      isCtor(isCtor) {}

Ltype Lfunc::call(Interp& interp, const std::list<Ltype>& args) {
  Interp::Env* newEnv;
  Ltype ret;

  Interp::ReclaimerCtx ctx(interp);

  // implicitly return nil by default, in case of no return statement
  ret = Lnil();

  newEnv = interp.alloc<Interp::Env>(ctx, enclosing);

  auto itArgs = args.cbegin();
  auto itArgsEnd = args.cend();
  auto itParams = funp->params.cbegin();
  for (; itArgs != itArgsEnd; itParams++, itArgs++)
    newEnv->def(*itParams, *itArgs);
  try {
    interp.execute(*funp->listp.get(), newEnv);
  } catch (Interp::Return& exception) {
    ret = exception.retVal;
  }

  if (isCtor)
    ret = enclosing->getAt(Token(Token::Type::THIS, "this", "", 0), 0);
  return ret;
}

Lfunc* Lfunc::bind(Interp& interp, Linstance* inst) const {
  Interp::Env* newEnv;
  Lfunc* ptr;
  Interp::ReclaimerCtx ctx(interp);

  newEnv = interp.alloc<Interp::Env>(ctx, enclosing);
  // bind as ctor if a ctor
  ptr = interp.alloc<Lfunc>(ctx, funp, newEnv, isCtor);
  newEnv->def(Token(Token::Type::THIS, "this", "", 0), inst);

  return ptr;
}

Lclass::Lclass(std::string name,
               std::size_t ctorArity,
               std::unordered_map<std::string, Lfunc*> methods,
               std::unordered_map<std::string, Lfunc*> staticMethods,
               Lclass* base,
               Interp::Env* superEnv)
    : Func(ctorArity),
      name(name),
      methods(methods),
      staticMethods(staticMethods),
      base(base),
      superEnv(superEnv) {}

// when called as class name
Ltype Lclass::call(Interp& interp, const std::list<Ltype>& args) {
  Linstance* ptr;
  Interp::ReclaimerCtx ctx(interp);

  ptr = interp.alloc<Linstance>(ctx, this);

  auto it = methods.find(name);
  if (it != methods.end())
    // bound
    std::get<LfunPtr>(ctx.add(ptr->get(interp, name).value()))
        ->call(interp, args);

  return ptr;
}

std::optional<Lfunc*> Lclass::get(
    const std::unordered_map<std::string, Lfunc*>& table,
    std::optional<Lfunc*> (Lclass::*getter)(std::string) const,
    std::string name) const {
  auto it = table.find(name);
  if (it == table.end()) {
    if (base == nullptr)
      return std::nullopt;
    return (base->*getter)(name);
  }
  return it->second;
}

std::optional<Lfunc*> Lclass::getMethod(std::string name) const {
  return get(methods, &Lclass::getMethod, name);
}

std::optional<Lfunc*> Lclass::getStaticMethod(std::string name) const {
  return get(staticMethods, &Lclass::getStaticMethod, name);
}

Linstance::Linstance(Lclass* lclass) : lclass(lclass), properties{} {}

std::optional<Ltype> Linstance::get(Interp& interp, std::string name) {
  std::optional<Lfunc*> ret;

  auto it = properties.find(name);
  if (it == properties.end()) {
    ret = lclass->getMethod(name);
    if (ret.has_value())
      // then must be a method
      return ret.value()->bind(interp, this);
    return std::nullopt;
  }
  // field
  return it->second;
}

void Linstance::set(std::string name, Ltype obj) {
  // create if doesn't exist or assign other
  properties[name] = obj;
}
