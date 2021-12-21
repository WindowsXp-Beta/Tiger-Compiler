#include "tiger/liveness/flowgraph.h"

extern frame::RegManager * reg_manager;
namespace fg {

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */
  temp::Map * co=temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name());
  tab::Table<assem::Instr, FNode> instr_map;
  FNodePtr pre=nullptr;
  FNodePtr cur=nullptr;
   for (auto p = instr_list_->GetList().begin(); p!=instr_list_->GetList().end(); p++) {
    cur = flowgraph_->NewNode((*p));
    instr_map.Enter((*p),cur);
    if (pre) {
      flowgraph_->AddEdge(pre, cur);
    }
    // (*p)->Print(stderr,co);
    assert(*p);
    if (typeid(*(*p)) ==typeid(assem::LabelInstr)) {
     label_map_->Enter(((assem::LabelInstr *) (*p))->label_,cur);
    }
    //存标号到点的映射，下一步把所有jump边都加入
    if (typeid(*(*p)) == typeid(assem::OperInstr) && ((assem::OperInstr *) (*p))->assem_.find("jmp") != -1) {
      pre = nullptr;
    } else {
      pre = cur;
    }
  }

  for (auto p = instr_list_->GetList().begin(); p!=instr_list_->GetList().end(); p++) {
    if (typeid(*(*p)) == typeid(assem::OperInstr) && ((assem::OperInstr *) (*p))->jumps_) {
      for (auto ptr = ((assem::OperInstr *) (*p))->jumps_->labels_->begin(); ptr!=((assem::OperInstr *) (*p))->jumps_->labels_->end(); ptr++) {
        
        FNodePtr jmpNode = instr_map.Look((*p));
        FNodePtr labelNode = label_map_->Look((*ptr));
        if (!labelNode){
          printf("%s\n",(*ptr)->Name().c_str());
          printf("addr:%p\n",(*ptr));
          (*p)->Print(stderr,co);
           label_map_->Dump([](temp::Label *t,  FNode *r) {
    fprintf(stderr, "label %s  \n", t->Name().c_str());
    printf("addr:%p\n",t);
  });
        }
        flowgraph_->AddEdge(jmpNode, labelNode);
      }
    }
  }
  
}

} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
  return tmp;
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
  if (!dst_) return tmp;
  for (auto ptr=dst_->GetList().begin();ptr!=dst_->GetList().end();ptr++)
  {
    if ((*ptr)!= reg_manager->StackPointer())
    {
      tmp->Append((*ptr));
    }
  }
  return tmp;
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
  if (!dst_) return tmp;
  for (auto ptr=dst_->GetList().begin();ptr!=dst_->GetList().end();ptr++)
  {
    if ((*ptr)!= reg_manager->StackPointer())
    {
      tmp->Append((*ptr));
    }
  }
  return tmp;
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
 return tmp;
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
  if (!src_) return tmp;
  for (auto ptr=src_->GetList().begin();ptr!=src_->GetList().end();ptr++)
  {
    if ((*ptr)!= reg_manager->StackPointer())
    {
      tmp->Append((*ptr));
    }
  }
  return tmp;
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  temp::TempList * tmp=new temp::TempList();
  if (!src_) return tmp;
  for (auto ptr=src_->GetList().begin();ptr!=src_->GetList().end();ptr++)
  {
    if ((*ptr)!= reg_manager->StackPointer())
    {
      tmp->Append((*ptr));
    }
  }
  return tmp;
}
} // namespace assem
