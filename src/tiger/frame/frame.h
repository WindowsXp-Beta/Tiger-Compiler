#ifndef TIGER_FRAME_FRAME_H_
#define TIGER_FRAME_FRAME_H_

#include <list>
#include <memory>
#include <string>

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/translate/tree.h"

namespace frame {

class RegManager {
public:
  RegManager()
      : temp_map_(temp::Map::Empty()),
        regs_{temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp(), temp::TempFactory::NewTemp(),
              temp::TempFactory::NewTemp()} {
    // Note: no frame pointer in tiger compiler
    std::array<std::string_view, REG_COUNT> reg_name{
        "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%rbp", "%rsp",
        "%r8",  "%r9",  "%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
    int reg = RAX;
    for (auto &name : reg_name) {
      temp_map_->Enter(regs_[reg], new std::string(name));
      reg++;
    }
  }

  enum Reg : unsigned long {
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    FP,
    REG_COUNT,
    SP = RSP, // use RSP as SP
    RV = RAX, // use RAX as RV
  };
  temp::Temp *GetRegister(Reg reg) { return regs_[reg]; }

  /**
   * Get general-purpose registers except RSI
   * NOTE: returned temp list should be in the order of calling convention
   * @return general-purpose registers
   */
  [[nodiscard]] temp::TempList *Registers();

  /**
   * Get registers which can be used to hold arguments
   * NOTE: returned temp list must be in the order of calling convention
   * @return argument registers
   */
  [[nodiscard]] temp::TempList *ArgRegs();

  /**
   * Get caller-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return caller-saved registers
   */
  [[nodiscard]] temp::TempList *CallerSaves();

  /**
   * Get callee-saved registers
   * NOTE: returned registers must be in the order of calling convention
   * @return callee-saved registers
   */
  [[nodiscard]] temp::TempList *CalleeSaves();

  /**
   * Get return-sink registers
   * @return return-sink registers
   */
  [[nodiscard]] temp::TempList *ReturnSink();

  constexpr static int word_size_ = 8;
  temp::Map *temp_map_;

private:
  std::array<temp::Temp *, REG_COUNT> regs_;
};

class Access {
public:
  virtual tree::Exp *ToExp(tree::Exp *framePtr) const = 0;

  virtual ~Access() = default;
};

class Frame {
  /* TODO: Put your lab5 code here */
};

/**
 * Fragments
 */

class Frag {
public:
  virtual ~Frag() = default;

  enum OutputPhase {
    Proc,
    String,
  };

  /**
   *Generate assembly for main program
   * @param out FILE object for output assembly file
   * @param reg_manager register manager
   */
  virtual void OutputAssem(FILE *out, frame::RegManager *reg_manager,
                           OutputPhase phase) const = 0;
};

class StringFrag : public Frag {
public:
  temp::Label *label_;
  std::string str_;

  StringFrag(temp::Label *label, std::string str)
      : label_(label), str_(std::move(str)) {}

  void OutputAssem(FILE *out, frame::RegManager *reg_manager,
                   OutputPhase phase) const override;
};

class ProcFrag : public Frag {
public:
  tree::Stm *body_;
  Frame *frame_;

  ProcFrag(tree::Stm *body, Frame *frame) : body_(body), frame_(frame) {}

  void OutputAssem(FILE *out, frame::RegManager *reg_manager,
                   OutputPhase phase) const override;
};

class Frags {
public:
  Frags() = default;
  void PushBack(Frag *frag);
  std::list<std::unique_ptr<Frag>> TransferFrags();

private:
  std::list<std::unique_ptr<Frag>> frags_;
};

frame::Frame *NewFrame(temp::Label *name, std::list<bool> formals,
                       frame::RegManager *reg_manager);

tree::Exp *ExternalCall(std::string_view s,
                        tree::ExpList *args); // call an external function

tree::Stm *ProcEntryExit1(frame::Frame *frame, tree::Stm *stm,
                          RegManager *reg_manager);
assem::InstrList *ProcEntryExit2(assem::InstrList *body,
                                 RegManager *reg_manager);
assem::Proc *ProcEntryExit3(frame::Frame *frame, assem::InstrList *body);

} // namespace frame

#endif