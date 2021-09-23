#include "tiger/frame/frame.h"

namespace frame {

class InFrameAccess : public Access {
public:
  int offset;

  explicit InFrameAccess(int offset) : offset(offset) {}

  tree::Exp *ToExp(tree::Exp *frame_ptr) const override {
    /* TODO: Put your lab5 code here */
  }
};

class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : reg(reg) {}

  tree::Exp *ToExp(tree::Exp *framePtr) const override {
    /* TODO: Put your lab5 code here */
  }
};

class X64Frame : public Frame {
/* TODO: Put your lab5 code here */
};

frame::Frame *NewFrame(temp::Label *name, std::list<bool> formals,
                       frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

tree::Exp *ExternalCall(std::string_view s, tree::ExpList *args) {
  /* TODO: Put your lab5 code here */
}

/**
 * Moving incoming formal parameters, the saving and restoring of callee-save
 * Registers
 * @param frame curruent frame
 * @param stm statements
 * @return statements with saving, restoring and view shift
 */
tree::Stm *ProcEntryExit1(frame::Frame *frame, tree::Stm *stm,
                          RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

/**
 * Appends a “sink” instruction to the function body to tell the register
 * allocator that certain registers are live at procedure exit
 * @param body function body
 * @return instructions with sink instruction
 */
assem::InstrList *ProcEntryExit2(assem::InstrList *body,
                                 RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

/**
 * The procedure entry/exit sequences
 * @param frame the frame of current func
 * @param body current function body
 * @return whole instruction list with prolog_ end epilog_
 */
assem::Proc *ProcEntryExit3(frame::Frame *frame, assem::InstrList *body) {
  /* TODO: Put your lab5 code here */
}

void Frags::PushBack(Frag *frag) { frags_.emplace_back(frag); }
std::list<std::unique_ptr<Frag>> Frags::TransferFrags() {
  return std::move(frags_);
}

temp::TempList *RegManager::Registers() {
  /* TODO: Put your lab5 code here */
}

temp::TempList *RegManager::ArgRegs() {
  /* TODO: Put your lab5 code here */
}

temp::TempList *RegManager::CallerSaves() {
  /* TODO: Put your lab5 code here */
}

temp::TempList *RegManager::CalleeSaves() {
  /* TODO: Put your lab5 code here */
}

temp::TempList *RegManager::ReturnSink() {
  /* TODO: Put your lab5 code here */
}

} // namespace frame