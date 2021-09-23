#ifndef TIGER_CODEGEN_CODEGEN_H_
#define TIGER_CODEGEN_CODEGEN_H_

#include "tiger/canon/canon.h"
#include "tiger/codegen/assem.h"
#include "tiger/frame/frame.h"
#include "tiger/translate/tree.h"

// Forward Declarations
namespace frame {
class RegManager;
class Frame;
} // namespace frame

namespace assem {
class Instr;
class InstrList;
} // namespace assem

namespace canon {
class Traces;
} // namespace canon

namespace cg {

class AssemInstr {
public:
  AssemInstr() = delete;
  AssemInstr(nullptr_t) = delete;
  explicit AssemInstr(assem::InstrList *instr_list) : instr_list_(instr_list) {}

  void Print(FILE *out, temp::Map *map) const;

  [[nodiscard]] assem::InstrList *GetInstrList() const { return instr_list_; }

private:
  assem::InstrList *instr_list_;
};

class CodeGen {
public:
  CodeGen(frame::Frame *frame, std::unique_ptr<canon::Traces> traces,
          frame::RegManager *reg_manager)
      : frame_(frame), traces_(std::move(traces)), reg_manager_(reg_manager) {}

  void Codegen();
  std::unique_ptr<AssemInstr> TransferAssemInstr() {
    return std::move(assem_instr_);
  }

private:
  frame::Frame *frame_;
  std::string fs_; // Frame size label_
  std::unique_ptr<canon::Traces> traces_;
  std::unique_ptr<AssemInstr> assem_instr_;
  frame::RegManager *reg_manager_;
};

} // namespace cg
#endif