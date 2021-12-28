#ifndef TIGER_REGALLOC_REGALLOC_H_
#define TIGER_REGALLOC_REGALLOC_H_

#include "tiger/codegen/assem.h"
#include "tiger/codegen/codegen.h"
#include "tiger/frame/frame.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/regalloc/color.h"
#include "tiger/util/graph.h"
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
  ~Result() {
    delete coloring_;
    delete il_;
  };
};

class RegAllocator {
  /* TODO: Put your lab6 code here */
public:
  void showStatus();
  RegAllocator(
    frame::Frame *frame,
    std::unique_ptr<cg::AssemInstr> assem_instr);
  ~RegAllocator();
  std::unique_ptr<Result> TransferResult() { return std::move(result_); }
  assem::InstrList *CoalesceInstr();
  void RegAlloc();
  void GarbageCollection();
  void Build();
  void AddEdge(live::INodePtr u, live::INodePtr v);
  void MakeWorklist();
  live::MoveList *NodeMoves(live::INodePtr node);
  live::INodeListPtr Adjacent(live::INodePtr node);
  bool MoveRelated(live::INodePtr node);
  void DecrementDegree(live::INodePtr node);
  void Simplify();
  void EnableMoves(live::INodeListPtr nodes);
  void Coalesce();
  live::INodePtr GetAlias(live::INodePtr node);
  void AddWorkList(live::INodePtr node);
  bool OK(live::INodePtr v, live::INodePtr u);
  bool Conservative(live::INodePtr u, live::INodePtr v);
  void Combine(live::INodePtr u, live::INodePtr v);
  void Freeze();
  void FreezeMoves(live::INodePtr u);
  void SelectSpill();
  void AssignColors();
  void RewriteProgram();
  temp::Map *AssignRegisters();

private:
  const int K = 15;
  frame::Frame *frame_;
  std::unique_ptr<cg::AssemInstr> assem_instr_;
  std::unique_ptr<Result> result_;
  //Since it doesn't have a default construct function
  //and cannot be put into construction list
  //I put it into the heap to avoid auto consturct
  live::LiveGraphFactory *live_graph_factory_;
  live::LiveGraph *live_graph;

  //list of low-degree non-move-related nodes
  live::INodeListPtr simplifyWorklist;
  //low-degree move-related nodes
  live::INodeListPtr freezeWorklist;
  //high-degree nodes
  live::INodeListPtr spillWorklist;
  //nodes marked for spilling during this round; initially empty
  live::INodeListPtr spilledNodes;
  //Moves enabled for coalescing
  live::MoveList *worklistMoves;
  //moves not yet ready for coalescing
  live::MoveList *activeMoves;
  //moves that have been coalesced.
  live::MoveList *coalescedMoves;
  //moves whose source and target interfere
  live::MoveList *constrainedMoves;
  //moves that will no longer be considered for coalescing
  live::MoveList *frozenMoves;
  //registers that have been coalesced; when u←v is coalesced,
  //vis added to this set and u put back on some work-list (or vice versa)
  live::INodeListPtr coalescedNodes;
  //nodes successfully colored
  live::INodeListPtr coloredNodes;
  //stack containing temporaries removed from the graph
  live::INodeListPtr selectStack;
  //the color chosen by the algorithm for a node
  //for precolored nodes this is initialized to the given color
  std::map<live::INodePtr, temp::Temp *> color;

  //a mapping from a node to the list of moves it is associated with
  std::map<live::INodePtr, live::MoveList *> movelist;

  //a map recording node to their current degree
  std::map<live::INodePtr, int> degree;

  //a map recording node to its alias
  std::map<live::INodePtr, live::INodePtr> alias;

  std::deque<temp::Temp *> noSpillTemps;

};

} // namespace ra

#endif