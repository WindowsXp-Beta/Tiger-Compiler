#include "tiger/env/env.h"
#include "tiger/translate/translate.h"

namespace tr {

void ProgTr::FillBaseTEnv() {
  tenv_->Enter(sym::Symbol::UniqueSymbol("int"), type::IntTy::Instance());
  tenv_->Enter(sym::Symbol::UniqueSymbol("string"), type::StringTy::Instance());
}

void ProgTr::FillBaseVEnv() {
  type::Ty *result;
  type::TyList *formals;

  temp::Label *label = nullptr;
  tr::Level *level = main_level_.get();

  venv_->Enter(sym::Symbol::UniqueSymbol("flush"),
               new env::FunEntry(level, label, new type::TyList(),
                                 type::VoidTy::Instance()));

  formals = new type::TyList(type::IntTy::Instance());

  venv_->Enter(
      sym::Symbol::UniqueSymbol("exit"),
      new env::FunEntry(level, label, formals, type::VoidTy::Instance()));

  result = type::StringTy::Instance();

  venv_->Enter(sym::Symbol::UniqueSymbol("chr"),
               new env::FunEntry(level, label, formals, result));

  venv_->Enter(sym::Symbol::UniqueSymbol("getchar"),
               new env::FunEntry(level, label, new type::TyList(), result));

  formals = new type::TyList(type::StringTy::Instance());

  venv_->Enter(
      sym::Symbol::UniqueSymbol("print"),
      new env::FunEntry(level, label, formals, type::VoidTy::Instance()));
  venv_->Enter(sym::Symbol::UniqueSymbol("printi"),
               new env::FunEntry(level, label,
                                 new type::TyList(type::IntTy::Instance()),
                                 type::VoidTy::Instance()));

  result = type::IntTy::Instance();
  venv_->Enter(sym::Symbol::UniqueSymbol("ord"),
               new env::FunEntry(level, label, formals, result));

  venv_->Enter(sym::Symbol::UniqueSymbol("size"),
               new env::FunEntry(level, label, formals, result));

  result = type::StringTy::Instance();
  formals = new type::TyList(
      {type::StringTy::Instance(), type::StringTy::Instance()});
  venv_->Enter(sym::Symbol::UniqueSymbol("concat"),
               new env::FunEntry(level, label, formals, result));

  formals =
      new type::TyList({type::StringTy::Instance(), type::IntTy::Instance(),
                        type::IntTy::Instance()});
  venv_->Enter(sym::Symbol::UniqueSymbol("substring"),
               new env::FunEntry(level, label, formals, result));

  // Seven arguments testcase
  result = type::IntTy::Instance();
  formals = new type::TyList({type::IntTy::Instance(), type::IntTy::Instance(),
                              type::IntTy::Instance(), type::IntTy::Instance(),
                              type::IntTy::Instance(), type::IntTy::Instance(),
                              type::IntTy::Instance()});
  venv_->Enter(sym::Symbol::UniqueSymbol("sumSeven"),
               new env::FunEntry(level, label, formals, result));
}

} // namespace tr
