#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;

namespace tr {

Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  auto frame_access = level->frame_->AllocLocal(escape);
  return new Access(level, frame_access);
}

// Conditional Jump
class Cx {
public:
  temp::Label **trues_;
  temp::Label **falses_;
  tree::Stm *stm_;

  Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : trues_(trues), falses_(falses), stm_(stm) {}
};

class Exp {
public:
  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;//convert to Exp
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;//convert to Stm
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;//convert to Cx
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    return exp_;
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(exp_);
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    // temp::Label *t = temp::LabelFactory::NewLabel();
    // temp::Label *f = temp::LabelFactory::NewLabel();
    tree::CjumpStm *stm = new tree::CjumpStm(tree::RelOp::NE_OP, exp_, new tree::ConstExp(0), nullptr, nullptr);
    return Cx(&(stm->true_label_), &(stm->false_label_), stm);
  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::EseqExp(stm_, new tree::ConstExp(0));
  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    return stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    errormsg->Error(0, "cannot convert a NxExp to a Cx\n");
    return Cx(nullptr, nullptr, nullptr);
  }
};

class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
      : cx_(trues, falses, stm) {}
  
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    temp::Temp *r = temp::TempFactory::NewTemp();
    temp::Label *t = temp::LabelFactory::NewLabel();
    temp::Label *f = temp::LabelFactory::NewLabel();
    *cx_.trues_ = t;
    *cx_.falses_ = f;
    return new tree::EseqExp(
      new tree::MoveStm(new tree::TempExp(r), new tree::ConstExp(1)),
      new tree::EseqExp(
        cx_.stm_,
        new tree::EseqExp(
          new tree::LabelStm(f),
          new tree::EseqExp(
            new tree::MoveStm(
              new tree::TempExp(r), 
              new tree::ConstExp(0)
            ),
            new tree::EseqExp(
              new tree::LabelStm(t),
              new tree::TempExp(r)
            )
          )
        )
      )
    );
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(UnEx());
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return cx_;
  }
};

tree::Exp *StaticLink(Level *current, Level *target) {
  tree::Exp *frame_ptr = new tree::TempExp(reg_manager->FramePointer());
  while (current != target) {
    frame_ptr = current->frame_->formals_->at(0)->ToExp(frame_ptr);
    //static link is always the first formal parameter
    current = current->parent_;
  }
  return frame_ptr;
}

inline tree::MoveStm *InitializeRecord(temp::Temp* record_temp, Exp *init_exp, int index) {
  return new tree::MoveStm(
    new tree::MemExp(
        new tree::BinopExp(
          tree::BinOp::PLUS_OP,
          new tree::TempExp(record_temp),
          new tree::ConstExp(index * reg_manager->WordSize())
        )),
    init_exp->UnEx()
  );
}

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  // label is only used in breakexp, so it's ok to pass a nullptr
  auto tr_absyn = absyn_tree_->Translate(venv_.get(), tenv_.get(), main_level_.get(), nullptr, errormsg_.get());
  frags->PushBack(frame::ProcEntryExit1(main_level_->frame_, tr_absyn->exp_->UnNx()));
}

} // namespace tr

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return root_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto var_entry = static_cast<env::VarEntry *>(venv->Look(sym_));
  return new tr::ExpAndTy(
    new tr::ExExp(
      var_entry->access_->access_->ToExp(
        tr::StaticLink(level, var_entry->access_->level_))),
    var_entry->ty_->ActualTy()
  );
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto var_exp = var_->Translate(venv, tenv, level, label, errormsg);
  auto field_list = static_cast<type::RecordTy *>(var_exp->ty_)->fields_->GetList();
  int offset = 0;
  type::Ty *type = nullptr;
  for (auto field : field_list) {
    if (field->name_ == sym_) {
      type = field->ty_;
      break;
    }
    offset++;
  }
  return new tr::ExpAndTy(
    new tr::ExExp(
      new tree::MemExp(
        new tree::BinopExp(
          tree::BinOp::PLUS_OP,
          var_exp->exp_->UnEx(),
          new tree::ConstExp(
            offset * reg_manager->WordSize()
          )
        )
      )
    ),
    type
  );
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto var_exp = var_->Translate(venv, tenv, level, label, errormsg);
  auto subscript_exp = subscript_->Translate(venv, tenv, level, label, errormsg);
  return new tr::ExpAndTy(
    new tr::ExExp(
      new tree::MemExp(
        new tree::BinopExp(
          tree::BinOp::PLUS_OP,
          var_exp->exp_->UnEx(),
          new tree::BinopExp(
            tree::BinOp::MUL_OP,
            subscript_exp->exp_->UnEx(),
            new tree::ConstExp(reg_manager->WordSize()))))),
      static_cast<type::ArrayTy *>(var_exp->ty_)->ty_
  );
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return var_->Translate(venv, tenv, level, label, errormsg);
}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(0)),
    type::NilTy::Instance()
  );
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(val_)),
    type::IntTy::Instance()
  );
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto string_label = temp::LabelFactory::NewLabel();
  frags->PushBack(new frame::StringFrag(string_label, str_));
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::NameExp(string_label)),
    type::StringTy::Instance()
  );
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto function_entry = static_cast<env::FunEntry *>(venv->Look(func_));
  auto function_level = function_entry->level_;
  auto absyn_arg_list = args_->GetList();
  auto tree_arg_list = new tree::ExpList();
  for (auto arg : absyn_arg_list) {
    auto arg_exp = arg->Translate(venv, tenv, level, label, errormsg);
    tree_arg_list->Append(arg_exp->exp_->UnEx());
  }
  tree::Exp *call_exp;
  if (function_level == nullptr) {
    // external calls don't need static link
    call_exp = frame::ExternalCall(
      func_->Name(),
      tree_arg_list
    );
  } else {
    tree_arg_list->Insert(tr::StaticLink(level, function_entry->level_->parent_));
    call_exp = new tree::CallExp(
      new tree::NameExp(function_entry->label_),
      tree_arg_list
    );
  }
  return new tr::ExpAndTy(
    new tr::ExExp(call_exp),
    function_entry->result_
  );
}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto tr_left = left_->Translate(venv, tenv, level, label, errormsg);
  auto tr_right = right_->Translate(venv, tenv, level, label, errormsg);
  tr::Exp *tr_exp;
  switch (oper_) {
    case PLUS_OP:
    case MINUS_OP:
    case TIMES_OP:
    case DIVIDE_OP:
    tr_exp = new tr::ExExp(
      new tree::BinopExp(
        static_cast<tree::BinOp>(oper_),
        tr_left->exp_->UnEx(),
        tr_right->exp_->UnEx()
      )
    );
    break;
    case LT_OP:
    case LE_OP: 
    case GT_OP:
    case GE_OP:
    {
      auto stm = new tree::CjumpStm(
        static_cast<tree::RelOp>(oper_ - 4),
        tr_left->exp_->UnEx(),
        tr_right->exp_->UnEx(),
        nullptr,//true label
        nullptr //false label
      );
      tr_exp = new tr::CxExp(
        &stm->true_label_,
        &stm->false_label_,
        stm);
    }
    case EQ_OP:
    case NEQ_OP:
    if (tr_left->ty_->IsSameType(type::StringTy::Instance())) {
      auto exp_list = new tree::ExpList{tr_left->exp_->UnEx(), tr_right->exp_->UnEx()};
      auto stm = new tree::CjumpStm(
        tree::RelOp::EQ_OP,
        frame::ExternalCall("string_equal", exp_list),
        new tree::ConstExp(1),
        nullptr,
        nullptr
      );
      tr_exp = new tr::CxExp(
        &stm->true_label_,
        &stm->false_label_,
        stm
      );
    } else {
      auto stm = new tree::CjumpStm(
        static_cast<tree::RelOp>(oper_ - 4),
        tr_left->exp_->UnEx(),
        tr_right->exp_->UnEx(),
        nullptr,
        nullptr
      );
      tr_exp = new tr::CxExp(
        &stm->true_label_,
        &stm->false_label_,
        stm
      );
    }
    break;
  }
  return new tr::ExpAndTy(
    tr_exp,
    type::IntTy::Instance()
  );
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto record_temp = temp::TempFactory::NewTemp();
  size_t fields_size = fields_->GetList().size();
  auto alloc_stm = new tree::MoveStm(
    new tree::TempExp(record_temp),
    new tree::CallExp(
      new tree::NameExp(temp::LabelFactory::NamedLabel("alloc_record")),
      new tree::ExpList{new tree::ConstExp(fields_size * reg_manager->WordSize())}
    ));
  // reverse the efieldlist
  // TODO:use recursive function
  tree::Stm *init_stm = nullptr;
  auto end_flag = fields_->GetList().rend();
  auto efield_it = fields_->GetList().rbegin();
  if (efield_it != end_flag) {
    auto tr_efield = (*efield_it)->exp_->Translate(venv, tenv, level, label, errormsg);
    init_stm = tr::InitializeRecord(record_temp, tr_efield->exp_, --fields_size);
    efield_it++;
    for (; efield_it != end_flag; efield_it++) {
      tr_efield = (*efield_it)->exp_->Translate(venv, tenv, level, label, errormsg);
      init_stm = new tree::SeqStm(
        tr::InitializeRecord(record_temp, tr_efield->exp_, --fields_size),
        init_stm);
    }
  }
  return new tr::ExpAndTy(
    new tr::ExExp(
      new tree::EseqExp(
        new tree::SeqStm(
          alloc_stm, init_stm
        ),
        new tree::TempExp(record_temp)
      )
    ),
    tenv->Look(typ_)
  );
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto exp_it = seq_->GetList().begin();
  auto end_flag = seq_->GetList().end();
  if (exp_it == end_flag) {
    return new tr::ExpAndTy(
      nullptr,
      type::VoidTy::Instance()
    );
  } else {
    auto tr_exp = (*exp_it)->Translate(venv, tenv, level, label, errormsg);
    if (++exp_it == end_flag) {
      return tr_exp;
    } else {
      auto seq_stm = tr_exp->exp_->UnNx();
      for (; std::next(exp_it) != end_flag; exp_it++) {
        seq_stm = new tree::SeqStm(
          seq_stm,
          (*exp_it)->Translate(venv, tenv, level, label, errormsg)->exp_->UnNx()
        );
      }
      auto tr_end = (*exp_it)->Translate(venv, tenv, level, label, errormsg);
      return new tr::ExpAndTy(
        new tr::ExExp(
          new tree::EseqExp(
            seq_stm,
            tr_end->exp_->UnEx()
          )
        ),
        tr_end->ty_
      );
    }
  }
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto tr_var = var_->Translate(venv, tenv, level, label, errormsg);
  auto tr_exp = exp_->Translate(venv, tenv, level, label, errormsg);
  return new tr::ExpAndTy(
    new tr::NxExp(
      new tree::MoveStm(
        tr_var->exp_->UnEx(), tr_exp->exp_->UnEx()
      )),
    type::VoidTy::Instance()
  );
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto tr_test = test_->Translate(venv, tenv, level, label, errormsg);
  auto tr_then = then_->Translate(venv, tenv, level, label, errormsg);
  auto true_label = temp::LabelFactory::NewLabel();
  auto false_label = temp::LabelFactory::NewLabel();
  auto joint_label = temp::LabelFactory::NewLabel();
  auto test2cx = tr_test->exp_->UnCx(errormsg);
  *test2cx.falses_ = false_label;
  *test2cx.trues_ = true_label;
  auto result_temp = temp::TempFactory::NewTemp();
  if (elsee_) {
    auto tr_else = elsee_->Translate(venv, tenv, level, label, errormsg);
    auto seq_stm = new tree::SeqStm(
      // tr_test is a CxExp
      // tr_test->exp_->UnNx(),
      test2cx.stm_,
      new tree::SeqStm(
        new tree::LabelStm(true_label),
        new tree::SeqStm(
          new tree::MoveStm(
            new tree::TempExp(result_temp),
            tr_then->exp_->UnEx()
          ),
          new tree::SeqStm(
            new tree::JumpStm(new tree::NameExp(joint_label), new std::vector<temp::Label *>{joint_label}),
            new tree::SeqStm(
              new tree::LabelStm(false_label),
              new tree::SeqStm(
                new tree::MoveStm(
                  new tree::TempExp(result_temp),
                  tr_else->exp_->UnEx()
                ),
                new tree::SeqStm(
                  new tree::JumpStm(new tree::NameExp(joint_label), new std::vector<temp::Label *>({joint_label})),
                  new tree::LabelStm(joint_label)
                )
              )
            )
          )
        )
      )
    );
    return new tr::ExpAndTy(
      new tr::ExExp(
        new tree::EseqExp(
          seq_stm,
          new tree::TempExp(result_temp)
        )
      ),
      tr_then->ty_
    );
  } else {
    auto seq_stm = new tree::SeqStm(
      // tr_test is a CxExp
      // tr_test->exp_->UnNx(),
      test2cx.stm_,
      new tree::SeqStm(
        new tree::LabelStm(true_label),
        new tree::SeqStm(
          tr_then->exp_->UnNx(),
          // new tree::MoveStm(
          //   new tree::TempExp(result_temp),
          //   tr_then->exp_->UnEx()
          // ),
          new tree::LabelStm(false_label)
        )
      )
    );
    delete result_temp;
    delete joint_label;// in case of memory leak
    return new tr::ExpAndTy(
      new tr::NxExp(
        seq_stm
      ),
      type::VoidTy::Instance()
    );
  }
}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto test_label = temp::LabelFactory::NewLabel();
  auto body_label = temp::LabelFactory::NewLabel();
  auto done_label = temp::LabelFactory::NewLabel();
  auto tr_test = test_->Translate(venv, tenv, level, label, errormsg);
  auto tr_body = body_->Translate(venv, tenv, level, done_label, errormsg);
  auto test2Cx = tr_test->exp_->UnCx(errormsg);
  *test2Cx.trues_ = body_label;
  *test2Cx.falses_ = done_label;
  auto seq_stm = new tree::SeqStm(
    new tree::LabelStm(test_label),
    new tree::SeqStm(
      test2Cx.stm_,
      new tree::SeqStm(
        new tree::LabelStm(body_label),
        new tree::SeqStm(
          tr_body->exp_->UnNx(),
          new tree::SeqStm(
            new tree::JumpStm(new tree::NameExp(test_label), new std::vector<temp::Label *>({test_label})),
            new tree::LabelStm(done_label)
          )
        )
      )
    )
  );
  return new tr::ExpAndTy(
    new tr::NxExp(
      seq_stm
    ),
    type::VoidTy::Instance()
  );
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  venv->BeginScope();
  // iterator variable i
  auto iterator_access = tr::Access::AllocLocal(level, escape_);
  auto tr_low = lo_->Translate(venv, tenv, level, label, errormsg);
  auto tr_high = hi_->Translate(venv, tenv, level, label, errormsg);
  auto test_label = temp::LabelFactory::NewLabel();
  auto body_label = temp::LabelFactory::NewLabel();
  auto done_label = temp::LabelFactory::NewLabel();
  venv->Enter(var_, new env::VarEntry(iterator_access, tr_low->ty_));
  auto tr_body = body_->Translate(venv, tenv, level, done_label, errormsg);
  auto iterator_exp = iterator_access->access_->ToExp(new tree::TempExp(reg_manager->FramePointer()));
  auto seq_stm = new tree::SeqStm(
    new tree::MoveStm(
      iterator_exp,
      tr_low->exp_->UnEx()
    ),
    new tree::SeqStm(
      new tree::LabelStm(test_label),
      new tree::SeqStm(
        new tree::CjumpStm(
          tree::RelOp::LE_OP,
          iterator_exp,
          tr_high->exp_->UnEx(),
          body_label,
          done_label
        ),
        new tree::SeqStm(
          new tree::LabelStm(body_label),
          new tree::SeqStm(
            tr_body->exp_->UnNx(),
            new tree::SeqStm(
              new tree::MoveStm(
                iterator_exp,
                new tree::BinopExp(
                  tree::BinOp::PLUS_OP,
                  iterator_exp,
                  new tree::ConstExp(1)
                )
              ),
              new tree::SeqStm(
                new tree::JumpStm(
                  new tree::NameExp(test_label),
                  new std::vector<temp::Label *>{test_label}
                ),
                new tree::LabelStm(done_label)
              )
            )
          )
        )
      )
    )
  );
  venv->EndScope();
  return new tr::ExpAndTy(
    new tr::NxExp(
      seq_stm
    ),
    type::VoidTy::Instance()
  );
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  // label should be done of while or for loop
  return new tr::ExpAndTy(
    new tr::NxExp(new tree::JumpStm(new tree::NameExp(label), new std::vector<temp::Label *>{label})),
    type::VoidTy::Instance()
  );
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  venv->BeginScope();
  tenv->BeginScope();
  tree::Stm *seq_stm = nullptr;
  for (auto dec : decs_->GetList()) {
    if (seq_stm == nullptr) {
      seq_stm = dec->Translate(venv, tenv, level, label, errormsg)->UnNx();
    } else {
      seq_stm = new tree::SeqStm(
        seq_stm,
        dec->Translate(venv, tenv, level, label, errormsg)->UnNx()
      );
    }
  }
  tr::ExpAndTy *tr_body;
  if (!body_) {
    tr_body = new tr::ExpAndTy(
      new tr::ExExp(new tree::ConstExp(0)),
      type::VoidTy::Instance()
    );
  } else {
    tr_body = body_->Translate(venv, tenv, level, label, errormsg);
  }
  tr::ExpAndTy *tr_return;
  if (!seq_stm) {
    // dec is empty
    tr_return = tr_body;
  } else {
    tr_return = new tr::ExpAndTy(
      new tr::ExExp(
        new tree::EseqExp(
          seq_stm,
          tr_body->exp_->UnEx()
        )
      ),
      tr_body->ty_
    );
  }
  tenv->EndScope();
  venv->EndScope();
  return tr_return;
}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto array_type = tenv->Look(typ_)->ActualTy();
  auto element_type = static_cast<type::ArrayTy *>(array_type)->ty_;
  auto tr_size = size_->Translate(venv, tenv, level, label, errormsg);
  auto tr_init = init_->Translate(venv, tenv, level, label, errormsg);
  auto exp_list = new tree::ExpList();
  exp_list->Append(tr_size->exp_->UnEx());
  exp_list->Append(tr_init->exp_->UnEx());
  return new tr::ExpAndTy(
    new tr::ExExp(
      new tree::CallExp(
        new tree::NameExp(
          temp::LabelFactory::NamedLabel("init_array")),
          exp_list
      )),
    array_type
  );
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    nullptr, type::VoidTy::Instance()
  );
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  for (auto fun_dec : functions_->GetList()) {
    auto formal_list = fun_dec->params_->GetList();
    auto escape_list = new std::list<bool>;
    for (auto formal : formal_list) {
      escape_list->push_back(formal->escape_);
    }
    auto fun_label = temp::LabelFactory::NamedLabel(fun_dec->name_->Name());
    auto fun_level = new tr::Level(
      level,
      fun_label,
      escape_list
    );
    auto formal_ty_list = fun_dec->params_->MakeFormalTyList(tenv, errormsg);
    type::Ty *result_type = type::VoidTy::Instance();
    if (fun_dec->result_ != nullptr) {
      result_type = tenv->Look(fun_dec->result_)->ActualTy();
    }
    venv->Enter(
      fun_dec->name_,
      new env::FunEntry(
        fun_level, fun_label,
        formal_ty_list, result_type
      )
    );
    delete escape_list;
  }

  for (auto fun_dec : functions_->GetList()) {
    env::FunEntry *fun_entry = static_cast<env::FunEntry *>(venv->Look(fun_dec->name_));
    auto param = fun_dec->params_->GetList().begin();
    auto param_end_flag = fun_dec->params_->GetList().end();
    auto frame_formal = fun_entry->level_->frame_->formals_->begin();
    auto type_formal = fun_entry->formals_->GetList().begin();
    frame_formal++;// skip the first parameter which is the framepointer for staticlink
    venv->BeginScope();
    for (; param != param_end_flag; param++, frame_formal++, type_formal++) {
      venv->Enter(
        (*param)->name_,
        new env::VarEntry(
          new tr::Access(fun_entry->level_, *frame_formal),
          *type_formal
        )
      );
    }
    auto tr_fun_body = fun_dec->body_->Translate(venv, tenv, fun_entry->level_, fun_entry->label_, errormsg);
    tree::MoveStm *move_return = new tree::MoveStm(
      new tree::TempExp(reg_manager->ReturnValue()),
      tr_fun_body->exp_->UnEx()
    );
    frags->PushBack(frame::ProcEntryExit1(fun_entry->level_->frame_, move_return));
    venv->EndScope();
  }
  return new tr::ExExp(new tree::ConstExp(0));
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto var_access = tr::Access::AllocLocal(level, escape_);
  tr::ExpAndTy *var_init = init_->Translate(venv, tenv, level, label, errormsg);
  venv->Enter(var_, new env::VarEntry(var_access, var_init->ty_));
  return new tr::NxExp(
          new tree::MoveStm(
            var_access->access_->ToExp(
              new tree::TempExp(
                reg_manager->FramePointer()
              )
            ),
            var_init->exp_->UnEx()
          )
        );
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  for (auto type_dec : types_->GetList()) {
    tenv->Enter(type_dec->name_, new type::NameTy(type_dec->name_, nullptr));
  }

  for (auto type_dec : types_->GetList()) {
    auto name_ty = static_cast<type::NameTy *>(tenv->Look(type_dec->name_));
    name_ty->ty_ = type_dec->ty_->Translate(tenv, errormsg);
  }
  return new tr::ExExp(new tree::ConstExp(0));
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  auto name_ty_ty = tenv->Look(name_);
  return new type::NameTy(name_, name_ty_ty);
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new type::RecordTy(record_->MakeFieldList(tenv, errormsg));
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new type::ArrayTy(tenv->Look(array_)->ActualTy());
}

} // namespace absyn
