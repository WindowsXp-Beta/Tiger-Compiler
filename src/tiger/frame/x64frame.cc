#include "tiger/frame/x64frame.h"

extern frame::RegManager *reg_manager;

namespace frame {
/* TODO: Put your lab5 code here */
X64RegManager::X64RegManager() {
  for (int i = 0; i < 16; i++) {
      regs_.push_back(temp::TempFactory::NewTemp());
    }
}

temp::TempList *X64RegManager::Registers() {
  auto general_purpose_regs = new temp::TempList();
  for (auto reg : regs_) {
    general_purpose_regs->Append(reg);
  }
  return general_purpose_regs;
}

temp::TempList *X64RegManager::ArgRegs() {
  auto arg_regs = new temp::TempList();
  arg_regs->Append(regs_[5]);
  arg_regs->Append(regs_[4]);
  arg_regs->Append(regs_[3]);
  arg_regs->Append(regs_[2]);
  arg_regs->Append(regs_[8]);
  arg_regs->Append(regs_[9]);
  return arg_regs;
}

temp::TempList *X64RegManager::CalleeSaves() {
  auto callee_saves_regs = new temp::TempList();
  callee_saves_regs->Append(regs_[1]);
  callee_saves_regs->Append(regs_[6]);
  callee_saves_regs->Append(regs_[12]);
  callee_saves_regs->Append(regs_[13]);
  callee_saves_regs->Append(regs_[14]);
  callee_saves_regs->Append(regs_[15]);
  return callee_saves_regs;
}

temp::TempList *X64RegManager::CallerSaves() {
  auto caller_saves_regs = new temp::TempList();
  caller_saves_regs->Append(regs_[9]);
  caller_saves_regs->Append(regs_[10]);
  return caller_saves_regs;
}

temp::TempList *X64RegManager::ReturnSink() {
  return nullptr;
}

int X64RegManager::WordSize() {
  return 8;
}

temp::Temp *X64RegManager::FramePointer() {
  return regs_[6];
}

temp::Temp *X64RegManager::StackPointer() {
  return regs_[7];
}

temp::Temp *X64RegManager::ReturnValue() {
  return regs_[0];
}

tree::Exp *InFrameAccess::ToExp(tree::Exp *framePtr) const {
  return new tree::MemExp(
    new tree::BinopExp(tree::BinOp::PLUS_OP,
    framePtr, new tree::ConstExp(offset)));
}

tree::Exp *InRegAccess::ToExp(tree::Exp *framePtr) const {
  return new tree::TempExp(reg);
}

X64Frame::X64Frame(temp::Label *name, std::list<bool> *formals) :
  Frame(name, formals) {
    //initialize and delete formals_ is in Frame() and ~Frame()
    if (formals != nullptr) {
      for (auto formal_flag : *formals) {
        formals_->push_back(AllocLocal(formal_flag));
      }
      size_t formal_size = formals_->size();
      tree::MoveStm *move_stm;
      auto frame_ptr = new tree::TempExp(reg_manager->FramePointer());
      for (int i = 0; i < formal_size; i++) {
        switch (i) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          move_stm = new tree::MoveStm(
            (*formals_)[i]->ToExp(frame_ptr),
            new tree::TempExp(reg_manager->ArgRegs()->NthTemp(i))
          );
          break;
          default:
          move_stm = new tree::MoveStm(
            (*formals_)[i]->ToExp(frame_ptr),
            new tree::MemExp(
              new tree::BinopExp(
                tree::BinOp::PLUS_OP,
                frame_ptr,
                new tree::ConstExp(reg_manager->WordSize() * (i - 6))
              )
            )
          );
        }
        view_shift.push_back(move_stm);
      }
    }
}

tree::Exp *ExternalCall(std::string fun, tree::ExpList *args) {
  auto extern_label = temp::LabelFactory::NamedLabel(fun);
  return new tree::CallExp(
    new tree::NameExp(extern_label),
    args);
}

Access *X64Frame::AllocLocal(bool isEscape) {
  Access *new_local;
  if (isEscape) {
    new_local = new InFrameAccess(current_stack_ptr);
    current_stack_ptr -= reg_manager->WordSize();
  } else {
    new_local = new InRegAccess(temp::TempFactory::NewTemp());
  }
  return new_local;
}

ProcFrag *ProcEntryExit1(frame::Frame *frame, tree::Stm *stm) {
  auto stm_it = frame->view_shift.begin();
  auto end_flag = frame->view_shift.end();
  if (stm_it != end_flag) {
    auto seq_stm = *stm_it;
    stm_it++;
    for (; stm_it != end_flag; stm_it++) {
      seq_stm = new tree::SeqStm(seq_stm, *stm_it);
    }
    stm = new tree::SeqStm(seq_stm, stm);
  }
  return new ProcFrag(stm, frame);
}

/* TODO: Put your lab5 code here */
} // namespace frame