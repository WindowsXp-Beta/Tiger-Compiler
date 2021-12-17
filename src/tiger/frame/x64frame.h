//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {
class X64RegManager : public RegManager {
  /* TODO: Put your lab5 code here */
public:
  X64RegManager();

  temp::TempList *Registers() override;

  temp::TempList *ArgRegs() override;

  temp::TempList *CallerSaves() override;

  temp::TempList *CalleeSaves() override;

  temp::TempList *ReturnSink() override;

  temp::TempList *AllWithoutRsp() override;

  int WordSize() override;

  temp::Temp *FramePointer() override;

  temp::Temp *StackPointer() override;

  temp::Temp *ReturnValue() override;
};

class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */
public:
  X64Frame(temp::Label *name, std::list<bool> *formals);

  ~X64Frame() {}

  Access *AllocLocal(bool isEscape) override;
};

class InFrameAccess : public Access {
public:
  int offset;

  explicit InFrameAccess(int offset) : offset(offset) {}
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override;
};

class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : reg(reg) {}
  /* TODO: Put your lab5 code here */
  tree::Exp *ToExp(tree::Exp *framePtr) const override;
};

} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
