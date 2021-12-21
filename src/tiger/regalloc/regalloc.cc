#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"
#include <sstream>

extern frame::RegManager *reg_manager;
 std::set<temp::Temp *> noSpillTemp;
namespace ra {
/* TODO: Put your lab6 code here */
bool precolored(temp::Temp *tmp) {
  temp::TempList *regs = reg_manager->Registers();
  for (auto itr = regs->GetList().begin(); itr != regs->GetList().end(); itr++) {
    if ((*itr) == tmp) {
      return true;
    }
  }
  return false;
}

void RegAllocator::AddEdge(graph::Node<temp::Temp> *u, graph::Node<temp::Temp> *v) {
  if (adjSet.find(std::make_pair(u, v)) == adjSet.end() && u != v) {
    adjSet.insert(std::make_pair(u, v));
    adjSet.insert(std::make_pair(v, u));
    if (!precolored(u->NodeInfo())) {
      adjList[u].insert(v);
      degree[u]++;
    }
    if (!precolored(v->NodeInfo())) {
      adjList[v].insert(u);
      degree[v]++;
    }
  }
}

void RegAllocator::Build()
{
  for (auto p = liveness->GetLiveGraph().moves->GetList().begin();
        p!=liveness->GetLiveGraph().moves->GetList().end();
        p++)
  {
    graph::Node<temp::Temp> *srcNode = (*p).first;
    graph::Node<temp::Temp> *dstNode = (*p).second;
    if (moveList.find(srcNode)==moveList.end()) {
      moveList[srcNode] = new live::MoveList();
      moveList[srcNode]->Append(srcNode,dstNode);
    }
    else {
      moveList[srcNode]->Append(srcNode,dstNode);
    }
    if (moveList.find(dstNode)==moveList.end()) {
      moveList[dstNode] = new live::MoveList();
      moveList[dstNode]->Append(srcNode,dstNode);
    }
    else {
      moveList[dstNode]->Append(srcNode,dstNode);
    }
  }
  workListMoves = liveness->GetLiveGraph().moves;
  for (auto p = liveness->GetLiveGraph().interf_graph->Nodes()->GetList().begin();
       p!=liveness->GetLiveGraph().interf_graph->Nodes()->GetList().end();
       p++)
  {
    auto ls = (*p)->Adj()->GetList();
    for (auto q = ls.begin(); q!= ls.end(); q++) {
      AddEdge((*p), (*q));
    }
  }
  auto registers = reg_manager->AllWithoutRsp();
  for (int i = 0; i < 15; i++) {
    auto temp = registers->NthTemp(i);
    coloring->Enter(temp, reg_manager->temp_map_->Look(temp));
  }
  std::string *rsp = new std::string("%rsp");
  coloring->Enter(reg_manager->StackPointer(), rsp);
}

live::MoveList*RegAllocator::NodeMoves(graph::Node<temp::Temp> *node) {
  return (workListMoves->Union(activeMoves))->Intersect(moveList[node]);
}

bool RegAllocator::MoveRelated(graph::Node<temp::Temp> *node) {
  return
    ((workListMoves->Union(activeMoves))->Intersect(moveList[node]))->GetList().size() != 0;
}
void RegAllocator::MakeWorklist()
{
     for (auto p = liveness->GetLiveGraph().interf_graph->Nodes()->GetList().begin();
          p!=liveness->GetLiveGraph().interf_graph->Nodes()->GetList().end(); p++) {
    graph::Node<temp::Temp> *tmpNode =(*p);
    temp::Temp *tmp = tmpNode->NodeInfo();
    if (precolored(tmp)) {
      continue;
    }
    if (degree[tmpNode] >= 15) {
      spillWorkList.insert(tmpNode);
    } else if (MoveRelated(tmpNode)) {
      freezeWorkList.insert(tmpNode);
    } else {
      simplifyWorkList.insert(tmpNode);
    }
  }
}

graph::NodeList<temp::Temp> *RegAllocator::Adjacent(graph::Node<temp::Temp> *node) {
  graph::NodeList<temp::Temp> *res=new graph::NodeList<temp::Temp>();
  for (graph::Node<temp::Temp> *adj : adjList[node]) {
    if (std::find(selectStack.begin(), selectStack.end(), adj) == selectStack.end()
        && coalescedNodes.find(adj) == coalescedNodes.end()) {
      res->Append(adj);
    }
  }
  return res;
}

void RegAllocator::DecrementDegree(graph::Node<temp::Temp> *node) {
  if (precolored(node->NodeInfo())) {
    return;
  }
  int d = degree[node];
  degree[node] = d - 1;
  if (d == 15) {
    graph::NodeList<temp::Temp> *tmp=new graph::NodeList<temp::Temp>();
    tmp->Append(node);
    tmp->CatList(Adjacent(node));
    EnableMoves(tmp);
    spillWorkList.erase(node);
    if (MoveRelated(node)) {
      freezeWorkList.insert(node);
    } else {
      if (std::find(selectStack.begin(), selectStack.end(),node) != selectStack.end()) {
        printf("replicate 1!");
        exit(1);
      }
      simplifyWorkList.insert(node);
    }
  }
}

void RegAllocator::EnableMoves(graph::NodeList<temp::Temp> *nodes) {
  for (auto p = nodes->GetList().begin(); p!=nodes->GetList().end(); p++) {
    for (auto q : (NodeMoves((*p))->GetList())) {
      if (activeMoves && activeMoves->Contain((q).first,(q).second)) {
        activeMoves->Delete((q).first,(q).second);
        workListMoves ->Append((q).first,(q).second);
      }
    }
  }
}

void RegAllocator::Simplify() {
  graph::Node<temp::Temp> *tmpNode = *(simplifyWorkList.begin());
  simplifyWorkList.erase(tmpNode);
  selectStack.push_back(tmpNode);
  auto ls=Adjacent(tmpNode)->GetList();
  for (auto itr = ls.begin(); itr!=ls.end(); itr++) {
    DecrementDegree((*itr));
  }
}

graph::Node<temp::Temp> *RegAllocator::GetAlias(graph::Node<temp::Temp> *node){
  if (coalescedNodes.find(node) != coalescedNodes.end()) {
    return GetAlias(alias[node]);
  } else {
    return node;
  }
}

void RegAllocator::AddWorkList( graph::Node<temp::Temp> *node){
  if (!precolored(node->NodeInfo()) && !MoveRelated(node) && degree[node] < 15) {
    freezeWorkList.erase(node);
     if (std::find(selectStack.begin(), selectStack.end(),node) != selectStack.end())
      {
        printf("replicate 2!");
        exit(1);
      }
    simplifyWorkList.insert(node);
  }
}

bool RegAllocator::OK(graph::Node<temp::Temp> *t, graph::Node<temp::Temp> *r) {
  return degree[t] < 15
    || precolored(t->NodeInfo())
    || adjSet.find(std::make_pair(t, r)) != adjSet.end();
}

bool RegAllocator::All_OK(graph::NodeList<temp::Temp> *nodes, graph::Node<temp::Temp> *r) {
  for (auto p = nodes->GetList().begin(); p!=nodes->GetList().end(); p++) {
    if (!OK((*p), r)) {
      return false;
    }
  }
  return true;
}

bool RegAllocator::Conservative(graph::NodeList<temp::Temp> *nodes){
  //Briggs
  int k = 0;
  for (auto node = nodes->GetList().begin(); node!=nodes->GetList().end(); node++) {
    if (precolored((*node)->NodeInfo()) || degree[(*node)] >= 15) {
      k++;
    }
  }
  return k < 15;
}

void RegAllocator::Coalesce() {
  graph::Node<temp::Temp> *x, *y;
  graph::Node<temp::Temp> *u, *v;
  std::pair<graph::Node<temp::Temp> *,graph::Node<temp::Temp> *> m=workListMoves->GetList().front();
  x = m.first;
  y = m.second;

  x = GetAlias(x);
  y = GetAlias(y);
  if (precolored(y->NodeInfo())) {
    u = y;
    v = x;
  } else {
    u = x;
    v = y;
  }

  workListMoves->Delete(m.first,m.second);
  if (u == v) {
    coalescedMoves->Append(m.first,m.second);
    AddWorkList(u);
  } else if (precolored(v->NodeInfo()) || adjSet.find(std::make_pair(u, v)) != adjSet.end()) {
    constrainedMoves->Append(m.first,m.second);
    AddWorkList(u);
    AddWorkList(v);
  } else if (
    (precolored(u->NodeInfo()) && All_OK(Adjacent(v), u))
    || (!precolored(u->NodeInfo()) && Conservative(Adjacent(u)->CatList(Adjacent(v))))
    )
  {
    coalescedMoves->Append(m.first,m.second);
    Combine(u, v);
    AddWorkList(u);
  } else {
    activeMoves->Append(m.first,m.second);
  }
}

void RegAllocator::Combine(graph::Node<temp::Temp> *u,graph::Node<temp::Temp> *v){
   if (freezeWorkList.find(v) != freezeWorkList.end()) {
    freezeWorkList.erase(v);
  } else {
    spillWorkList.erase(v);
  }
  coalescedNodes.insert(v);
  alias[v] = u;
  moveList[u] = moveList[u]->Union(moveList[v]);
  graph::NodeList<temp::Temp> *arg=new graph::NodeList<temp::Temp>();
  arg->Append(v);
  EnableMoves(arg);
  auto ls=Adjacent(v)->GetList();
  for (auto p = ls.begin(); p !=ls.end(); p++) {
    AddEdge((*p), u);
    DecrementDegree((*p));
  }

  if (degree[u] >= 15 && freezeWorkList.find(u) != freezeWorkList.end()) {
    freezeWorkList.erase(u);
    spillWorkList.insert(u);
  }
}

void RegAllocator::FreezeMoves(graph::Node<temp::Temp> *u) {
  for (auto p : (NodeMoves(u)->GetList())) {
    graph::Node<temp::Temp> *v;
    auto x = (p).first;
    auto y = (p).second;
    if (GetAlias(y) == GetAlias(u)) {
      v = GetAlias(x);
    } else {
      v = GetAlias(y);
    }
    activeMoves->Delete(x,y);
    frozenMoves->Append(x,y);

    if (!precolored(v->NodeInfo()) && !NodeMoves(v) && degree[v] < 15) {
      freezeWorkList.erase(v);
       if (std::find(selectStack.begin(), selectStack.end(),v) != selectStack.end())
      {
        printf("replicate 3!");
        exit(1);
      }
      simplifyWorkList.insert(v);
    }
  }
}

void RegAllocator::Freeze() {
  auto node = *(freezeWorkList.begin());
  freezeWorkList.erase(node);
   if (std::find(selectStack.begin(), selectStack.end(),node) != selectStack.end())
      {
        printf("replicate 4!");
        exit(1);
      }
  simplifyWorkList.insert(node);
  FreezeMoves(node);
}

void RegAllocator::SelectSpill() {
  graph::Node<temp::Temp> *chosen = nullptr;
  double chosen_priority = 100000000;
  for (auto node : spillWorkList) {
    if (noSpillTemp.find(node->NodeInfo()) != noSpillTemp.end()) {
      continue;
    }
    if ((*(liveness->GetLiveGraph().priority))[node->NodeInfo()] < chosen_priority) {
      chosen = node;
      chosen_priority = (*(liveness->GetLiveGraph().priority))[node->NodeInfo()];
    }
  }
  if (!chosen) {
    printf("no chosen here!\n");
    exit(1);
  }
  spillWorkList.erase(chosen);
   if (std::find(selectStack.begin(), selectStack.end(),chosen) != selectStack.end())
      {
        printf("replicate 5!");
        exit(1);
      }
  simplifyWorkList.insert(chosen);
  FreezeMoves(chosen);
}

void RegAllocator::AssignColors() {
  while (!selectStack.empty()) {
    auto n = selectStack[selectStack.size() - 1];
    selectStack.pop_back();
    std::set<std::string> okColors;
    auto colorVec = reg_manager->colors;
    okColors.insert(colorVec.begin(), colorVec.end());
    for (auto w : adjList[n]) {
      if (coloredNodes.find(GetAlias(w)) != coloredNodes.end()
          || precolored(GetAlias(w)->NodeInfo()))
      {
        okColors.erase(*(coloring->Look(GetAlias(w)->NodeInfo())));
      }
    }
    if (okColors.empty()) {
      spilledNodes.insert(n);
    } else {
      coloredNodes.insert(n);
      std::string *color = new std::string;
      *color = *(okColors.begin());
      coloring->Enter(n->NodeInfo(), color);
    }
  }
  for (auto n : coalescedNodes) {
    coloring->Enter(n->NodeInfo(), coloring->Look(GetAlias(n)->NodeInfo()));
  }
}

cg::AssemInstr *RegAllocator::RewriteProgram(frame::Frame *frame, assem::InstrList *il) {
  assem::InstrList *newIl = new assem::InstrList();
  for (auto node : spilledNodes) {
    temp::Temp *spilledTemp = node->NodeInfo();
    auto access = static_cast<frame::InFrameAccess *>(frame->AllocLocal(true));
    for (auto p = il->GetList().begin(); p != il->GetList().end(); p++) {
      temp::TempList *src, *dst;
      if (typeid(*(*p)) == typeid(assem::LabelInstr)) {
        src = nullptr;
        dst = nullptr;
      }
      else if (typeid(*(*p)) == typeid(assem::MoveInstr)) {
        src = ((assem::MoveInstr *)(*p))->src_;
        dst = ((assem::MoveInstr *)(*p))->dst_;
      }
      else if (typeid(*(*p)) == typeid(assem::OperInstr)) {
        src = ((assem::OperInstr *)(*p))->src_;
        dst = ((assem::OperInstr *)(*p))->dst_;
      } else {
        printf("error ocurred!");
        exit(1);
      }
      if (src && src->Contain(spilledTemp) && dst && dst->Contain(spilledTemp)) {
        temp::Temp *newTemp = temp::TempFactory::NewTemp();
        noSpillTemp.insert(newTemp);
        src->Replace(spilledTemp, newTemp);
        dst->Replace(spilledTemp, newTemp);
        std::ostringstream assem;
        assem << "movq (" << frame->label->Name() << "_framesize" << access->offset << ")(`s0), `d0";
        newIl->Append(
          new assem::OperInstr(
            assem.str(),
            new temp::TempList(newTemp),
            new temp::TempList(reg_manager->StackPointer()),
            nullptr
          )
        );
        newIl->Append((*p));
        assem.str(std::string());
        assem << "movq `s0, (" << frame->label->Name() << "_framesize" << access->offset << ")(`s1)";
        newIl->Append(
          new assem::OperInstr(
            assem.str(),
            nullptr,
            new temp::TempList{newTemp, reg_manager->StackPointer()},
            nullptr
          )
        );
      } else if (src && src->Contain(spilledTemp)) {
        temp::Temp *newTemp = temp::TempFactory::NewTemp();
        noSpillTemp.insert(newTemp);
        src->Replace(spilledTemp, newTemp);
        std::ostringstream assem;
        assem << "movq (" << frame->label->Name() << "_framesize" << access->offset << ")(`s0), `d0";
        newIl->Append(
          new assem::OperInstr(
            assem.str(),
            new temp::TempList(newTemp),
            new temp::TempList(reg_manager->StackPointer()),
            nullptr)
        );
        newIl->Append((*p));
      } else if (dst && dst->Contain(spilledTemp)) {
        temp::Temp *newTemp = temp::TempFactory::NewTemp();
        noSpillTemp.insert(newTemp);
        dst->Replace(spilledTemp, newTemp);
        newIl->Append((*p));
        std::ostringstream assem;
        assem << "movq `s0,(" << frame->label->Name() << "_framesize" << access->offset << ")(`s1)";
        newIl->Append(
          new assem::OperInstr(
            assem.str(),
            nullptr,
            new temp::TempList{newTemp, reg_manager->StackPointer()},
            nullptr
          )
        );
      } else {
        newIl->Append((*p));
      }
    }
    il = newIl;
    newIl = new assem::InstrList();
  }
  return new cg::AssemInstr(il);
}

void RegAllocator::RegAlloc(){
  temp::Map *co=temp::Map::LayerMap(reg_manager->temp_map_, temp::Map::Name());
  fg::FlowGraphFactory  *cfg=new fg::FlowGraphFactory(asi_->GetInstrList());
  cfg->AssemFlowGraph();
  liveness=new live::LiveGraphFactory(cfg->GetFlowGraph());
  liveness->Liveness();
  printf("liveness done!\n");
  liveness->GetLiveGraph().interf_graph->Show(
    stderr,
    liveness->GetLiveGraph().interf_graph->Nodes(),
    [](temp::Temp *t) {printf("nodeinfo:%d\n",t->Int());}
  );
  Build();
  printf("finish build\n");
  MakeWorklist();
  printf("finish makeworklist\n");
  do {
    if (!simplifyWorkList.empty()) {
      // printf("enter 1\n");
      Simplify();
      // printf("done 1\n");
    } else if (workListMoves->GetList().size()) {
      // printf("enter 2\n");
      Coalesce();
      // printf("done 2\n");
    } else if (!freezeWorkList.empty()) {
      // printf("enter 3\n");
      Freeze();
      // printf("done 3\n");
    } else if (!spillWorkList.empty()) {
      // printf("enter 4\n");
      SelectSpill();
      // printf("done 4\n");
    }
  } while (
    !simplifyWorkList.empty() || workListMoves->GetList().size() 
    || !freezeWorkList.empty() || !spillWorkList.empty());

  printf("finish 4 step\n");
  AssignColors();
  printf("finish assigncolors\n");
  if (!spilledNodes.empty()) {
    for (auto ns:spilledNodes) {
      printf("spill nodes:%d\n",ns->NodeInfo()->Int());
    }
    cg::AssemInstr *il = RewriteProgram(frame_, asi_->GetInstrList());
    std::unique_ptr<cg::AssemInstr> il2=std::unique_ptr<cg::AssemInstr>(il);
    il2->Print(stderr,co);
    RegAllocator tmp(frame_,std::move(il2));
    tmp.RegAlloc();
    result_=tmp.TransferResult();
  } else {
    Result *result=new Result();
    result->coloring_ = coloring;
    result->il_=asi_->GetInstrList();
    std::unique_ptr<ra::Result> tp=std::unique_ptr<ra::Result>(result);
    result_=std::move(tp);
  }
}

} // namespace ra