#include "tiger/output/output.h"

#include <cstdio>

#include "tiger/output/logger.h"

namespace output {
void AssemGen::GenAssem() {
  frame::Frag::OutputPhase phase;

  // Output proc
  phase = frame::Frag::Proc;
  fprintf(out_, ".text\n");
  for (auto &&frag : frags_)
    frag->OutputAssem(out_, reg_manager_.get(), phase);

  // Output string
  phase = frame::Frag::String;
  fprintf(out_, ".section .rodata\n");
  for (auto &&frag : frags_)
    frag->OutputAssem(out_, reg_manager_.get(), phase);
}
} // namespace output

namespace frame {

void ProcFrag::OutputAssem(FILE *out, frame::RegManager *reg_manager,
                           OutputPhase phase) const {
  std::unique_ptr<canon::Traces> traces;
  std::unique_ptr<cg::AssemInstr> assem_instr;
  std::unique_ptr<ra::Result> allocation;

  // When generating proc fragment, do not output string assembly
  if (phase != Proc)
    return;

  TigerLog("-------====IR tree=====-----\n");
  TigerLog(body_);

  {
    // Canonicalize
    TigerLog("-------====Canonicalize=====-----\n");
    canon::Canon canon(body_);

    // Linearize to generate canonical trees
    TigerLog("-------====Linearlize=====-----\n");
    tree::StmList *stm_linearized = canon.Linearize();
    TigerLog(stm_linearized);

    // Group list into basic blocks
    TigerLog("------====Basic block_=====-------\n");
    canon::StmListList *stm_lists = canon.BasicBlocks();
    TigerLog(stm_lists);

    // Order basic blocks into traces_
    TigerLog("-------====Trace=====-----\n");
    tree::StmList *stm_traces = canon.TraceSchedule();
    TigerLog(stm_traces);

    traces = canon.TransferTraces();
  }

  {
    // Lab 5 & Lab 6: code generation
    TigerLog("-------====Code generate=====-----\n");
    cg::CodeGen code_gen(frame_, std::move(traces), reg_manager);
    code_gen.Codegen();
    assem_instr = code_gen.TransferAssemInstr();
    TigerLog(assem_instr.get(),
             temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name()));
  }

  {
    // Lab 6: register allocation
    TigerLog("----====Register allocate====-----\n");
    ra::RegAllocator reg_allocator(frame_, std::move(assem_instr), reg_manager);
    reg_allocator.RegAlloc();
    allocation = reg_allocator.TransferResult();
  }

  TigerLog("-------====Output assembly for %s=====-----\n",
           frame_->name_->Name().data());

  assem::Proc *proc = frame::ProcEntryExit3(frame_, allocation->il_);

  std::string proc_name = frame_->GetLabel();

  fprintf(out, ".globl %s\n", proc_name.data());
  fprintf(out, ".type %s, @function\n", proc_name.data());
  // prologue
  fprintf(out, "%s", proc->prolog_.data());
  // body
  proc->body_->Print(
      out, temp::Map::LayerMap(reg_manager->temp_map_, allocation->coloring_));
  // epilog_
  fprintf(out, "%s", proc->epilog_.data());
  fprintf(out, ".size %s, .-%s\n", proc_name.data(), proc_name.data());
}

void StringFrag::OutputAssem(FILE *out, frame::RegManager *reg_manager,
                             OutputPhase phase) const {
  // When generating string fragment, do not output proc assembly
  if (phase != String)
    return;

  fprintf(out, "%s:\n", label_->Name().data());
  int length = static_cast<int>(str_.size());
  // It may contain zeros in the middle of string. To keep this work, we need
  // to print all the charactors instead of using fprintf(str)
  fprintf(out, ".long %d\n", length);
  fprintf(out, ".string \"");
  for (int i = 0; i < length; i++) {
    if (str_[i] == '\n') {
      fprintf(out, "\\n");
    } else if (str_[i] == '\t') {
      fprintf(out, "\\t");
    } else if (str_[i] == '\"') {
      fprintf(out, "\\\"");
    } else {
      fprintf(out, "%c", str_[i]);
    }
  }
  fprintf(out, "\"\n");
}
} // namespace frame
