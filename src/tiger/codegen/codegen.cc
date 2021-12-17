#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>

extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  auto *list = new assem::InstrList();
  for (auto stm : traces_->GetStmList()->GetList()) {
    stm->Munch(*list, fs_);
  }
  assem_instr_ = std::make_unique<AssemInstr>(frame::ProcEntryExit2(list));
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList())
    instr->Print(out, map);
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {
/* TODO: Put your lab5 code here */

/**
 * TODO:refactor
 * 1. make a function to handle all the memory access(Imm(r), Imm, (r)) or move statement.
 * 2. implenment maximal munch in BinOpExp.
 * 3. address all the address format((r1, r2), (, r1), etc)
 */
void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  left_->Munch(instr_list, fs);
  right_->Munch(instr_list, fs);
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(
    new assem::LabelInstr(
      label_->Name(),
      label_
    )
  );
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(
    new assem::OperInstr(
      "jmp `j0",
      nullptr, nullptr,
      new assem::Targets(jumps_)
    )
  );
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  instr_list.Append(
    new assem::OperInstr(
      "cmpq `s0,`s1",
      nullptr,
      new temp::TempList{left_->Munch(instr_list, fs), right_->Munch(instr_list, fs)},
      nullptr
    )
  );
  std::ostringstream assem;
  switch (op_) {
    case EQ_OP:
      assem << "je ";
      break;
    case NE_OP:
      assem << "jne ";
      break;
    case LT_OP:
      assem << "jg ";
      break;
    case GT_OP:
      assem << "jl ";
      break;
    case LE_OP:
      assem << "jge ";
      break;
    case GE_OP:
      assem << "jle ";
      break;
  }
  assem << "`j0";
  instr_list.Append(
    new assem::OperInstr(
      assem.str(),
      nullptr, nullptr,
      new assem::Targets(new std::vector<temp::Label *>{true_label_, false_label_})
    )
  );
}

/**
 * TODO: This funciton really needs to be refactored!!!
 * In fact, I don't suggest to do maximal munch in MoveStm...
 * You have no idea how many bugs you will encounter...
 */
void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  // movq s, Mem
  if (typeid(*dst_) == typeid(MemExp)) {
    auto dst2mem = static_cast<MemExp *>(dst_);
    if (typeid(*dst2mem->exp_) == typeid(BinopExp)) {
      auto mem2bin = static_cast<BinopExp *>(dst2mem->exp_);
      if (mem2bin->op_ == PLUS_OP) {
        if (typeid(*mem2bin->left_) == typeid(ConstExp)) {
          // movq s, Imm(r)
          // be cartful, it doesn't have destination registers
          auto right_temp = mem2bin->right_->Munch(instr_list, fs);
          auto left2cst = static_cast<ConstExp *>(mem2bin->left_);
          std::ostringstream assem;
          if (typeid(*src_) == typeid(ConstExp)) {
            // movq $Ism, Imm(r)
            auto src2cst = static_cast<ConstExp *>(src_);
            if (right_temp == reg_manager->FramePointer()) {
              assem << "movq $" << src2cst->consti_ << ",(" << fs << "_framesize" << left2cst->consti_ << ")(`s0)";
              right_temp = reg_manager->StackPointer();
            } else {
              assem << "movq $" << src2cst->consti_ << "," << left2cst->consti_ << "(`s0)";
            }
            instr_list.Append(
              new assem::OperInstr(
                assem.str(),
                nullptr,
                new temp::TempList(right_temp),
                nullptr
              )
            );
          } else {
            // movq s, Imm(r)
            if (right_temp == reg_manager->FramePointer()) {
              assem << "movq `s0,(" << fs << "_framesize" << left2cst->consti_ << ")(`s1)";
              right_temp = reg_manager->StackPointer();
            } else {
              assem << "movq `s0," << left2cst->consti_ << "(`s1)";
            }
            instr_list.Append(
              new assem::OperInstr(
                assem.str(),
                nullptr,
                new temp::TempList{src_->Munch(instr_list, fs), right_temp},
                nullptr
              )
            );
          }
        } else if (typeid(*mem2bin->right_) == typeid(ConstExp)) {
          // movq s, Imm(r)
          auto left_temp = mem2bin->left_->Munch(instr_list, fs);
          auto right2cst = static_cast<ConstExp *>(mem2bin->right_);
          std::ostringstream assem;
          if (typeid(*src_) == typeid(ConstExp)) {
            // movq $Ism, Imm(r)
            auto src2cst = static_cast<ConstExp *>(src_);
            if (left_temp == reg_manager->FramePointer()) {
              assem << "movq $" << src2cst->consti_ << ",(" << fs << "_framesize" << right2cst->consti_ << ")(`s0)";
              left_temp = reg_manager->StackPointer();
            } else {
              assem << "movq $" << src2cst->consti_ << "," << right2cst->consti_ << "(`s0)";
            }
            instr_list.Append(
              new assem::OperInstr(
                assem.str(),
                nullptr,
                new temp::TempList(left_temp),
                nullptr
              )
            );
          } else {
            // movq s, Imm(r)
            if (left_temp == reg_manager->FramePointer()) {
              assem << "movq `s0,(" << fs << "_framesize" << right2cst->consti_ << ")(`s1)";
              left_temp = reg_manager->StackPointer();
            } else {
              assem << "movq `s0," << right2cst->consti_ << "(`s1)";
            }
            instr_list.Append(
              new assem::OperInstr(
                assem.str(),
                nullptr,
                new temp::TempList{src_->Munch(instr_list, fs), left_temp},
                nullptr
              )
            );
          }
        } else {
          instr_list.Append(
            new assem::OperInstr(
              "movq `s0,(`s1)",
              nullptr,
              new temp::TempList{src_->Munch(instr_list, fs), dst2mem->exp_->Munch(instr_list, fs)},
              nullptr
            )
          );
        }
      }
    } else if (typeid(*dst2mem->exp_) == typeid(ConstExp)) {
      // movq s, Ism
      auto dst2cst = static_cast<ConstExp *>(dst2mem->exp_);
      if (typeid(*src_) == typeid(ConstExp)) {
        // movq $Ism, Ism
        auto src2cst = static_cast<ConstExp *>(src_);
        std::stringstream assem;
        assem << "movq $" << src2cst->consti_ << "," << dst2cst->consti_;
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            nullptr, nullptr, nullptr
          )
        );
      } else {
        // movq s, Ism
        std::stringstream assem;
        assem << "movq `s0," << dst2cst->consti_;
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            nullptr,
            new temp::TempList(src_->Munch(instr_list, fs)),
            nullptr
          )
        );
      }
    } else {
      // movq s, (r)
      if (typeid(*src_) == typeid(ConstExp)) {
        // movq $Ism, (r)
        std::stringstream assem;
        auto src2cst = static_cast<ConstExp *>(src_);
        assem << "movq $" << src2cst->consti_ << "(`s0)";
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            nullptr,
            new temp::TempList(dst2mem->Munch(instr_list, fs)),
            nullptr
          )
        );
      } else {
        // movq s,(r)
        instr_list.Append(
          new assem::OperInstr(
            "movq `s0,(`s1)",
            nullptr,
            new temp::TempList{src_->Munch(instr_list, fs), dst2mem->exp_->Munch(instr_list, fs)},
            nullptr
          )
        );
      }
    }
  } else if(typeid(*src_) == typeid(MemExp)) {
    auto src2mem = static_cast<MemExp *>(src_);
    if (typeid(*src2mem->exp_) == typeid(BinopExp)) {
      auto mem2bin = static_cast<BinopExp *>(src2mem->exp_);
      if (mem2bin->op_ == PLUS_OP) {
        if (typeid(*mem2bin->left_) == typeid(ConstExp)) {
          // movq Imm(r), d
          auto right_temp = mem2bin->right_->Munch(instr_list, fs);
          auto left2cst = static_cast<ConstExp *>(mem2bin->left_);
          std::ostringstream assem;
          if (right_temp == reg_manager->FramePointer()) {
            if (left2cst->consti_ < 0) {
              assem << "movq (" << fs << "_framesize" << left2cst->consti_ << ")(`s0)," << "`d0";
            } else {
              assem << "movq (" << fs << "_framesize+" << left2cst->consti_ << ")(`s0)," << "`d0";
            }
            right_temp == reg_manager->StackPointer();
          } else {
            assem << "movq " << left2cst->consti_ << "(`s0)," << "`d0";
          }
          instr_list.Append(
            new assem::OperInstr(
              assem.str(),
              new temp::TempList(dst_->Munch(instr_list, fs)),
              new temp::TempList(right_temp),
              nullptr
            )
          );
        } else if (typeid(*mem2bin->right_) == typeid(ConstExp)) {
          // movq Imm(r), d
          auto left_temp = mem2bin->left_->Munch(instr_list, fs);
          auto right2cst = static_cast<ConstExp *>(mem2bin->right_);
          std::ostringstream assem;
          if (left_temp == reg_manager->FramePointer()) {
            if (right2cst->consti_ < 0) {
              assem << "movq (" << fs << "_framesize" << right2cst->consti_ << ")(`s0)," << "`d0";
            } else {
              assem << "movq (" << fs << "_framesize+" << right2cst->consti_ << ")(`s0)," << "`d0";
            }
            left_temp = reg_manager->StackPointer();
          } else {
            assem << "movq " << right2cst->consti_ << "(`s0)," << "`d0";
          }
          instr_list.Append(
            new assem::OperInstr(
              assem.str(),
              new temp::TempList(dst_->Munch(instr_list, fs)),
              new temp::TempList(left_temp),
              nullptr
            )
          );
        } else {
          // movq (r), d 
          instr_list.Append(
            new assem::OperInstr(
              "movq (`s0),`d0",
              new temp::TempList(dst_->Munch(instr_list, fs)),
              new temp::TempList(src2mem->exp_->Munch(instr_list, fs)),
              nullptr
            )
          );
        }
      }
    } else if (typeid(*src2mem->exp_) == typeid(ConstExp)) {
      // movq Ism, d
      auto src2cst = static_cast<ConstExp *>(src2mem->exp_);
      std::ostringstream assem;
      assem << "movq " << src2cst->consti_ << ",`d0";
      instr_list.Append(
        new assem::OperInstr(
          assem.str(),
          new temp::TempList(dst_->Munch(instr_list, fs)),
          nullptr, nullptr
        )
      );
    } else {
      // movq (r), d 
      instr_list.Append(
        new assem::OperInstr(
          "movq (`s0),`d0",
          new temp::TempList(dst_->Munch(instr_list, fs)),
          new temp::TempList(src2mem->exp_->Munch(instr_list, fs)),
          nullptr
        )
      );
    }
  } else if (typeid(*src_) == typeid(ConstExp)) {
    // movq $Ism, d
    auto src2cst = static_cast<ConstExp *>(src_);
    std::ostringstream assem;
    assem << "movq $" << src2cst->consti_ << ",`d0";
    instr_list.Append(
      new assem::OperInstr(
        assem.str(),
        new temp::TempList(dst_->Munch(instr_list, fs)),
        nullptr, nullptr
      )
    );
  } else {
    // movq s, d
    instr_list.Append(
      new assem::MoveInstr(
        "movq `s0,`d0",
        new temp::TempList(dst_->Munch(instr_list, fs)),
        new temp::TempList(src_->Munch(instr_list, fs))
      )
    );
  }
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  exp_->Munch(instr_list, fs);
}

/**
 * Hint:
 * The semantic of tree::BinopExp is different from X86_64's addq, subq
 * since it will not change the original left_'s value.
 * It will return a new temp storing the result.
 */
temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto left_temp = left_->Munch(instr_list, fs);
  auto right_temp = right_->Munch(instr_list, fs);
  auto result_temp = temp::TempFactory::NewTemp();
  switch (op_) {
    case PLUS_OP:{
      /**
       * TODO: find out why canon will change mov(Imm(r),r)
       * into 2 steps
       * 1. addq Imm,r
       * 2. mov(r,r)
       */
      if (left_temp == reg_manager->FramePointer()) {
        std::ostringstream assem;
        assem << "leaq " << fs << "_framesize(%rsp),`d0";
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            new temp::TempList(result_temp),
            nullptr, nullptr
          )
        );
      } else {
        instr_list.Append(
          new assem::MoveInstr(
            "movq `s0,`d0",
            new temp::TempList(result_temp),
            new temp::TempList(left_temp)
          )
        );
      }
      instr_list.Append(
        new assem::OperInstr(
          "addq `s0,`d0",
          new temp::TempList(result_temp),
          new temp::TempList{right_temp, result_temp},
          nullptr
        )
      );
      return result_temp;
    }
    case MINUS_OP:{
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0,`d0",
          new temp::TempList(result_temp),
          new temp::TempList(left_temp)
        )
      );
      instr_list.Append(
        new assem::OperInstr(
          "subq `s0,`d0",
          new temp::TempList(result_temp),
          new temp::TempList{right_temp, result_temp},
          nullptr
        )
      );
      return result_temp;
    }
    case MUL_OP:{
      auto rax = reg_manager->GetRegister(0);
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0,%rax",
          new temp::TempList(rax),
          new temp::TempList(left_temp)
        )
      );
      instr_list.Append(
        new assem::OperInstr(
          "imulq `s0",
          new temp::TempList(rax),
          new temp::TempList{right_temp, rax},
          nullptr
        )
      );
      return rax;
    }
    case DIV_OP:{
      auto rax = reg_manager->GetRegister(0);
      auto rdx = reg_manager->GetRegister(3);
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0,%rax",
          new temp::TempList(rax),
          new temp::TempList(left_temp)
        )
      );
      instr_list.Append(
        new assem::OperInstr(
          "cqto",
          new temp::TempList(rdx),
          nullptr, nullptr
        )
      );
      instr_list.Append(
        new assem::OperInstr(
          "idivq `s0",// [%rdx%rax] / right_temp = %rax......%rdx
          new temp::TempList{rax, rdx},
          new temp::TempList{right_temp, rax, rdx},
          nullptr
        )
      );
      return rax;
    }
  }
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto return_temp = temp::TempFactory::NewTemp();
  if (typeid(*exp_) == typeid(BinopExp)) {
    auto exp2bin = static_cast<BinopExp *>(exp_);
    if (exp2bin->op_ == PLUS_OP) {
      if (typeid(*exp2bin->left_) == typeid(ConstExp)) {
        auto left2cst = static_cast<ConstExp *>(exp2bin->left_);
        auto right_temp = exp2bin->right_->Munch(instr_list, fs);
        // movq Imm(r),d
        std::ostringstream assem;
        if (right_temp == reg_manager->FramePointer()) {
          assem << "movq (" << fs << "_framesize" << left2cst->consti_ << ")(`s0),`d0";
          right_temp = reg_manager->StackPointer();
        } else {
          assem << "movq " << left2cst->consti_ << "(`s0),`d0";
        }
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            new temp::TempList(return_temp),
            new temp::TempList(right_temp),
            nullptr
          )
        );
      } else if (typeid(*exp2bin->right_) == typeid(ConstExp)) {
        auto right2cst = static_cast<ConstExp *>(exp2bin->right_);
        // movq Imm(r),d
        auto left_temp = exp2bin->left_->Munch(instr_list, fs);
        std::ostringstream assem;
        if (left_temp == reg_manager->FramePointer()) {
          assem << "movq (" << fs << "_framesize" << right2cst->consti_ << ")(`s0),`d0";
          left_temp = reg_manager->StackPointer();
        } else {
          assem << "movq " << right2cst->consti_ << "(`s0),`d0";
        }
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            new temp::TempList(return_temp),
            new temp::TempList(left_temp),
            nullptr
          )
        );
      } else {
        // movq (r), d
        instr_list.Append(
          new assem::OperInstr(
            "movq (`s0),`d0",
            new temp::TempList(return_temp),
            new temp::TempList(exp_->Munch(instr_list, fs)),
            nullptr
          )
        );
      }
    }
  } else if (typeid(*exp_) == typeid(ConstExp)) {
    // movq Imm, d
    auto exp2cst = static_cast<ConstExp *>(exp_);
    std::ostringstream assem;
    assem << "movq " << exp2cst->consti_ << ",`d0";
    instr_list.Append(
      new assem::OperInstr(
        assem.str(),
        new temp::TempList(return_temp),
        nullptr, nullptr
      )
    );
  } else {
    // movq (r), d
    instr_list.Append(
      new assem::OperInstr(
        "movq (`s0),`d0",
        new temp::TempList(return_temp),
        new temp::TempList(exp_->Munch(instr_list, fs)),
        nullptr
      )
    );
  }
  return return_temp;
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  return temp_;
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  stm_->Munch(instr_list, fs);
  return exp_->Munch(instr_list, fs);
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto string_address = temp::TempFactory::NewTemp();
  std::ostringstream assem;
  assem << "leaq " << name_->Name() << "(%rip),`d0";
  instr_list.Append(
    new assem::OperInstr(
      assem.str(),
      new temp::TempList(string_address),
      nullptr, nullptr
    )
  );
  return string_address;
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto return_temp = temp::TempFactory::NewTemp();
  std::ostringstream assem;
  assem << "movq $" << consti_ << ",`d0";
  instr_list.Append(
    new assem::OperInstr(
      assem.str(),
      new temp::TempList(return_temp),
      nullptr, nullptr
    )
  );
  return return_temp;
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  size_t args_size = args_->GetList().size();
  if (args_size > 6) {
    std::ostringstream assem;
    assem << "subq $" << (args_size - 6) * reg_manager->WordSize() << ",%rsp";
    instr_list.Append(
      new assem::OperInstr(
        assem.str(),
        nullptr, nullptr, nullptr
      )
    );
  }
  auto calldefs = reg_manager->CallerSaves();
  calldefs->Append(reg_manager->ReturnValue());
  auto temp_list = args_->MunchArgs(instr_list, fs);
  std::ostringstream assem;
  assem << "callq " << static_cast<NameExp *>(fun_)->name_->Name();
  instr_list.Append(
    new assem::OperInstr(
      assem.str(),
      calldefs, temp_list,
      nullptr
    )
  );
  if (args_size > 6) {
    std::ostringstream assem;
    assem << "addq $" << (args_size - 6) * reg_manager->WordSize() << ",%rsp";
    instr_list.Append(
      new assem::OperInstr(
        assem.str(),
        nullptr, nullptr, nullptr
      )
    );
  }
  return reg_manager->ReturnValue();
}

temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  auto temp_list = new temp::TempList();
  size_t exp_list_size = exp_list_.size();
  auto exp_list_it = exp_list_.begin();
  auto arg_temp_list = reg_manager->ArgRegs();
  for (int i = 0; i < exp_list_size; i++, exp_list_it++) {
    auto exp_temp = (*exp_list_it)->Munch(instr_list, fs);
    switch (i) {
      case 0:
        if (exp_temp == reg_manager->FramePointer()) {
          std::ostringstream assem;
          // we cannot use movq because we just want the address itself
          assem << "leaq " << fs << "_framesize(%rsp),`d0";
          instr_list.Append(
            new assem::OperInstr(
              assem.str(),
              new temp::TempList(arg_temp_list->NthTemp(i)),
              nullptr, nullptr
            )
          );
        } else {
          instr_list.Append(
            new assem::MoveInstr(
              "movq `s0,`d0",
              new temp::TempList(arg_temp_list->NthTemp(i)),
              new temp::TempList(exp_temp)
            )
          );
        }
        break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
        instr_list.Append(
          new assem::MoveInstr(
            "movq `s0,`d0",
            new temp::TempList(arg_temp_list->NthTemp(i)),
            new temp::TempList(exp_temp)
          )
        );
        break;
      default: {
        std::ostringstream assem;
        /* <del>We don't modify the %rsp here. We do it at the beginning of the function In ProcEntryExit3</del> */
        // we modify %rsp here!!! You can use gcc to have a check.
        // Actually we don't modify it here, we modify it one-time before and after the callexp.
        // assem << "subq $" << reg_manager->WordSize() << ",`d0";
        // instr_list.Append(
        //   new assem::OperInstr(
        //     assem.str(),
        //     new temp::TempList(reg_manager->StackPointer()),
        //     new temp::TempList(reg_manager->StackPointer()),
        //     nullptr
        //   )
        // );
        assem << "movq `s0," << (i - 6) * reg_manager->WordSize() << "(%rsp)";
        instr_list.Append(
          new assem::OperInstr(
            assem.str(),
            nullptr,
            new temp::TempList(exp_temp),
            nullptr
          )
        );
      } break;
    }
  }
  return temp_list;
}

} // namespace tree
