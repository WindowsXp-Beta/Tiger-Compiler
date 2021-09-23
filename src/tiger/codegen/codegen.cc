#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>

namespace {

constexpr int maxlen = 1024;

/* TODO: Put your lab5 code here */
} // namespace

namespace cg {

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {

void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                   frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                     frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                    frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                     frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                    frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs,
                   frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                            frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                          frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                           frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                           frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                           frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                            frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs,
                           frame::RegManager *reg_manager) {
  /* TODO: Put your lab5 code here */
}

} // namespace tree