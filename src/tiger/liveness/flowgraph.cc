#include "tiger/liveness/flowgraph.h"
#include <map>

namespace fg {

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */
  flowgraph_ = new graph::Graph<assem::Instr>;
  std::map<temp::Label *, FNodePtr> label2fnode;
  // record labelInstr to its FNode for further Jump
  std::list<std::pair<assem::Instr *, FNodePtr>> jump2fnode;
  FNodePtr last = nullptr;
  for (auto instr : instr_list_->GetList()) {
    auto cur = flowgraph_->NewNode(instr);
    if (typeid(*instr) == typeid(assem::LabelInstr)) {
      label2fnode[static_cast<assem::LabelInstr *>(instr)->label_] = cur;
    } else if (typeid(*instr) == typeid(assem::OperInstr) && static_cast<assem::OperInstr *>(instr)->jumps_) {
      // Jump OR Cjump
      last = nullptr;
      jump2fnode.emplace_back(instr, cur);
    } else {
      last = cur;
    }
    if (last)
      flowgraph_->AddEdge(last, cur);
  }

  for (auto instr_fnode : jump2fnode) {
    auto jump_instr = static_cast<assem::OperInstr *>(instr_fnode.first);
    for (auto label : *jump_instr->jumps_->labels_) {
      flowgraph_->AddEdge(instr_fnode.second, label2fnode[label]);
    }
  }
}

} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return dst_;
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return dst_;
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return dst_;
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return src_;
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return src_;
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return src_;
}
} // namespace assem
