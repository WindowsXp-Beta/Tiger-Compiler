#include "tiger/frame/x64frame.h"
#include <map>
#include <sstream>

extern frame::RegManager *reg_manager;

namespace frame {
/* TODO: Put your lab5 code here */
X64RegManager::X64RegManager() {
  auto x64_reg = std::map<int, std::string *>{
   {0, new std::string("%rax")}, {1, new std::string("%rbx")},
   {2, new std::string("%rcx")}, {3, new std::string("%rdx")},
   {4, new std::string("%rsi")}, {5, new std::string("%rdi")},
   {6, new std::string("%rbp")}, {7, new std::string("%rsp")},
   {8, new std::string("%r8")}, {9, new std::string("%r9")},
   {10, new std::string("%r10")}, {11, new std::string("%r11")},
   {12, new std::string("%r12")}, {13, new std::string("%r13")},
   {14, new std::string("%r14")}
  };
  for (int i = 0; i < 16; i++) {
    auto new_temp = temp::TempFactory::NewTemp();
    regs_.push_back(new_temp);
    temp_map_->Enter(new_temp, x64_reg[i]);
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
  return new temp::TempList{
    regs_[5], regs_[4], regs_[3], regs_[2], regs_[8], regs_[9]
  };
}

temp::TempList *X64RegManager::CalleeSaves() {
  return new temp::TempList{
    regs_[1], regs_[6], regs_[12], regs_[13], regs_[14], regs_[15]
  };
}

temp::TempList *X64RegManager::CallerSaves() {
  return new temp::TempList{
    regs_[9], regs_[10]
  };
}

temp::TempList *X64RegManager::ReturnSink() {
  auto temp_list = CalleeSaves();
  temp_list->Append(StackPointer());
  temp_list->Append(ReturnValue());
  return temp_list;
}

int X64RegManager::WordSize() {
  return 8;
}

temp::Temp *X64RegManager::FramePointer() {
  return regs_[6];
  // change the return value from %rbp to %rsp 
  // so we don't need to change previous invocation
  // Above are bullshit...
  // otherwise it will cause previous call to StackPointer
  // also match the judge case like(some_reg == reg->FramePointer())
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
                new tree::ConstExp(reg_manager->WordSize() * (i + 1 /* skip the return address */ - 6))
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

assem::InstrList *ProcEntryExit2(assem::InstrList *body) {
  body->Append(
    new assem::OperInstr(
      "", nullptr,
      reg_manager->ReturnSink(),
      nullptr
    )
  );
  return body;
}

assem::Proc *ProcEntryExit3(frame::Frame *frame, assem::InstrList *body) {
  std::ostringstream prologue;
  int framesize = -(frame->current_stack_ptr + reg_manager->WordSize());
  prologue << ".set " << frame->label->Name() << "_framesize, " << framesize << "\n";
  prologue << frame->label->Name() << ":\n";
  prologue << "subq $" << framesize << ",%rsp\n";
  std::ostringstream epilogue;
  epilogue << "addq $" << framesize << ",%rsp\n";
  epilogue << "retq\n";
  return new assem::Proc(
    prologue.str(), body, epilogue.str()
  );
}

/* TODO: Put your lab5 code here */
} // namespace frame