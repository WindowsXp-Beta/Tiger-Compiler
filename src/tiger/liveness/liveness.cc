#include "tiger/liveness/liveness.h"
#include <algorithm>
#include <map>

// else printf("[ERROR]File: %s line: %d: ", __FILE__, __LINE__); \

#define DEBUG_LIVE 0

#define debug_log(flag, ...) do{ \
    if (DEBUG_LIVE) { \
      if (flag) printf("[INFO]File: %s line: %d: ", __FILE__, __LINE__); \
      printf(__VA_ARGS__); \
      fflush(stdout); \
    } }while(0);

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
  for (auto move : move_list_) {
    res->move_list_.push_back(move);
  }
  for (auto move : list->GetList()) {
    if (!res->Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

bool Contains(temp::TempList *base, temp::Temp *target) {
  assert(base);
  auto temp_list = base->GetList();
  return std::find(temp_list.begin(), temp_list.end(), target) != temp_list.end();
}

temp::TempList *Union(temp::TempList *lhs, temp::TempList *rhs) {
  if (lhs == nullptr) {
    if (rhs == nullptr) return new temp::TempList();
    return new temp::TempList(rhs->GetList());
  }
  auto union_list = new temp::TempList(lhs->GetList());
  for (auto temp : rhs->GetList()) {
    if (!Contains(union_list, temp)) union_list->Append(temp);
  }
  return union_list;
}

temp::TempList *Subtract(temp::TempList *lhs, temp::TempList *rhs) {
  if (lhs == nullptr) {
    return new temp::TempList();
  }
  auto result_list = new temp::TempList(lhs->GetList());
  if (rhs != nullptr) {
    for (auto temp : rhs->GetList()) {
      result_list->Remove(temp);
    }
  }
  return result_list;
}

bool Equal(temp::TempList *lhs, temp::TempList *rhs) {
  if (lhs == nullptr) {
    if (rhs == nullptr) return true;
    return false;
  }
  if (rhs == nullptr) return false;

  for (auto temp : lhs->GetList()) {
    if (!Contains(rhs, temp)) {
      return false;
    }
  }
  return true;
}

void LiveGraphFactory::ShowInOut() {
  // return;
  debug_log(true, "\n-----liveness-----\n");
  auto end_flag = flowgraph_->Nodes()->GetList().rend();
  auto first = flowgraph_->Nodes()->GetList().rbegin();
  for (auto flow_node_it = first; flow_node_it != end_flag; flow_node_it++) {
    auto instr = (*flow_node_it)->NodeInfo();
    if (typeid(*instr) == typeid(assem::MoveInstr)) {
      debug_log(false, "%s: ", static_cast<assem::MoveInstr *>(instr)->assem_.data());
    } else if (typeid(*instr) == typeid(assem::OperInstr)) {
      debug_log(false, "%s: ", static_cast<assem::OperInstr *>(instr)->assem_.data());
    } else continue;
    auto in = in_->Look(*flow_node_it);
    debug_log(false, "In is: ");
    for (auto temp : in->GetList()) {
      debug_log(false, "%d\t", temp->Int());
    }
    auto out = out_->Look(*flow_node_it);
    debug_log(false, "\nOut is: ");
    for (auto temp : out->GetList()) {
      debug_log(false, "%d\t", temp->Int());
    }
    debug_log(false, "\n");
  }
}

void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */
  for (auto flow_node : flowgraph_->Nodes()->GetList()) {
    in_->Enter(flow_node, new temp::TempList());
    out_->Enter(flow_node, new temp::TempList());
  }

  auto end_flag = flowgraph_->Nodes()->GetList().rend();
  auto first = flowgraph_->Nodes()->GetList().rbegin();
  while (true) {
    bool is_all_same = true;
    ShowInOut();
    for (auto flow_node_it = first; flow_node_it != end_flag; flow_node_it++) {
      auto old_in = in_->Look(*flow_node_it);
      auto old_out = out_->Look(*flow_node_it);
      auto out_minus_def = Subtract(old_out, (*flow_node_it)->NodeInfo()->Def());
      auto new_in = Union((*flow_node_it)->NodeInfo()->Use(), out_minus_def);

      auto succ_it = (*flow_node_it)->Succ()->GetList().begin();
      auto end_flag = (*flow_node_it)->Succ()->GetList().end();
      temp::TempList *new_out = nullptr;
      if (succ_it != end_flag) { //skip last instr
        new_out = new temp::TempList(*in_->Look(*succ_it));
        succ_it++;
        temp::TempList *last_out = nullptr;
        for (; succ_it != end_flag; succ_it++) {
          last_out = new_out;
          new_out = Union(new_out, in_->Look(*succ_it));
          // In case of memory leak, because each time Union will new a temp::TempList
          delete last_out;
        }
        if (!Equal(new_in, old_in) || !Equal(new_out, old_out)) is_all_same = false;
        out_->Enter(*flow_node_it, new_out);
        delete old_out;
      }
      in_->Enter(*flow_node_it, new_in);
      delete old_in;
    }
    if (is_all_same) break;
  }
}

void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */
  // add precolored nodes
  auto all_without_rsp = reg_manager->AllWithoutRsp();
  for (auto temp : all_without_rsp->GetList()) {
    temp_node_map_[temp] = live_graph_.interf_graph->NewNode(temp);
  }

  auto temp_it = all_without_rsp->GetList().begin();
  auto end_flag = all_without_rsp->GetList().end();
  for (; temp_it != end_flag; temp_it++) {
    for (auto temp_inner_it = std::next(temp_it); temp_inner_it != end_flag; temp_inner_it++) {
      assert(temp_inner_it != end_flag);
      live_graph_.interf_graph->AddEdge(temp_node_map_[*temp_it], temp_node_map_[*temp_inner_it]);
      live_graph_.interf_graph->AddEdge(temp_node_map_[*temp_inner_it], temp_node_map_[*temp_it]);
    }
  }
  delete all_without_rsp;
  // add live temp
  auto nodes = flowgraph_->Nodes()->GetList();
  for (auto flow_node : nodes) {
    auto out_temps = out_->Look(flow_node)->GetList();
    for (auto temp : out_temps) {
      if (!temp_node_map_.count(temp))
        temp_node_map_[temp] = live_graph_.interf_graph->NewNode(temp);
    }
  }
  auto rsp = reg_manager->StackPointer();
  for (auto flow_node : nodes) {
    auto assem = flow_node->NodeInfo();
    auto live = out_->Look(flow_node);
    if (typeid(*assem) == typeid(assem::MoveInstr)) {
      auto out_minus_use = Subtract(live, assem->Use());
      for (auto def : assem->Def()->GetList()) {
        // function doesn't use its parameters
        if (temp_node_map_.count(def) == 0) {
          temp_node_map_[def] = live_graph_.interf_graph->NewNode(def);
        }
        if (def == rsp) continue;
        for (auto out : out_minus_use->GetList()) {
          if (out == rsp) continue;
          // Otherwise we will have to modify codegen and
          // change all the StackPoiner into hard code
          // debug_log(false, "current def is %d\tout is %d\n", def->Int(), out->Int());
          live_graph_.interf_graph->AddEdge(temp_node_map_.at(def), temp_node_map_.at(out));
          live_graph_.interf_graph->AddEdge(temp_node_map_.at(out), temp_node_map_.at(def));
        }
        for (auto use : assem->Use()->GetList()) {
          if (use == rsp) continue;
          if (!live_graph_.moves->Contain(temp_node_map_[def], temp_node_map_[use])
              && !live_graph_.moves->Contain(temp_node_map_[use], temp_node_map_[def])) {
                live_graph_.moves->Append(temp_node_map_[def], temp_node_map_[use]);
              }
        }
      }
    } else {
      for (auto def : assem->Def()->GetList()) {
        if (def == rsp) continue;
        for (auto out : live->GetList()) {
          if (out == rsp) continue;
          live_graph_.interf_graph->AddEdge(temp_node_map_[def], temp_node_map_[out]);
          live_graph_.interf_graph->AddEdge(temp_node_map_[out], temp_node_map_[def]);
        }
      }
    }
  }
}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live
