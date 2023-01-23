#pragma once
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "expr_visitor.hpp"
#include "ltype.hpp"
#include "runent.hpp"
#include "stmt_visitor.hpp"
#include "token.hpp"

class Lfunc;
class Lclass;
class Linstance;

class Interp : public ExprVisitor, public StmtVisitor {
 private:
  Ltype visit(const ExprBinary* expr) final;
  Ltype visit(const ExprGrouping* expr) final;
  Ltype visit(const ExprLiteral* expr) final;
  Ltype visit(const ExprUnary* expr) final;
  Ltype visit(const ExprTern* expr) final;
  Ltype visit(const ExprVar* expr) final;
  Ltype visit(const ExprAssign* expr) final;
  Ltype visit(const ExprCall* expr) final;
  Ltype visit(const ExprGet* expr) final;
  Ltype visit(const ExprSet* expr) final;
  Ltype visit(const ExprThis* expr) final;
  Ltype visit(const ExprSuper* expr) final;
  Ltype visit(const ExprComma* expr) final;
  Ltype visit(const ExprLogical* expr) final;
  Ltype visit(std::shared_ptr<const ExprFun> expr) final;

  void visit(const StmtExpr& stmt) final;
  void visit(const StmtPrint& stmt) final;
  void visit(const StmtVar& stmt) final;
  void visit(const StmtLoop& stmt) final;
  void visit(const StmtIf& stmt) final;
  void visit(const StmtList& stmt) final;
  void visit(const StmtReturn& stmt) final;
  void visit(const StmtLoopFlow& stmt) final;
  void visit(std::shared_ptr<const StmtFun> stmtp) final;
  void visit(const StmtClass& stmt) final;

  Ltype eval(std::shared_ptr<const Expr> expr);
  bool equality(const Ltype& left, const Ltype& right);
  bool floatcmp(double a, double b);
  bool isTruthful(const Ltype& obj) const;
  void checkNumberOperands(Token oper, const Ltype& obj) const;
  void checkNumberOperands(Token oper,
                           const Ltype& left,
                           const Ltype& right) const;

  class RuntimeError : public std::runtime_error {
   public:
    Token token;

    RuntimeError() = delete;
    RuntimeError(Token token, std::string what);
  };
  static bool hadError;
  static bool hadRuntimeError;

  void handleRuntimeError(RuntimeError& ex);

 public:
  class Env : public RunEnt, public ClassUncopyable {
   private:
    friend class Interp;
    Env* enclosing;
    std::unordered_map<std::string, std::optional<Ltype>> m;

   public:
    Env();
    Env(Env* enclosing);

    void def(const Token& token, std::optional<Ltype> obj);

    Ltype get(const Token& token) const;
    Ltype assertGet(const Token& token) const;
    Ltype getAt(const Token& token, std::size_t distance);
    Env* ancestor(std::size_t distance);

    void assign(const Token& token, Ltype obj);
    void assignAt(const Token& token, Ltype obj, std::size_t distance);
  };

 private:
  Env global;
  Env* envp;
  // is altered only by Resolver and never when code is run
  std::unordered_map<const Expr*, std::size_t> locals;

 public:
  void resolve(const Expr* expr, std::size_t distance);

 private:
  Ltype lookupVariable(const Token& token, const Expr* expr);

  using Allocated = std::variant<Env*, Lfunc*, Lclass*, Linstance*>;
  std::list<Allocated> traced;
  std::list<std::variant<Env*, Ltype>> stack;
  std::size_t heapSize;

  template <typename T, typename... Ts>
  T* doAlloc(Ts&&... args) {
    T* ret;

    const std::size_t LIM = 2500;

    if (heapSize + sizeof(T) >= LIM) {
      for (auto& x : stack) {
        if (std::holds_alternative<Env*>(x))
          mark(std::get<Env*>(x));
        else
          markLtype(std::get<Ltype>(x));
      }

      mark(envp);
      reclaim();
      unmark();
    }
    if (heapSize + sizeof(T) >= LIM)
      throw std::runtime_error("out of memory");

    ret = new T(std::forward<Ts>(args)...);
    traced.push_back(ret);
    heapSize += sizeof(T);

    return ret;
  }

 public:
  class ReclaimerCtx {
   public:
    std::size_t count;
    Interp& interp;

    ReclaimerCtx(Interp& interp) : count(0), interp(interp) {}

    Ltype add(Ltype l) {
      interp.stack.push_back(l);
      count++;
      return l;
    }

    ~ReclaimerCtx() {
      std::size_t i;

      for (i = 0; i < count; i++)
        interp.stack.pop_back();
    }
  };

  template <typename T, typename... Ts>
  T* alloc(ReclaimerCtx& ctx, Ts&&... args) {
    T* ret;

    ret = doAlloc<T>(args...);
    stack.push_back(ret);
    ctx.count++;
    return ret;
  }

 private:
  // avoid overloading on Ltype
  void markLtype(Ltype& l);
  void mark(Lclass* lclass);
  void mark(Linstance* obj);
  void mark(Lfunc* func);
  void mark(Env* env);
  void reclaim();
  void unmark();

  void interpret(const std::list<std::shared_ptr<const Stmt>>& list);
  void run(std::string inputStr);

 public:
  // empty
  class ControlFlow : public std::exception {};
  class Break : public ControlFlow {};
  class Continue : public ControlFlow {};
  class Return : public ControlFlow {
   public:
    Ltype retVal;

    Return() = delete;
    Return(Ltype retVal) : retVal(retVal) {}
  };

  void execute(const Stmt& stmt);
  void execute(const StmtList& stmt, Env* setEnv);

  Interp();
  ~Interp();

  void runPrompt();
  void runFile(std::string path);
  static void testScanner(std::string inputStr);

  static void error(std::size_t lineNum, std::string msg);
  static void error(Token token, std::string msg);
  static void report(Token token, std::string msg);
  static void report(std::size_t lineNum,
                     std::string location,
                     std::string msg);
};
