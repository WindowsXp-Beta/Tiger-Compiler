#include "tiger/absyn/absyn.h"
#include "tiger/semant/semant.h"

namespace absyn {

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  root_->SemAnalyze(venv, tenv, 0, errormsg);
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *entry = venv->Look(sym_);
  if(entry && typeid(*entry) == typeid(env::VarEntry)) {
    return (static_cast<env::VarEntry *>(entry))->ty_->ActualTy();
  } else {
    errormsg->Error(pos_, "undefined variable %s", sym_->Name().data());
  }
  return type::IntTy::Instance();
}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *record_type = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();

  if(typeid(*record_type) != typeid(type::RecordTy)){
    errormsg->Error(pos_, "not a record type");
    return type::VoidTy::Instance();
  }

  auto fieldlist = static_cast<type::RecordTy *>(record_type)->fields_->GetList();
  auto field_it = fieldlist.begin();
  auto end_flag = fieldlist.end();
  for(; field_it != end_flag; field_it++){
    if((*field_it)->name_ == sym_){
      return (*field_it)->ty_;
    }
  }
  errormsg->Error(pos_, "field %s doesn't exist", sym_->Name().data());
  return type::IntTy::Instance();
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *array_type = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();

  if(typeid(*array_type) != typeid(type::ArrayTy)){
    errormsg->Error(pos_, "array type required");
  }

  return static_cast<type::ArrayTy *>(array_type)->ty_;
}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return var_->SemAnalyze(venv, tenv, labelcount, errormsg);
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  env::EnvEntry *function_called = venv->Look(func_);
  if(function_called == nullptr){
    errormsg->Error(pos_, "undefined function %s", func_->Name().c_str());
    return type::VoidTy::Instance();
  }
  if(typeid(*function_called) != typeid(env::FunEntry)){
    errormsg->Error(pos_, "need to be a function");
    return type::VoidTy::Instance();
  }
  auto formal_list = static_cast<env::FunEntry *>(function_called)->formals_->GetList();
  auto result_type = static_cast<env::FunEntry *>(function_called)->result_;
  auto formal_it = formal_list.begin();
  auto formal_list_end_flag = formal_list.end();

  auto exp_list = args_->GetList();
  auto exp_it = exp_list.begin();
  auto exp_list_end_flag = exp_list.end();

  for(; formal_it != formal_list_end_flag; formal_it++, exp_it++){
    if(exp_it == exp_list_end_flag){
      errormsg->Error(pos_, "para type mismatch");
      return type::VoidTy::Instance();
    }
    type::Ty *exp_type = (*exp_it)->SemAnalyze(venv, tenv, labelcount, errormsg);
    if(!exp_type->IsSameType(*formal_it)){
      errormsg->Error(pos_, "para type mismatch");
      return type::VoidTy::Instance();
    }
  }
  if(exp_it != exp_list_end_flag){
    errormsg->Error(pos_, "too many params in function %s", func_->Name().data());
  }
  return result_type;
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *left_ty = left_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *right_ty = right_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(oper_ == PLUS_OP || oper_ == MINUS_OP || oper_ == TIMES_OP || oper_ == DIVIDE_OP){
    if(typeid(*left_ty) != typeid(type::IntTy)) {
      errormsg->Error(left_->pos_, "integer required");
    }
    if(typeid(*right_ty) != typeid(type::IntTy)) {
      errormsg->Error(right_->pos_, "integer required");
    }
    return type::IntTy::Instance();
  } else {
    if(!left_ty->IsSameType(right_ty)){
      errormsg->Error(pos_, "same type required");
      return type::IntTy::Instance();
    }
  }
  return type::IntTy::Instance();
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *record_type = tenv->Look(typ_);
  if(record_type == nullptr){
    errormsg->Error(pos_, "undefined type %s", typ_->Name().c_str());
    return type::VoidTy::Instance();
  }
  record_type = record_type->ActualTy();
  if(typeid(*record_type) != typeid(type::RecordTy)){
    errormsg->Error(pos_, "not a record type");
  }
  auto fieldlist = static_cast<type::RecordTy *>(record_type)->fields_->GetList();
  auto efieldlist = fields_->GetList();
  auto field_it = fieldlist.begin();
  auto fieldlist_end_flag = fieldlist.end();
  auto efield_it = efieldlist.begin();
  auto efieldlist_end_flag = efieldlist.end();

  for(; field_it != fieldlist_end_flag; field_it++, efield_it++){
    if((*field_it)->name_ != (*efield_it)->name_){
      errormsg->Error(pos_, "field type mismatch");
      return type::VoidTy::Instance();
    }
    type::Ty *efield_type = (*efield_it)->exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if(!(*field_it)->ty_->IsSameType(efield_type)){
      errormsg->Error(pos_, "field and efield type mismatch");
    }
  }
  return record_type;
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto exp_list = seq_->GetList();
  auto exp_it = exp_list.begin();
  auto exp_list_end_flag = exp_list.end();
  type::Ty *return_type;
  for(; exp_it != exp_list_end_flag; exp_it++){
    return_type = (*exp_it)->SemAnalyze(venv, tenv, labelcount, errormsg);
  }
  return return_type;
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *var_type = var_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*var_) == typeid(SimpleVar)){
    env::EnvEntry *var_entry = venv->Look(static_cast<SimpleVar *>(var_)->sym_);
    if(typeid(*var_entry) != typeid(env::VarEntry)){
      errormsg->Error(pos_, "not a variable");
    }
    if(static_cast<env::VarEntry *>(var_entry)->readonly_){
      errormsg->Error(pos_, "loop variable can't be assigned");
    }
  }
  type::Ty *exp_type = exp_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if(!(var_type->IsSameType(exp_type))){
    errormsg->Error(pos_, "unmatched assign exp");
  }
  return type::VoidTy::Instance();
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *test_type = test_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*test_type) != typeid(type::IntTy)){
    errormsg->Error(pos_, "test exp of If needs to be IntTy");
  }
  type::Ty *then_type = then_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(elsee_ != NULL){
    type::Ty *else_type = elsee_->SemAnalyze(venv, tenv, labelcount, errormsg);
    if(!then_type->IsSameType(else_type)){
      errormsg->Error(pos_, "then exp and else exp type mismatch");
    }
  } else {
    if(typeid(*then_type) != typeid(type::VoidTy)){
      errormsg->Error(pos_, "if-then exp's body must produce no value");
    }
  }
  return then_type;
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *test_type = test_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*test_type) != typeid(type::IntTy)){
    errormsg->Error(pos_, "test exp of while needs to be IntTy");
  }
  type::Ty *body_type = body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg)->ActualTy();
  if(typeid(*body_type) != typeid(type::VoidTy)){
    errormsg->Error(pos_, "while body must produce no value");
  }
  return type::VoidTy::Instance();
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  type::Ty *lo_type = lo_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *hi_type = hi_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typeid(*lo_type) != typeid(type::IntTy) || typeid(*hi_type) != typeid(type::IntTy)){
    errormsg->Error(pos_, "for exp's range type is not integer");
  }
  venv->Enter(var_, new env::VarEntry(type::IntTy::Instance(), true));
  type::Ty *body_type = body_->SemAnalyze(venv, tenv, labelcount + 1, errormsg)->ActualTy();
  if(typeid(*body_type) != typeid(type::VoidTy)){
    errormsg->Error(pos_, "for's body must be a void exp");
  }
  venv->EndScope();
  return type::VoidTy::Instance();
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  if(labelcount == 0){
    errormsg->Error(pos_, "break is not inside any loop");
  }
  return type::VoidTy::Instance();
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  tenv->BeginScope();
  for(Dec *dec : decs_->GetList())
    dec->SemAnalyze(venv, tenv, labelcount, errormsg);
  type::Ty *result;
  if(!body_) result = type::VoidTy::Instance();
  else result = body_->SemAnalyze(venv, tenv, labelcount, errormsg);
  //The result of LetExp is the last expression of body

  tenv->EndScope();
  venv->EndScope();
  return result;
}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *array_type = tenv->Look(typ_)->ActualTy();
  if(typeid(*array_type) != typeid(type::ArrayTy)){
    errormsg->Error(pos_, "not array type %s", typ_->Name().data());
  }
  type::Ty *element_type = static_cast<type::ArrayTy *>(array_type)->ty_;
  type::Ty *size_type = size_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  type::Ty *init_type = init_->SemAnalyze(venv, tenv, labelcount, errormsg);
  if(typeid(*size_type) != typeid(type::IntTy)){
    errormsg->Error(pos_, "size of array must be int");
    // return type::VoidTy::Instance();
  }
  if(!(init_type->IsSameType(element_type))){
    errormsg->Error(pos_, "type mismatch");
    // return type::VoidTy::Instance();
  }
  return array_type;
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto end_flag = functions_->GetList().end();
  for(auto fundec_it = functions_->GetList().begin(); fundec_it != end_flag; fundec_it++){
    auto fun_name = (*fundec_it)->name_;
    type::TyList *formal_list = (*fundec_it)->params_->MakeFormalTyList(tenv, errormsg);
    type::Ty *result_type = type::VoidTy::Instance();
    if((*fundec_it)->result_ != nullptr){
      result_type = tenv->Look((*fundec_it)->result_)->ActualTy();
    }
    if(venv->Look(fun_name) != nullptr){
      errormsg->Error(pos_, "two functions have the same name");
      return;
    }
    venv->Enter(fun_name, new env::FunEntry(formal_list, result_type));
  }
  for(auto fundec_it = functions_->GetList().begin(); fundec_it != end_flag; fundec_it++){
    auto fun_name = (*fundec_it)->name_;
    
    env::FunEntry *fun_entry = static_cast<env::FunEntry *>(venv->Look(fun_name));
    auto formal_it = fun_entry->formals_->GetList().begin();
    auto formal_end_flag = fun_entry->formals_->GetList().end();
    auto param_it = (*fundec_it)->params_->GetList().begin();
    auto param_end_flag = (*fundec_it)->params_->GetList().end();
    venv->BeginScope();
    for(; formal_it != formal_end_flag; formal_it++, param_it++){
      venv->Enter((*param_it)->name_, new env::VarEntry(*formal_it));
    }
    type::Ty *return_type = (*fundec_it)->body_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
    if(!fun_entry->result_->IsSameType(return_type)){
      if(typeid(*fun_entry->result_) == typeid(type::VoidTy)){
        errormsg->Error(pos_, "procedure returns value");
      }
      else errormsg->Error(pos_, "return type mismatch");
    }
    venv->EndScope();
  }
}

void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *init_ty = init_->SemAnalyze(venv, tenv, labelcount, errormsg)->ActualTy();
  if(typ_ != NULL && !tenv->Look(typ_)->IsSameType(init_ty)){
    errormsg->Error(pos_, "type mismatch");
  } else if(typ_ == NULL && typeid(*init_ty) == typeid(type::NilTy)){
    errormsg->Error(pos_, "init should not be nil without type specified");
  }
  venv->Enter(var_, new env::VarEntry(init_ty));
}

//{name -> absyn::Ty} -> {name -> Type::Ty}
void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  auto end_flag = types_->GetList().end();
  for(auto iterator = types_->GetList().begin(); iterator != end_flag; iterator++){
    auto name = (*iterator)->name_;
    if(tenv->Look(name) != nullptr){
      errormsg->Error(pos_, "two types have the same name");
      return;
    }
    tenv->Enter(name, new type::NameTy(name, NULL));
  }
  for(auto iterator = types_->GetList().begin(); iterator != end_flag; iterator++){
    auto name = (*iterator)->name_;
    type::NameTy *name_ty = static_cast<type::NameTy *>(tenv->Look(name));
    name_ty->ty_ = (*iterator)->ty_->SemAnalyze(tenv, errormsg);
  }

  bool is_cycle = false;
  for(auto iterator = types_->GetList().begin(); iterator != end_flag; iterator++){
    auto name = (*iterator)->name_;
    auto type = tenv->Look(name);
    while(true){
      type::Ty *name_ty_ty = static_cast<type::NameTy *>(type)->ty_;
      if(typeid(*name_ty_ty) == typeid(type::NameTy)){
        if(static_cast<type::NameTy *>(name_ty_ty)->sym_ == name){
          is_cycle = true;
          break;
        }
        type = name_ty_ty;
      } else {
        break;
      }
    }
    if(is_cycle){
      errormsg->Error(pos_, "illegal type cycle");
      break;
    }
  }
}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *name_ty = tenv->Look(name_);
  if(name_ty == nullptr) {
    errormsg->Error(pos_, "undefined type %s", name_->Name().data());
    return type::VoidTy::Instance();
  }
  return new type::NameTy(name_, name_ty);
}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  return new type::RecordTy(record_->MakeFieldList(tenv, errormsg));
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *array_ty = tenv->Look(array_)->ActualTy();
  if(array_ty == nullptr){
    errormsg->Error(pos_, "undefined type %s", array_->Name().data());
    return type::VoidTy::Instance();
  }
  return new type::ArrayTy(array_ty);
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}
} // namespace sem
