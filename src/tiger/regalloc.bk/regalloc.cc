#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"
#include <sstream>

#define DEBUG

// fprintf(debug_log, "%d,%s: ", __LINE__, __func__);
#ifdef DEBUG
  #define LOG(format, args...) do{            \
    FILE *debug_log = fopen("register.log", "a+"); \
    fprintf(debug_log, format, ##args);       \
    fclose(debug_log);\
  } while(0)
#else
  #define LOG(format, args...) do{} while(0)
#endif

extern frame::RegManager *reg_manager;

namespace ra {
/* TODO: Put your lab6 code here */
RegAllocator::RegAllocator(
    frame::Frame *frame,
    std::unique_ptr<cg::AssemInstr> assem_instr) :
    frame_(frame), assem_instr_(std::move(assem_instr))
    {
      result_ = std::make_unique<Result>();
    }

void RegAllocator::RegAlloc() {
  while (true) {
    Build();
    MakeWorklist();
    showStatus();
    while (!simplifyWorklist->Empty() || !worklistMoves->Empty()
           || !freezeWorklist->Empty() || !spillWorklist->Empty())
    {
      if (!simplifyWorklist->Empty()) {
        LOG("before simplify\n");
        showStatus();
        Simplify();
        LOG("after simplify\n");
        showStatus();
      }
      else if (!worklistMoves->Empty()) {
        LOG("before coalesce\n");
        showStatus();
        Coalesce();
        LOG("after coalesce\n");
        showStatus();
      }
      else if (!freezeWorklist->Empty()) {
        Freeze();
      }
      else if (!spillWorklist->Empty()) SelectSpill();
    }

    AssignColors();

    if (!spilledNodes->Empty()) {
      RewriteProgram();
      GarbageCollection();
    } else {
      result_->coloring_ = AssignRegisters();
      result_->il_ = assem_instr_.get()->GetInstrList();
      break;
      // TODO: remove move instrutions
    }
  }
}

void RegAllocator::showStatus() {
  LOG("---live graph---\n");
  for (auto node : live_graph->interf_graph->Nodes()->GetList()) {
    LOG("%d's neighbor: ", node->NodeInfo()->Int());
    for (auto adj : node->Succ()->GetList()) {
      LOG("%d\t", adj->NodeInfo()->Int());
    }
    LOG("\n");
  }
  LOG("---simplifyWorklist---\n");
  for (auto node : simplifyWorklist->GetList()) {
    LOG("%d\t", node->NodeInfo()->Int());
  }
  LOG("\n");
  LOG("---freezeWorklist---\n");
  for (auto node : freezeWorklist->GetList()) {
    LOG("%d\t", node->NodeInfo()->Int());
  }
  LOG("\n");
  LOG("---Color---\n");
  for (auto pair : color) {
    LOG("%d is %d\t", pair.first->NodeInfo()->Int(), pair.second->Int());
  }
  LOG("\n");
}


assem::InstrList *RegAllocator::CoalesceInstr() {
  auto coalesce_instrs = new assem::InstrList();
  for (auto instr : assem_instr_->GetInstrList()->GetList()) {
    if (typeid(*instr) == typeid(assem::MoveInstr)) {
      auto move_instr = static_cast<assem::MoveInstr *>(instr);
      assert(move_instr->src_->GetList().size() == 1);
      assert(move_instr->dst_->GetList().size() == 1);
      auto dst = move_instr->dst_->GetList().front();
      auto src = move_instr->src_->GetList().front();
      if (result_->coloring_->Look(dst) == result_->coloring_->Look(src)) {
        std::ostringstream assem;
        assem << "# " << move_instr->assem_;
        coalesce_instrs->Append(
          new assem::MoveInstr(
            assem.str(),
            nullptr, nullptr
          )
        );
      } else {
        coalesce_instrs->Append(move_instr); 
      }
    }
  }
}

void RegAllocator::Build() {
  auto flow_graph_factory = fg::FlowGraphFactory(assem_instr_->GetInstrList());
  flow_graph_factory.AssemFlowGraph();
  live_graph_factory_ = new live::LiveGraphFactory(flow_graph_factory.GetFlowGraph());
  live_graph_factory_->Liveness();
  live_graph = live_graph_factory_->GetLiveGraph();
  worklistMoves = live_graph->moves;
  auto temp2Inode = live_graph_factory_->GetTempNodeMap();

  simplifyWorklist = new live::INodeList();
  freezeWorklist = new live::INodeList();
  spillWorklist = new live::INodeList();
  spilledNodes = new live::INodeList();
  coalescedNodes = new live::INodeList();
  coloredNodes = new live::INodeList();
  selectStack = new live::INodeList();

  activeMoves = new live::MoveList();
  coalescedMoves = new live::MoveList();
  frozenMoves = new live::MoveList();

  for (auto reg : reg_manager->AllWithoutRsp()->GetList()) {
    coloredNodes->Append(temp2Inode[reg]);
    color[temp2Inode[reg]] = reg;
  }

  auto nodes = live_graph->interf_graph->Nodes();
  for (auto node : nodes->GetList()) {
    auto new_movelist = new live::MoveList();
    for (auto move : worklistMoves->GetList()) {
      if (move.first == node || move.second == node) {
        new_movelist->Append(move.first, move.second);
      }
    }
    movelist[node] = new_movelist;
    degree[node] = node->Degree();
  }
}

void RegAllocator::AddEdge(live::INodePtr u, live::INodePtr v) {
  if (!u->Succ()->Contain(v) && u != v) {
    live_graph->interf_graph->AddEdge(u, v);
    live_graph->interf_graph->AddEdge(v, u);
    degree[u]++;
    degree[v]++;
  }
}

void RegAllocator::MakeWorklist() {
  auto nodes = live_graph->interf_graph->Nodes();
  for (auto node : nodes->GetList()) {
    if (!coloredNodes->Contain(node)) {
      if (node->Degree() >= K)
        spillWorklist->Append(node);
      else if (MoveRelated(node))
        freezeWorklist->Append(node);
      else
        simplifyWorklist->Append(node);
    }
  }
}

live::INodeListPtr RegAllocator::Adjacent(live::INodePtr node) {
  auto raw_adj = node->Adj();
  auto union_list = selectStack->Union(coalescedNodes);
  auto real_adj = raw_adj->Diff(union_list);
  delete raw_adj;
  delete union_list;
  return real_adj;
}

live::MoveList *RegAllocator::NodeMoves(live::INodePtr node) {
  auto union_list = activeMoves->Union(worklistMoves);
  auto result = movelist[node]->Intersect(union_list);
  delete union_list;
  return result;
}

bool RegAllocator::MoveRelated(live::INodePtr node) {
  auto node_moves = NodeMoves(node);
  bool is_empty = node_moves->GetList().empty();
  delete node_moves;
  return !is_empty;
}

void RegAllocator::DecrementDegree(live::INodePtr node) {
  int d = degree[node];
  degree[node] = d - 1;
  if (d == K) {
    auto m_with_adj = Adjacent(node);
    m_with_adj->Append(node);
    EnableMoves(m_with_adj);
    spillWorklist->DeleteNode(node);
    if (MoveRelated(node))
      freezeWorklist->Append(node);
    else simplifyWorklist->Append(node);
  }
}

void RegAllocator::EnableMoves(live::INodeListPtr nodes) {
  for (auto node : nodes->GetList()) {
    auto node_moves = NodeMoves(node);
    for (auto pair_node : node_moves->GetList()) {
      if (activeMoves->Contain(pair_node.first, pair_node.second)) {
        assert(!activeMoves->Contain(pair_node.second, pair_node.first));
        activeMoves->Delete(pair_node.first, pair_node.second);
      }
    }
  }
}

void RegAllocator::Simplify() {
  auto node = simplifyWorklist->GetList().front();
  simplifyWorklist->DeleteNode(node);
  selectStack->Prepend(node);
  auto adjacent = Adjacent(node);
  for (auto adj_node : adjacent->GetList()) {
    DecrementDegree(adj_node);
  }
  delete adjacent;
}

void RegAllocator::Coalesce() {
  auto m = worklistMoves->GetList().front();
  auto x = GetAlias(m.first);
  auto y = GetAlias(m.second);
  live::INodePtr u, v;
  if (coloredNodes->Contain(y)) {
    u = y;
    v = x;
  } else {
    u = x;
    v = y;
  }
  worklistMoves->Delete(m.first, m.second);
  //we don't need to delete twice here
  if (u == v) {
    coalescedMoves->Append(m.first, m.second);
    AddWorkList(u);
  } else if (coloredNodes->Contain(v) || u->Succ()->Contain(v)) {
    constrainedMoves->Append(u, v);
    AddWorkList(u);
    AddWorkList(v);
  } else if (
    (coloredNodes->Contain(u) && OK(v, u))
    || (!coloredNodes->Contain(u) && Conservative(u, v))) {
      coalescedMoves->Append(u, v);
        Combine(u, v);
        AddWorkList(u);
    } else activeMoves->Append(m.first, m.second);
}

live::INodePtr RegAllocator::GetAlias(live::INodePtr node) {
  if (coalescedNodes->Contain(node))
    return GetAlias(alias[node]);
  else return node;
}

void RegAllocator::AddWorkList(live::INodePtr node) {
  if (!coloredNodes->Contain(node) 
      && !MoveRelated(node) 
      && degree[node] < K) {
        freezeWorklist->DeleteNode(node);
        simplifyWorklist->Append(node);
      }
}

bool RegAllocator::OK(live::INodePtr v, live::INodePtr u) {
  auto v_adj = Adjacent(v);
  for (auto t : v_adj->GetList()) {
    if (!(degree[t] < K || coloredNodes->Contain(t) || t->Succ()->Contain(u))) {
      delete v_adj;
      return false;
    }
  }
  delete v_adj;
  return true;
}

bool RegAllocator::Conservative(live::INodePtr u, live::INodePtr v) {
  auto adj_u = Adjacent(u);
  auto adj_v = Adjacent(v);
  auto v_union_u = adj_u->Union(adj_v);
  int k = 0;
  for (auto node : v_union_u->GetList()) {
    if (degree[node] >= K) k++;
  }
  delete adj_u;
  delete adj_v;
  delete v_union_u;
  return k < K;
}

void RegAllocator::Combine(live::INodePtr u, live::INodePtr v) {
  if (freezeWorklist->Contain(v)) {
    freezeWorklist->DeleteNode(v);
  } else {
    spillWorklist->DeleteNode(v);
  }
  coalescedNodes->Append(v);
  alias[v] = u;
  auto u_moves = movelist[u];
  movelist[u] = u_moves->Union(movelist[v]);
  delete u_moves;
  auto v_adj = Adjacent(v);
  for (auto t : v_adj->GetList()) {
    AddEdge(t, u);
    DecrementDegree(t);
  }
  if (degree[u] >= K && freezeWorklist->Contain(u)) {
    freezeWorklist->DeleteNode(u);
    spillWorklist->Append(u);
  }
}

void RegAllocator::Freeze() {
  auto u = freezeWorklist->GetList().front();
  freezeWorklist->DeleteNode(u);
  simplifyWorklist->Append(u);
  FreezeMoves(u);
}

void RegAllocator::FreezeMoves(live::INodePtr u) {
  auto node_moves = NodeMoves(u);
  for (auto move_pair : node_moves->GetList()) {
    live::INodePtr v;
    if (GetAlias(move_pair.second) == GetAlias(u))
      v = GetAlias(move_pair.first);
    else v = GetAlias(move_pair.second);
    activeMoves->Delete(move_pair.first, move_pair.second);
    frozenMoves->Append(move_pair.first, move_pair.second);
    if (!MoveRelated(v) && degree[v] < K) {
      freezeWorklist->DeleteNode(v);
      simplifyWorklist->Append(v);
    }
  }
  delete node_moves;
}

void RegAllocator::SelectSpill() {
  auto m = spillWorklist->GetList().front();
  spillWorklist->DeleteNode(m);
  simplifyWorklist->Append(m);
  FreezeMoves(m);
}

void RegAllocator::AssignColors() {
  for (auto n : selectStack->GetList()) {
    auto okColors = reg_manager->AllWithoutRsp();
    for (auto adj_node : n->Succ()->GetList()) {
      if (!coloredNodes->Contain(GetAlias(adj_node)))
        okColors->Remove(color[GetAlias(adj_node)]);
    }
    if (okColors->GetList().empty()) {
      spilledNodes->Append(n);
    } else {
      coloredNodes->Append(n);
      color[n] = okColors->GetList().front();
    }
  }

  for (auto n : coalescedNodes->GetList()) {
    color[n] = color[GetAlias(n)];
  }
}

temp::TempList *replaceTempList(temp::TempList *temp_list, temp::Temp *old_temp, temp::Temp *new_temp) {
  auto new_temp_list = new temp::TempList();
  for (auto temp : temp_list->GetList()) {
    if (temp == old_temp) {
      new_temp_list->Append(new_temp);
    } else new_temp_list->Append(temp);
  }
  delete temp_list;
  return new_temp_list;
}

void RegAllocator::RewriteProgram() {
  for (auto node : spilledNodes->GetList()) {
    auto access = static_cast<frame::InFrameAccess *>(frame_->AllocLocal(true));
    auto spilled_temp = node->NodeInfo();
    auto end_flag = assem_instr_->GetInstrList()->GetList().end();
    auto instr_it = assem_instr_->GetInstrList()->GetList().begin();
    for (; instr_it != end_flag; instr_it++) {
      temp::TempList **src = nullptr, **dst = nullptr;
      if (typeid(*(*instr_it)) == typeid(assem::MoveInstr)) {
        src = &static_cast<assem::MoveInstr *>(*instr_it)->src_;
        dst = &static_cast<assem::MoveInstr *>(*instr_it)->dst_;
      } else if (typeid(*instr_it) == typeid(assem::OperInstr)) {
        src = &static_cast<assem::OperInstr *>(*instr_it)->src_;
        dst = &static_cast<assem::OperInstr *>(*instr_it)->dst_;
      } else {
        continue;// LabelInstr
      }

      if (src && (*src)->Contain(spilled_temp)) {
        auto new_temp = temp::TempFactory::NewTemp();
        noSpillTemps.push_back(new_temp);
        *src = replaceTempList(*src, spilled_temp, new_temp);

        std::ostringstream assem;
        assem << "# Warning:Spill before\n" << "movq ("
          << frame_->label->Name() << "_framesize" << access->offset << ")(`s0),`d0";
        assem_instr_->GetInstrList()->Insert(
          instr_it, new assem::OperInstr(
            assem.str(),
            new temp::TempList(reg_manager->StackPointer()),
            new temp::TempList(spilled_temp),
            nullptr
          )
        );
      }

      if (dst && (*dst)->Contain(spilled_temp)) {
        auto new_temp = temp::TempFactory::NewTemp();
        noSpillTemps.push_back(new_temp);
        *dst = replaceTempList(*dst, spilled_temp, new_temp);

        std::ostringstream assem;
        assem << "# Warning:Spill after\n" << "movq `s0,("
          << frame_->label->Name() << "_framesize" << access->offset << ")(`d0)";
        assem_instr_->GetInstrList()->Insert(
          std::next(instr_it), new assem::OperInstr(
            assem.str(),
            new temp::TempList(spilled_temp),
            new temp::TempList(reg_manager->StackPointer()),
            nullptr
          )
        );
      }
    }
  }
}

temp::Map *RegAllocator::AssignRegisters() {
  auto reg_map = temp::Map::Empty();
  for (auto node : live_graph->interf_graph->Nodes()->GetList()) {
    reg_map->Enter(node->NodeInfo(), reg_manager->temp_map_->Look(color[node]));
  }
  return reg_map;
}

void RegAllocator::GarbageCollection() {
  delete live_graph_factory_;
  for (auto move_pair : movelist) {
    delete move_pair.second;
  }

  delete simplifyWorklist;
  delete freezeWorklist;
  delete spillWorklist;
  delete spilledNodes;
  delete coalescedNodes;
  delete coloredNodes;
  delete selectStack;

  delete activeMoves;
  delete coalescedMoves;
  delete frozenMoves;
}

RegAllocator::~RegAllocator() {
  GarbageCollection();
}

} // namespace ra