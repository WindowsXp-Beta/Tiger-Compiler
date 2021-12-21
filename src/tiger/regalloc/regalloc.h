#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"
#include <set>
#include <map>

namespace ra {

class Result {
public:
  temp::Map *coloring_;
  assem::InstrList *il_;

  Result() : coloring_(nullptr), il_(nullptr) {}
  Result(temp::Map *coloring, assem::InstrList *il)
      : coloring_(coloring), il_(il) {}
  Result(const Result &result) = delete;
  Result(Result &&result) = delete;
  Result &operator=(const Result &result) = delete;
  Result &operator=(Result &&result) = delete;
  ~Result(){};
};

class RegAllocator {
  /* TODO: Put your lab6 code here */
  public:
  void RegAlloc();
   std::unique_ptr<ra::Result> TransferResult() {
    return std::move(result_);
  }
  RegAllocator(frame::Frame *frame,std::unique_ptr<cg::AssemInstr> asi){
    result_=std::unique_ptr<ra::Result>(new Result());
    frame_ = frame;
    asi_=std::move(asi);
    liveness=nullptr;
    coloring=temp::Map::Empty();
    activeMoves=new live::MoveList();
    coalescedMoves=new live::MoveList();
    constrainedMoves=new live::MoveList();
    frozenMoves=new live::MoveList();
  }
  std::unique_ptr<ra::Result> result_;
  std::unique_ptr<cg::AssemInstr> asi_;
  frame::Frame *frame_;
  live::LiveGraphFactory *liveness;
  //
  std::set<graph::Node<temp::Temp> *> spillWorkList, freezeWorkList, simplifyWorkList;
  std::set<graph::Node<temp::Temp> *> coalescedNodes, spilledNodes, coloredNodes;
  std::vector<graph::Node<temp::Temp> *> selectStack;
  //
  live::MoveList *workListMoves = nullptr, *activeMoves = nullptr, *coalescedMoves = nullptr, *constrainedMoves = nullptr, *frozenMoves = nullptr;
  //
  std::set<std::pair<graph::Node<temp::Temp> *, graph::Node<temp::Temp> *>> adjSet;
  std::map<graph::Node<temp::Temp> *, std::set<graph::Node<temp::Temp> *>> adjList;
  std::map<graph::Node<temp::Temp> *, int> degree;
  std::map<graph::Node<temp::Temp> *, live::MoveList *> moveList;
  std::map<graph::Node<temp::Temp> *, graph::Node<temp::Temp>*> alias;
  temp::Map *coloring ;
  //
  void Build();
  void AddEdge(graph::Node<temp::Temp> *src,graph::Node<temp::Temp> *dst);
  void MakeWorklist();
  live::MoveList *NodeMoves(graph::Node<temp::Temp> *node);
  bool MoveRelated(graph::Node<temp::Temp> *node);
  void Simplify();
  graph::NodeList<temp::Temp> *Adjacent(graph::Node<temp::Temp> *node);
  void DecrementDegree(graph::Node<temp::Temp> *node);
  void EnableMoves(graph::NodeList<temp::Temp> *nodes);
  void Coalesce();
  graph::Node<temp::Temp> *GetAlias(graph::Node<temp::Temp> *node);
  void AddWorkList( graph::Node<temp::Temp> *node);
  bool OK(graph::Node<temp::Temp> *t,graph::Node<temp::Temp> *r);
  bool All_OK(graph::NodeList<temp::Temp> *nodes, graph::Node<temp::Temp> *r);
  bool Conservative(graph::NodeList<temp::Temp> *nodes);
  void Combine(graph::Node<temp::Temp> *u,graph::Node<temp::Temp> *v);
  void Freeze();
  void FreezeMoves(graph::Node<temp::Temp> *u);
  void SelectSpill();
  void AssignColors();
  cg::AssemInstr *RewriteProgram(frame::Frame *f, assem::InstrList *il);
};

} // namespace ra

#endif