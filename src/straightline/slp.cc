#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  int args1 = stm1->MaxArgs();
  int args2 = stm2->MaxArgs();
  return args1 > args2 ? args1 : args2;
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  return stm2 -> Interp(stm1 -> Interp(t));
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return exp->MaxArgs();
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable *tmp = exp -> Interp(t);
  t = tmp -> t;
  return t -> Update(id, tmp -> i);
}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  int exps_num = exps->NumExps();
  int exps_max = exps->MaxArgs();
  return exps_num > exps_max ? exps_num : exps_max;
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  return exps->Interp(t)->t;
}

int A::IdExp::MaxArgs() const {
  return 0;
}

IntAndTable *A::IdExp::Interp(Table *t) const {
  return new IntAndTable(t -> Lookup(id), t);
}

int A::NumExp::MaxArgs() const {
  return 0;
}

IntAndTable *A::NumExp::Interp(Table *t) const {
    return new IntAndTable(num, t);
}

int A::OpExp::MaxArgs() const {
  int left_num = left->MaxArgs();
  int right_num = right->MaxArgs();
  return left_num > right_num ? left_num : right_num;
}

IntAndTable *A::OpExp::Interp(Table *t) const {
  IntAndTable *tmp = left->Interp(t);
  int left_result = tmp->i;
  tmp = right->Interp(tmp->t);
  int right_result = tmp->i;
  switch (oper){
    case PLUS:
      return new IntAndTable(left_result + right_result, tmp->t);
      break;
    case MINUS:
      return new IntAndTable(left_result - right_result, tmp->t);
      break;
    case TIMES:
      return new IntAndTable(left_result * right_result, tmp->t);
      break;
    case DIV:
      return new IntAndTable(left_result / right_result, tmp->t);
      break;
    default:
      break;
  }
}

int A::EseqExp::MaxArgs() const {
  int stm_num = stm->MaxArgs();
  int exp_num = exp->MaxArgs();
  return stm_num > exp_num ? stm_num : exp_num;
}

IntAndTable *A::EseqExp::Interp(Table *t) const { 
  t = stm->Interp(t);
  return exp->Interp(t);
}

int A::PairExpList::MaxArgs() const {
  int exp_num = exp->MaxArgs();
  int tail_num = tail->MaxArgs();
  return exp_num > tail_num ? exp_num : tail_num;
}

int A::PairExpList::NumExps() const {
  return 1 + tail->NumExps();
}

IntAndTable *A::PairExpList::Interp(Table *t) const {
  IntAndTable *tmp = exp->Interp(t);
  std::cout << tmp->i << ' ';
  return tail->Interp(tmp->t);
}

int A::LastExpList::MaxArgs() const {
  return exp->MaxArgs();
}

int A::LastExpList::NumExps() const {
  return 1;
}

IntAndTable *A::LastExpList::Interp(Table *t) const {
  IntAndTable *tmp = exp->Interp(t);
  std::cout << tmp->i << '\n';
  return tmp;
}

int Table::Lookup(const std::string &key) const {
  if (id == key) {
    return value;
  } else if (tail != nullptr) {
    return tail->Lookup(key);
  } else {
    assert(false);
  }
}

Table *Table::Update(const std::string &key, int val) const {
  return new Table(key, val, this);
}
}  // namespace A
