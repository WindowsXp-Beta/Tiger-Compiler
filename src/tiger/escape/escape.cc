#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TODO: Put your lab5 code here */
  root_->Traverse(env, 0);
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto escape_entry = env->Look(sym_);
  if (escape_entry == nullptr) {
    printf("error: %s not found\n", sym_->Name().c_str());
  }
  if (escape_entry != nullptr && depth > escape_entry->depth_) {
    *escape_entry->escape_ = true;
  }
}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
}

void VarExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto arg_list = args_->GetList();
  auto arg_list_iterator = arg_list.begin();
  auto end_flag = arg_list.end();
  for (; arg_list_iterator != end_flag; arg_list_iterator++) {
    (*arg_list_iterator)->Traverse(env, depth);
  }
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  left_->Traverse(env, depth);
  right_->Traverse(env, depth);
}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto efield_list = fields_->GetList();
  auto efield_list_iterator = efield_list.begin();
  auto end_flag = efield_list.end();
  for (; efield_list_iterator != end_flag; efield_list_iterator++) {

    (*efield_list_iterator)->exp_->Traverse(env, depth);
  }
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto seq_list = seq_->GetList();
  auto seq_iterator = seq_list.begin();
  auto end_flag = seq_list.end();
  for (; seq_iterator != end_flag; seq_iterator++) {
    (*seq_iterator)->Traverse(env, depth);
  }
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  var_->Traverse(env, depth);
  exp_->Traverse(env, depth);
}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);
  then_->Traverse(env, depth);
  if (elsee_ != nullptr) elsee_->Traverse(env, depth);
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  test_->Traverse(env, depth);
  body_->Traverse(env, depth);
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &escape_));
  lo_->Traverse(env, depth);
  hi_->Traverse(env, depth);
  body_->Traverse(env, depth);
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  auto dec_list = decs_->GetList();
  auto dec_list_iterator = dec_list.begin();
  auto end_flag = dec_list.end();
  for (; dec_list_iterator != end_flag; dec_list_iterator++) {
    (*dec_list_iterator)->Traverse(env, depth);
  }
  body_->Traverse(env, depth);
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  size_->Traverse(env, depth);
  init_->Traverse(env, depth);
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  depth++;
  auto fun_dec_list = functions_->GetList();
  auto fun_dec_iterator = fun_dec_list.begin();
  auto end_flag = fun_dec_list.end();
  for (; fun_dec_iterator != end_flag; fun_dec_iterator++) {
    auto field_list = (*fun_dec_iterator)->params_->GetList();
    auto field_list_iterator = field_list.begin();
    auto field_end_flag = field_list.end();
    for (; field_list_iterator != field_end_flag; field_list_iterator++) {
      auto field = *field_list_iterator;
      field->escape_ = false;
      env->Enter(field->name_, new esc::EscapeEntry(depth, &field->escape_));
    }
    (*fun_dec_iterator)->body_->Traverse(env, depth);
  }
}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  init_->Traverse(env, depth);
  escape_ = false;
  env->Enter(var_, new esc::EscapeEntry(depth, &escape_));
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  return ;
}

} // namespace absyn
