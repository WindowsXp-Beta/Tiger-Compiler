#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"

namespace {
frame::ProcFrag *ProcEntryExit(tr::Level *level, tr::Exp *body,
                               frame::RegManager *reg_manager);
}

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape,
                           frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

class Cx {
public:
  temp::Label **trues;
  temp::Label **falses;
  tree::Stm *stm;

  Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : trues(trues), falses(falses), stm(stm) {}
};

class Exp {
public:
  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;
};

class ExpAndTy {
public:
  tr::Exp *exp;
  type::Ty *ty;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp(exp), ty(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp;

  explicit ExExp(tree::Exp *exp) : exp(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm;

  explicit NxExp(tree::Stm *stm) : stm(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
  }
};

class CxExp : public Exp {
public:
  Cx cx;

  CxExp(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : cx(trues, falses, stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
  }
};

void ProgTr::Translate() {
  FillBaseVEnv();
  FillBaseTEnv();
  temp::Label *main_label = temp::LabelFactory::NewLabel();
  tr::ExpAndTy *main_exp_ty = absyn_tree_->Translate(
      venv_.get(), tenv_.get(), main_level_.get(), main_label, frags_.get(),
      reg_manager_.get(), errormsg_.get());
  frame::ProcFrag *proc_frag =
      ProcEntryExit(main_level_.get(), main_exp_ty->exp, reg_manager_.get());
  frags_->PushBack(proc_frag);
}

} // namespace tr

namespace {

/**
 * Wrapper for `ProcExitEntry1`, which deals with the return value of the
 * function body
 * @param level current level
 * @param body function body
 * @return statements after `ProcExitEntry1`
 */
frame::ProcFrag *ProcEntryExit(tr::Level *level, tr::Exp *body,
                               frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}
} // namespace

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   frame::Frags *frags,
                                   frame::RegManager *reg_manager,
                                   err::ErrorMsg *errormsg) const {
  if (!root_ || !venv || !tenv || !level || !label)
    throw std::invalid_argument("NULL pointer argument");
  return root_->Translate(venv, tenv, level, label, frags, reg_manager,
                          errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   frame::Frags *frags,
                                   frame::RegManager *reg_manager,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  frame::Frags *frags,
                                  frame::RegManager *reg_manager,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      frame::Frags *frags,
                                      frame::RegManager *reg_manager,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   frame::Frags *frags,
                                   frame::RegManager *reg_manager,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 frame::Frags *frags,
                                 frame::RegManager *reg_manager,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               frame::Frags *frags,
                               frame::RegManager *reg_manager,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   frame::Frags *frags,
                                   frame::RegManager *reg_manager,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   frame::Frags *frags,
                                   frame::RegManager *reg_manager,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               frame::Frags *frags,
                               frame::RegManager *reg_manager,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  frame::Frags *frags,
                                  frame::RegManager *reg_manager,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  frame::Frags *frags,
                                  frame::RegManager *reg_manager,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  frame::Frags *frags,
                                  frame::RegManager *reg_manager,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 frame::Frags *frags,
                                 frame::RegManager *reg_manager,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                frame::Frags *frags,
                                frame::RegManager *reg_manager,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           frame::Frags *frags, frame::RegManager *reg_manager,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            frame::Frags *frags, frame::RegManager *reg_manager,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
}

} // namespace absyn
