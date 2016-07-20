// Copyright 2010-2014 Google
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_SAT_PRECEDENCES_H_
#define OR_TOOLS_SAT_PRECEDENCES_H_

#include <algorithm>
#include <queue>

#include "sat/integer.h"
#include "sat/model.h"
#include "sat/sat_base.h"
#include "sat/sat_solver.h"
#include "util/bitset.h"

namespace operations_research {
namespace sat {

// This class implement a propagator on simple inequalities between integer
// variables of the form (i1 + offset <= i2). The offset can be constant or
// given by the value of a third integer variable. Offsets can also be negative.
//
// The algorithm work by mapping the problem onto a graph where the edges carry
// the offset and the nodes correspond to one of the two bounds of an integer
// variable (lower_bound or -upper_bound). It then find the fixed point using an
// incremental variant of the Bellman-Ford(-Tarjan) algorithm.
//
// This is also known as an "integer difference logic theory" in the SMT world.
// Another word is "separation logic".
class PrecedencesPropagator : public Propagator {
 public:
  PrecedencesPropagator(IntegerTrail* integer_trail,
                        GenericLiteralWatcher* watcher)
      : Propagator("PrecedencesPropagator"),
        integer_trail_(integer_trail),
        watcher_(watcher),
        watcher_id_(watcher->Register(this)) {
    integer_trail_->RegisterWatcher(&modified_vars_);
  }

  static PrecedencesPropagator* CreateInModel(Model* model) {
    PrecedencesPropagator* precedences =
        new PrecedencesPropagator(model->GetOrCreate<IntegerTrail>(),
                                  model->GetOrCreate<GenericLiteralWatcher>());

    // TODO(user): Find a way to have more control on the order in which
    // the propagators are added.
    model->GetOrCreate<SatSolver>()->AddPropagator(
        std::unique_ptr<PrecedencesPropagator>(precedences));
    return precedences;
  }

  bool Propagate(Trail* trail) final;
  void Untrail(const Trail& trail, int trail_index) final;

  // Add a precedence relation (i1 + offset <= i2) between integer variables.
  void AddPrecedence(IntegerVariable i1, IntegerVariable i2);
  void AddPrecedenceWithOffset(IntegerVariable i1, IntegerVariable i2,
                               int offset);

  // Same as above, but the relation is only true when the given literal is.
  void AddConditionalPrecedence(IntegerVariable i1, IntegerVariable i2,
                                Literal l);
  void AddConditionalPrecedenceWithOffset(IntegerVariable i1,
                                          IntegerVariable i2, int offset,
                                          Literal l);

  // Note that we currently do not support marking a variable appearing as
  // an offset_var as optional (with MarkIntegerVariableAsOptional()). We could
  // give it a meaning (like the arcs are not propagated if it is optional), but
  // the code currently do not implement this.
  //
  // TODO(user): support optional offset_var?
  //
  // TODO(user): the variable offset should probably be tested more because
  // when I wrote this, I just had a couple of problems to test this on.
  void AddPrecedenceWithVariableOffset(IntegerVariable i1, IntegerVariable i2,
                                       LbVar offset_var);

  // An optional integer variable has a special behavior:
  // - If the bounds on i cross each other, then is_present must be false.
  // - It will only propagate any outgoing arcs if is_present is true.
  //
  // TODO(user): Accept a BinaryImplicationGraph* here, so that and arc
  // (tail -> head) can still propagate if tail.is_present => head.is_present.
  // Note that such propagation is only useful if the status of tail presence
  // is still undecided. Or at least propagate if both tail and head is_present
  // are the same literal.
  void MarkIntegerVariableAsOptional(IntegerVariable i, Literal is_present);

  // Finds all the LbVar that are "after" one of the LbVar in to_consider.
  // Returns a vector of these precedences relation sorted by
  // LbVarPrecedences.var so that it is efficient to find all the LbVar "before"
  // another one.
  //
  // Note that we only consider direct precedences here. Given our usage, it may
  // be better to compute the full reachability in the precedence graph, but in
  // pratice that may be too slow. On a good note, because we have all the
  // potential precedences between tasks in disjunctions, on a single machine,
  // both notion should be the same since we automatically work on the
  // transitive closure.
  //
  // Note that the LbVar in the vector are also returned in topological order
  // for a more efficient propagation in
  // DisjunctiveConstraint::PrecedencesPass() where this is used.
  struct LbVarPrecedences {
    int index;            // in to_consider.
    LbVar var;            // An LbVar that is >= to to_consider[index].
    LiteralIndex reason;  // The reaon for it to be >= or kNoLiteralIndex.

    // Only needed for testing.
    bool operator==(const LbVarPrecedences& o) const {
      return index == o.index && var == o.var && reason == o.reason;
    }
  };
  void ComputePrecedences(const std::vector<LbVar>& to_consider,
                          std::vector<LbVarPrecedences>* output);

 private:
  // Information about an individual arc.
  struct ArcInfo {
    LbVar tail_var;
    LbVar head_var;

    int offset;
    LbVar offset_var;         // kNoLbVar if none.
    LiteralIndex presence_l;  // kNoLiteralIndex if none.

    // Used temporarily by our implementation of the Bellman-Ford algorithm. It
    // should be false at the beginning of BellmanFordTarjan().
    mutable bool is_marked;
  };

  // Internal functions to add new precedence relations.
  void AdjustSizeFor(IntegerVariable i);
  void AddArc(IntegerVariable tail, IntegerVariable head, int offset,
              LbVar offset_var, LiteralIndex l);

  // Helper function for a slightly more readable code.
  LiteralIndex OptionalLiteralOf(LbVar var) const {
    return optional_literals_[IntegerVariableOf(var)];
  }

  // Enqueue a new lower bound for the variable arc.head_lb that was deduced
  // from the current value of arc.tail_lb and the offset of this arc.
  bool EnqueueAndCheck(const ArcInfo& arc, int new_head_lb, Trail* trail);
  bool PropagateMaxOffsetIfNeeded(const ArcInfo& arc, Trail* trail);
  int ArcOffset(const ArcInfo& arc) const;

  // Inspect all the optional arcs that needs inspection (to stay sparse) and
  // check if their presence literal can be propagated to false.
  void PropagateOptionalArcs(Trail* trail);

  // The core algorithm implementation is split in these functions. One must
  // first call InitializeBFQueueWithModifiedNodes() that will push all the
  // LbVar that have been modified since the last call. Then,
  // BellmanFordTarjan() will take care of all the propagation and returns false
  // in case of conflict. Internally, it uses DisassembleSubtree() which is the
  // Tarjan variant to detect a possible positive cycle. Before exiting, it will
  // call CleanUpMarkedArcsAndParents().
  //
  // The Tarjan version of the Bellam-Ford algorithm is really nice in our
  // context because it was really easy to make it incremental. Moreover, it
  // supports batch increment!
  //
  // This implementation is kind of unique because of our context and the fact
  // that it is incremental, but a good reference is "Negative-cycle detection
  // algorithms", Boris V. Cherkassky, Andrew V. Goldberg, 1996,
  // http://people.cs.nctu.edu.tw/~tjshen/doc/ne.pdf
  void InitializeBFQueueWithModifiedNodes();
  bool BellmanFordTarjan(Trail* trail);
  bool DisassembleSubtree(int source, int target, std::vector<bool>* can_be_skipped);
  void ReportPositiveCycle(int first_arc, Trail* trail);
  void CleanUpMarkedArcsAndParents();

  // Loops over all the arcs and verify that there is no propagation left.
  // This is only meant to be used in a DCHECK() and is not optimized.
  bool NoPropagationLeft(const Trail& trail) const;

  // External class needed to get the LbVar values and Enqueue new ones.
  IntegerTrail* integer_trail_;
  GenericLiteralWatcher* watcher_;
  int watcher_id_;

  // The key to our incrementality. This will be cleared once the propagation
  // is done, and automatically updated by the integer_trail_ with all the
  // LbVar that changed since the last clear.
  SparseBitset<LbVar> modified_vars_;

  // An arc needs to be inspected for propagation (i.e. is impacted) if:
  // - Its tail_var changed.
  // - Its offset_var changed.
  //
  // All the int are arc indices in the arcs_ vector.
  //
  // The first vector (impacted_arcs_) correspond to the arc currently present
  // whereas the second vector (impacted_potential_arcs_) list all the potential
  // arcs (the one not allways present) and is just used for propagation of the
  // arc presence literals.
  ITIVector<LbVar, std::vector<int>> impacted_arcs_;
  ITIVector<LbVar, std::vector<int>> impacted_potential_arcs_;

  // Temporary vectors used by ComputePrecedences().
  ITIVector<LbVar, int> lbvar_to_degree_;
  ITIVector<LbVar, int> lbvar_to_last_index_;
  struct SortedLbVar {
    LbVar var;
    int lower_bound;
    bool operator<(const SortedLbVar& other) const {
      return lower_bound < other.lower_bound;
    }
  };
  std::vector<SortedLbVar> tmp_sorted_lbvars_;
  std::vector<LbVarPrecedences> tmp_precedences_;

  // The set of arcs that must be added to impacted_arcs_ when a literal become
  // true.
  ITIVector<LiteralIndex, std::vector<int>> potential_arcs_;

  // Used for MarkIntegerVariableAsOptional(). The nodes associated to an
  // IntegerVariable whose entry is not kNoLiteralIndex will only propagate
  // something to its neighbors if the coresponding literal is assigned to true.
  ITIVector<IntegerVariable, LiteralIndex> optional_literals_;
  ITIVector<LiteralIndex, std::vector<int>> potential_nodes_;

  // TODO(user): rearranging the index so that the arc of the same node are
  // consecutive like in StaticGraph should have a big performance impact.
  std::vector<ArcInfo> arcs_;

  // Temp vectors to hold the reason of an assignment.
  std::vector<Literal> literal_reason_;
  std::vector<IntegerLiteral> integer_reason_;

  // Temp vectors for the Bellman-Ford algorithm. The graph in which this
  // algorithm works is in one to one correspondance with the LbVar in
  // impacted_arcs_.
  std::deque<int> bf_queue_;
  std::vector<bool> bf_in_queue_;
  std::vector<bool> bf_can_be_skipped_;
  std::vector<int> bf_parent_arc_of_;

  // Temp vector used by the tree traversal in DisassembleSubtree().
  std::vector<int> tmp_vector_;

  DISALLOW_COPY_AND_ASSIGN(PrecedencesPropagator);
};

// =============================================================================
// Implementation of the small API functions below.
// =============================================================================

inline void PrecedencesPropagator::AddPrecedence(IntegerVariable i1,
                                                 IntegerVariable i2) {
  AddArc(i1, i2, /*offset=*/0, /*offset_var=*/kNoLbVar, /*l=*/kNoLiteralIndex);
}

inline void PrecedencesPropagator::AddPrecedenceWithOffset(IntegerVariable i1,
                                                           IntegerVariable i2,
                                                           int offset) {
  AddArc(i1, i2, offset, /*offset_var=*/kNoLbVar, /*l=*/kNoLiteralIndex);
}

inline void PrecedencesPropagator::AddConditionalPrecedence(IntegerVariable i1,
                                                            IntegerVariable i2,
                                                            Literal l) {
  AddArc(i1, i2, /*offset=*/0, /*offset_var=*/kNoLbVar, l.Index());
}

inline void PrecedencesPropagator::AddConditionalPrecedenceWithOffset(
    IntegerVariable i1, IntegerVariable i2, int offset, Literal l) {
  AddArc(i1, i2, offset, /*offset_var=*/kNoLbVar, l.Index());
}

inline void PrecedencesPropagator::AddPrecedenceWithVariableOffset(
    IntegerVariable i1, IntegerVariable i2, LbVar offset_var) {
  AddArc(i1, i2, /*offset=*/0, offset_var, /*l=*/kNoLiteralIndex);
}

}  // namespace sat
}  // namespace operations_research

#endif  // OR_TOOLS_SAT_PRECEDENCES_H_