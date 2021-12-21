#include "tiger/liveness/liveness.h"
#include <set>

extern frame::RegManager *reg_manager;

namespace live {

bool MoveList::Contain(INodePtr src, INodePtr dst) {
  return std::any_of(move_list_.cbegin(), move_list_.cend(),
                     [src, dst](std::pair<INodePtr, INodePtr> move) {
                       return move.first == src && move.second == dst;
                     });
}

void MoveList::Delete(INodePtr src, INodePtr dst) {
  assert(src && dst);
  auto move_it = move_list_.begin();
  for (; move_it != move_list_.end(); move_it++) {
    if (move_it->first == src && move_it->second == dst) {
      break;
    }
  }
  move_list_.erase(move_it);
}

MoveList *MoveList::Union(MoveList *list) {
  auto *res = new MoveList();
  for (auto move: move_list_)
  {
    res->move_list_.push_back(move);
  }
  if (list)
  {
    for (auto move : list->GetList()) {
    if (!Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  
  auto *res = new MoveList();
  if (!list) return res;
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}
bool contains(temp::TempList *list, temp::Temp *temp) {
  if (!list) return false;
  for (auto p = list->GetList().begin(); p!=list->GetList().end(); p++) {
    if ((*p) == temp) {
      return true;
    }
  }
  return false;
}

temp::TempList *Union(temp::TempList *lhs, temp::TempList *rhs) {
  temp::TempList *result = new temp::TempList();
  if (lhs)
  {for (auto p = lhs->GetList().begin(); p!=lhs->GetList().end(); p++) {
    if (!contains(result, (*p))) {
      result->Append((*p));
    }
  }
  }
  if (rhs)
  {
  for (auto p = rhs->GetList().begin(); p!=rhs->GetList().end(); p++) {
    if (!contains(result, (*p))) {
      result->Append((*p));
    }
  }
  }

  return result;
}

temp::TempList *Subtract(temp::TempList *lhs, temp::TempList *rhs) {
  temp::TempList *result = new temp::TempList();
  if (lhs)
  {
    for (auto p = lhs->GetList().begin(); p!=lhs->GetList().end(); p++) {
    if (!contains(rhs, (*p))) {
      result->Append((*p));
    }
  }
  }
  return result;
}

bool equal(temp::TempList *lhs, temp::TempList *rhs) {
  std::set<int> setlhs, setrhs;
  if (!lhs||!rhs)
  {
    return lhs==rhs;
  }
  for (auto p = lhs->GetList().begin(); p!=lhs->GetList().end(); p++) {
    setlhs.insert((*p)->Int());
  }

   for (auto p = rhs->GetList().begin(); p!=rhs->GetList().end(); p++) {
    setrhs.insert((*p)->Int());
  }

  return setlhs == setrhs;
}


void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */
  //
  temp::Map * co=temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name());
  //
   for (auto p = flowgraph_->Nodes()->GetList().begin(); p!=flowgraph_->Nodes()->GetList().end(); p++) {
      in_->Enter((*p),nullptr);
      out_ ->Enter((*p),nullptr);
    }

  while (true) {
    bool flag=false;
    for (auto p = flowgraph_->Nodes()->GetList().begin(); p!=flowgraph_->Nodes()->GetList().end(); p++) {
      temp::TempList *defs = (*p)->NodeInfo()->Def();
      temp::TempList *uses = (*p)->NodeInfo()->Use();
      temp::TempList * old_in=in_->Look((*p));
      temp::TempList * new_in=Union(uses,Subtract(out_->Look((*p)),defs));
      if (!equal(old_in,new_in))
      {
        flag=true;
      }
      in_->Set((*p),new_in);
      temp::TempList * old_out=out_->Look((*p));
      temp::TempList * res=nullptr;
      for (auto itr = (*p)->Succ()->GetList().begin(); itr!=(*p)->Succ()->GetList().end(); itr++) {
        res = Union(res, in_->Look((*itr)));
      }
      if (!equal(old_out,res))
      {
        flag=true;
      }
      out_->Set((*p),res);
    }
    // in_->Dump([co](graph::Node<assem::Instr> * is,temp::TempList * list){
    //   is->NodeInfo()->Print(stderr,co);
    //   if (list)
    //   {
    //     printf("in_ variable:");
    //     for (auto tmp:list->GetList()){
    //       printf("%d ",tmp->Int());
    //     }
    //     printf("\n");
    //   }
    // });
    //   out_->Dump([co](graph::Node<assem::Instr> * is,temp::TempList * list){
    //  is->NodeInfo()->Print(stderr,co);
    //   if (list)
    //   {
    //     printf("out_ variable:");
    //     for (auto tmp:list->GetList()){
    //       printf("%d ",tmp->Int());
    //     }
    //     printf("\n");
    //   }
    // });
    if (!flag) {
      break;
    }
  }
}


void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */
  //加入机器寄存器
  for (temp::Temp *temp1 : reg_manager->Registers()->GetList()) {
    for (temp::Temp *temp2 : reg_manager->Registers()->GetList()) {
      graph::Node<temp::Temp> *temp1Node = GetNode(temp1);
      graph::Node<temp::Temp> *temp2Node = GetNode(temp2);
      if (temp1Node != temp2Node) {
        live_graph_.interf_graph->AddEdge(temp1Node,temp2Node);
        live_graph_.interf_graph->AddEdge(temp2Node,temp1Node);
        
      }
    }
  }
  //
  for (auto p = flowgraph_->Nodes()->GetList().begin(); p!=flowgraph_->Nodes()->GetList().end(); p++) {
    temp::TempList *defs = (*p)->NodeInfo()->Def();
    temp::TempList *uses = (*p)->NodeInfo()->Use();
    if (typeid(*((*p)->NodeInfo()))==typeid(assem::MoveInstr) && defs && uses) {
      graph::Node<temp::Temp> *srcNode = GetNode(uses->GetList().front());
      graph::Node<temp::Temp> *dstNode = GetNode( defs->GetList().front());
      live_graph_.moves->Append(srcNode,dstNode);
      for (auto q = out_->Look((*p))->GetList().begin(); q!=out_->Look((*p))->GetList().end();q++) {
        if ((*q) == uses->GetList().front()) {
          continue;
        }
        graph::Node<temp::Temp> *outNode = GetNode((*q));
        if (dstNode != outNode) {
         live_graph_.interf_graph->AddEdge(dstNode, outNode);
         live_graph_.interf_graph->AddEdge(outNode, dstNode);
        }
      }
    } else {
      if (!defs) continue;
      for (auto q=defs->GetList().begin(); q!=defs->GetList().end(); q++) {
        for (auto r = out_->Look((*p))->GetList().begin(); r!=out_->Look((*p))->GetList().end(); r++) {
          graph::Node<temp::Temp> *dstNode = GetNode(*(q));
          graph::Node<temp::Temp> *outNode = GetNode(*(r));
          if (dstNode != outNode) {
           live_graph_.interf_graph->AddEdge(dstNode, outNode);
          live_graph_.interf_graph->AddEdge(outNode, dstNode);
          }
        }
      }
    }
  }
  //
  for (auto p = flowgraph_->Nodes()->GetList().begin(); p!=flowgraph_->Nodes()->GetList().end(); p++) {
    temp::TempList *defs = (*p)->NodeInfo()->Def();
    temp::TempList *uses = (*p)->NodeInfo()->Use();
    for (auto q = defs->GetList().begin(); q!=defs->GetList().end(); q++) {
      (*(live_graph_.priority))[(*q)]++;
    }
    for (auto q = uses->GetList().begin(); q!= uses->GetList().end(); q++) {
      (*(live_graph_.priority))[(*q)]++;
    }
  }
  for (auto p = live_graph_.interf_graph->Nodes()->GetList().begin(); p!=live_graph_.interf_graph->Nodes()->GetList().end(); p++) {
    (*(live_graph_.priority))[(*p)->NodeInfo()]/=(*p)->Degree();
  }
  //


}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live