#pragma once
#include <list>
#include <optional>
#include <unordered_map>
#include <variant>

#include "functional.hpp"
#include "interp.hpp"
#include "ltype.hpp"
#include "runent.hpp"
#include "stmt.hpp"
#include "uncopyable.hpp"

class Func : public ClassUncopyable {
public:
  Func(std::size_t arity);

  virtual ~Func() = default;

  const std::size_t arity;

  virtual Ltype call(Interp &interp, const std::list<Ltype> &args) = 0;

  //virtual void
  //accept(RunStructVisitor &v) = 0;
};

class Clock : public Func {
private:
  friend class Interp;

  Clock();

public:
  static Clock *get();

  Ltype call(Interp &interp, const std::list<Ltype> &args) final;

  //virtual void
  //accept(RunStructVisitor &v) final;
};

class Lfunc : public Func, public RunEnt {
private:
  friend class Interp;

  const std::shared_ptr<const Functional> funp;
  Interp::Env *enclosing;
  bool isCtor;

public:
  Lfunc(std::shared_ptr<const Functional> funp, Interp::Env *enclosing,
        bool isCtor);
  Lfunc() = delete;

  Ltype call(Interp &interp, const std::list<Ltype> &args) final;

  Lfunc *bind(Interp &interp, Linstance *inst) const;

  //virtual void
  //accept(RunStructVisitor &v) final;
};

class Lclass : public Func, public RunEnt {
private:
  friend class Interp;
  const std::string name;

  const std::unordered_map<std::string, Lfunc *> methods;
  const std::unordered_map<std::string, Lfunc *> staticMethods;
  Lclass *base;
  Interp::Env *superEnv;

  std::optional<Lfunc *>
  get(const std::unordered_map<std::string, Lfunc *> &table,
      std::optional<Lfunc *> (Lclass::*getter)(std::string) const,
      std::string name) const;

public:
  Lclass(std::string name, std::size_t ctorArity,
         std::unordered_map<std::string, Lfunc *> methods,
         std::unordered_map<std::string, Lfunc *> staticMethods, Lclass *base,
         Interp::Env *superEnv);

  Lclass() = delete;

  std::optional<Lfunc *> getMethod(std::string name) const;

  std::optional<Lfunc *> getStaticMethod(std::string name) const;

  Ltype call(Interp &interp, const std::list<Ltype> &args) final;

  //virtual void
  //accept(RunStructVisitor &v) final;
};

class Linstance : public ClassUncopyable, public RunEnt {
private:
  friend class Interp;

  Lclass *lclass;
  std::unordered_map<std::string, Ltype> properties;

public:
  Linstance(Lclass *lclass);
  Linstance() = delete;

  std::optional<Ltype> get(Interp &interp, std::string name);

  void set(std::string name, Ltype obj);

  //virtual void
  //accept(RunStructVisitor &v) final;
};
