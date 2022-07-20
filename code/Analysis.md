# Implementation notes

## General matching: Micali-Vazirani

In `matching/micali_vazirani.hpp` we solve matching on non-bipartite graphs with MV.

Heavily optimized:

- `findpath` does not do depth first search
    - but it still constructs `list<int>` which could be improved
- no hashtables used
- no data structure required to hold a bloom's support
- efficient storage of phaselists and bridges
- two very good greedy maximal matching algorithms that run in O(V + E) time
    - node_bootstrap() generally yields a better maximal matching
    - edge_bootstrap() is simpler and also good
- extended search phases supported, tunable

### Breadth first search and MIN

For each node with min level equal to phase, find more nodes (previously unseen) and find
new bridges.

For each node u in the current phaselist:

- If the phase is odd, u is inner and we're looking for the matched edge out of u that
    goes to an unaugmented node through an unprocessed edge.
- If the phase is even, u is outer and we're looking for unmatched edges out of u that go
    to unaugmented nodes through unprocessed edges.

Let the target node be v, and let the edge from u to v be e.

- If the minlevel of the node v has not been set or is phase+1 already, the edge to it is
    a prop and is part of the primary bfs tree. A successor and predecessor relationship
    is recorded for the other subroutines, and v is added to the next phaselist.
- If the minlevel of v has been set and is <=phase then v is already in the tree and
    adding the edge e to it as a prop would form a back edge. Therefore e is not a prop
    and not part of the tree, hence it is a bridge.

For bridges:

- If the phase is odd then e is matched and is horizontal (i.e. the minlevel of v is phase
    and v is processed in the same iteration as u). The level of this bridge is known
    (lvl=phase) and so the bridge can be saved.
- If phase is even however, the edge e is unmatched and it is possible that the evenlevel
    of v has not yet been computed. In this case the tenacity/level of the bridge is
    unknown at this point. This happens because the even level of u is smaller than the
    even level path to v and the bridge is not horizontal, but actually oblique in the
    phase graph. This bridge is ignored and will be processed later, when the even level
    of v is set.

### Double depth first search and MAX

For a given bridge `e` with endpoints 'red' and 'blue', either find a bloom, or find an
augmenting path, or do nothing and exit immediately.

The idea is straightforward: we maintain two dfs "trails" instead of just one. The goal is
to arrive at two distinct exposed nodes and thus identify an augmenting path made up of
the concatenation of both trails, but this will be impossible in the presence of a bloom.

The two trails are colored red and blue, and along the ddfs the nodes visited are
appropriately colored. Predecessor relationships are stored, which maintain the trail data
attached to the nodes.

The trails cannot enter into nodes visited by the other trail, identified by their color,
even if the other trail has meanwhile backtracked out of that node. This guarantees the
two trails never intersect. In practice, the implementation maintains two trail heads `r`
and `b`, and the parent trail pointers are attached to the nodes as they are visited. The
two trail heads may collide, at which point backtracking is performed on `b` first by
convention.

Along the ddfs we may need to backtrack along one of the two trails. Being optimistic,
while advancing we can just take predecessor edges at will and hope we don't collide the
two trails, and then "backtrack" one of the two trails in case of collision, by convention
`b`. If the first backtracking fails, we try the second trail, and if that fails we found
a "barrier" node that is a common ancestor of all the nodes visited by the ddfs (and the
ones skipped over). Indeed all prop edges emanating from this barrier node are visited, to
ensure there is no way around it. Backtracking succeeds if the trail can get "around" the
collision point, and fails otherwise. Getting "around" means finding another reachable
node at the same or lower depth than the collision point. In case of failure the trail
head is commited to the collision point and its other trail's turn to backtrack.

To backtrack efficiently we maintain an arc index attached to each of the nodes, which
identify the next predecessor to take at that node after backtracking to it along a trail.
If the index is invalid, there are no more predecessors and further backtracking is
needed.

#### Saved data

The augmenting path subroutine will need to reuse the colored trails found during ddfs,
going backwards from the found exposed nodes. The blooms formed also maintain predecessor
(petal) and successor (trail) node relationships. Therefore to traverse the bloom we just
need to follow the pointers, and no dfs is required.

The bloom formation subroutine will need to perform dfs again (this time with no major
fuss) to assign maxlevels, bloom ids and identify new bridges. It will also need to form
two primary colored petals starting from the base.

Nothing is done by ddfs if red and blue were already in blooms with the same base.

Observation 1 (correctness optimization)

-   A node Q is unerased iff it has not been used in an augmenting path and there is still
    a path from it to an exposed vertex.
-   After augmentations, function `erase_successors()` erases nodes that have become
    unreachable from exposed vertices. Our ddfs implementation requires this procedure to
    be implemented properly - when advancing into a node u, there must be some path from
    that node to an exposed node.
-   The predecessors of a node we're going to advance to can be stored in arbitrary order.
    As such, as we iterate over the predecessor array of a node u, we can eliminate nodes
    that have been erased by swapping them with the last element.

#### Data structures

-   Three pointers: `r` (red, left), `b` (blue, right), `barrier` (for b).
-   One arc index per node
-   Four trail pointers per node, two for each color.
-   Given a node u, if the trail head is at u and a predecessor edge to v is taken, and
    w=base\*(v) is the next trail head, then we set

```
    w.trail.hi := u,   w.trail.lo := v.   w.arc := index of next(v)
```

Colored trail pointers are necessary for the bloom bases.

#### Algorithm

-   Advance whichever of r and b is further from the roots (i.e. largest min_level)
    advancing r in case of a tie.
    -   When advancing into nodes that belong to blooms skip directly to the bloom's base
        star (recursive base).
    -   Be careful not to advance into nodes that have been erased (by a previous
        augmentation in the same search) (observation 2)
    -   Notice that advancing casually (i.e. not while backtracking) only one step at a
        time guarantees the two paths can only cross at their endpoints (when `r=b`) and
        not somewhere in between. This is not necessarily true while backtracking either r
        or b as we usually need to backtrack and readvance several steps.
-   If `r=b` meet at the same node, this node is a bottleneck.
    -   Backtrack b first. Repeat, until there are no more nodes to search or the trail
        has gone around the bottleneck:
        -   Pop nodes from the trail until one has more predecessor nodes to go to, or
            until we reach the barrier.
        -   Advance the trail greedily along predecessor nodes, until the head collides
            with the other trail, revisits an old node, or goes around the trail.
        -   Backtracking fails when the head is equal to the barrier and there are no more
            predecessor edges; otherwise it succeeds. In this case, set barrier to this
            bottleneck and backtrack r.
-   If at any point both `r!=b` have no predecessors, i.e. they are exposed roots, we are
    done and found an augmenting path.
-   If neither trail can get around the bottleneck, the bottleneck is the base of a new
    bloom.

### Bloom formation and coloring

              red  (blue)          walk examples, without the expanded blooms:
            ____R  (B)___
       ____/   / \  |\   \___             - walk_peak (using bloom succ):
      /       /   \ | \      \            p->i->e->B       (blue)
     /       /     \|  \      \           q->x->c->B       (blue)
    a       b      (c) (d)    (e)         r->n->h->b->R     red
    |\_   _/|\       \ / \___  |
    |  \_/  | \___   (x)     \ |          - walk_base (using bloom pred):
    |__/ \__|     \  / \      \|          R->b->h->n->t     red
    f       g      h/   \     (i)         B->c->x->q       (blue)
    |      /|    _/|\    \     |
    |     / |  _/ /| \    \    |          - walk_down (using whatever):
    |    /  |_/  / |  \    \   |          g->l->q
    j   k   l   /  m   n   (o)(p)         a->f->j->q
     \   \  |  /   |  /|  _/  /           d->i->p->t
      \   \ | /   _|_/ | /   /
       \___\|/   / |   |/___/         bloom succ of b: R
           (q)  r  s   t              bloom succ of c: B
             \  |  |  /               bloom succ of i: d or e, doesn't matter
              \ |  | /                bloom pred of h: n
               [base]                 bloom pred of c: x

#### After finding a bloom

If a bloom is found we identify:

-   the peak bridge (e)
-   the base (barrier)

All nodes reachable from the peak through predecessor edges up to the barrier have been
visited and properly colored. All these nodes constitute the bloom. Every node u in this
bloom has been colored red or blue such that: a) If u is red there is a red path from u to
the red peak node. If u is blue there is a blue path from u to the blue peak node. b)
There is a red path from the red peak to the base (primary red petal). There is a blue
path from the blue peak to the base (primary blue petal). c) If u is red there isn't
necessarily a red path from u to the base. If u is blue there isn't necessarily a blue
path from u to the base.

a) This invariant allow us to find a path from any node u to a peak using only one color.
If an augmenting path is found through the bloom that needs to go "around" the bloom, this
path is used to go from the first node to the peak, making sure to use only one color as
another disjoint path going down must be found too. We optimistically assume such an
augmenting path will be found, so to facilitate its construction, we assign bloom
successors to every node alongside their maxlevels. If `P(u)` is a path of u's color from
u to the peak, then the bloom successor of u is the "guide" prop edge e that leads to the
second node in `P(u)`. Suppose ddfs, to reach u, went from vertex v through edge e,
pointing to w. Then `u=base*(w)` and the successor prop of u is edge e. The second node in
`P(u)` is v, before expanding blooms. Storing the edge e instead of v allows us to know
the vertex w immediately to expand the blooms up to u.

b) These paths correspond to the red and blue trails constructed during ddfs. We didn't
store these, but we can recover them quickly with a naive dfs following predecessors of
the same color. If an augmenting path is found through the bloom that needs to go "around"
the bloom, this path is used to go down from the peak to the base, making sure to use the
opposite color of that used to go from the first node to the peak. To identify this
"primary petal" we assign bloom predecessors to the nodes on it. Only one such petal is
constructed for each color.

c) This happens because the ddfs might backtrack successfully more than one node, leaving
for example a blue node with only red predecessor nodes. Such nodes were removed from the
trail but are still part of the bloom. If an augmenting path is found through the bloom
that needs to "descend" the bloom directly, i.e. not go "around" it, then the colors don't
matter as only a single subpath through the bloom is needed, not two disjoint ones.

Be careful not to touch the bloom's base in any way.

#### Algorithm

-   To form the primary red and blue petals, walk the trail pointers starting at the base.
    Both color trails have been filled.
-   Perform dfs to assign maxlevels, bloom ids, and identify bridges.
    -   Visit only pred nodes of the same color.
    -   Avoid revisiting nodes reachable in multiple ways.
    -   For bloom nodes with even maxlevel, look for bridges out of the bloom.

### Path augmentation and bloom expansion

    a
     \
      \  5~ ~ ~6
       \ |     |    b            going down the bloom:
        \|     |   /               a-->3-->1-->base
         3     4  /                   \_____/
         ~     ~ /                   walk_down()
         ~     ~/
         1     2                 going around the bloom:
          \   /                    b-->2-->4-->6-->5-->3-->1-->base
           \ /                        \_________/ \_________/
           base                        walk_peak() walk_base()

#### After finding an augmenting path

If an augmenting path is found we identify: - the peak bridge `e` - the red and blue
trails, properly colored, including exposed nodes the trails can be walked starting at the
exposed nodes

Each trail red/blue identifies a sequence of nodes `t=u(0),u(1),u(2),...,u(n)` such that t
is the red/blue peak and u(n) is an exposed vertex. The two sequences are disjoint by
construction.

#### Algorithm

-   For each trail red/blue with color c and top t:
-   Expand the initial bloom-star of t, if it exists.
-   Walk the trail, starting at u(n). Expand the bloom-star contracted in
    `[trail[u(i)][c].lo, u(i)]`, then append u(i), for `i=n,n-1,...,1`, and finally append
    t.
-   Be careful to maintain the path's correct direction throughout.
-   Join the two expanded paths.
-   Augment the constructed path by inverting all edges along it.
-   Erase all nodes in the path and all of their successors who can no longer be reached
    by any exposed node, recursively.

#### Node erasure

-   Using lazy erasure, erase only the first element of the successor's pred list until it
    is an unerased node. If the pred list becomes empty as a result the successor is to be
    erased as well.
-   Use lazy erasure also in ddfs to avoid entering erased nodes.

---

## Link Cut Tree

We have link cut trees in `struct/link_cut_tree_path.hpp` and
`struct/link_cut_tree_subtree.hpp`.

The first type aggregates on paths, and implements the operations *set-node-update*,
*lazy-add-path-update* and *path-sum-query*.

The second type aggregates on subtrees, and implements the operations *set-node-update*
and *subtree-sum-query*.

Both should be straightforward to modify and augment, and also support other operations
such as `lca(u,v)`, `path_length(u,v)` and `subtree_size(u,v)`.

Both LCTs are **unrooted**, meaning interface operations will reroot represented subtrees
for their own convenience. For meaningful `lca` queries the subtree should be rooted
beforehand.

We follow the left-hand rule in the implementation: going right in an internal splay tree
is going down in the represented tree (by default).

Nodes are numbered 1 to N. Node 0 is the head sentinel and should be filled with neutral
values (often not the default value for other nodes).

### Lazy propagation and rerooting - Implementation notes

To support tree rerooting we maintain a lazy flip boolean on each node. This is pretty
standard but can be difficult to grasp at first.

Notice that when we reroot a tree previously rooted at $r$ on a node $u$, we flip exactly
those edges on the *path from $u$ to $r$ in the represented tree*, i.e. the operation
needs to be propagated on the splay tree that is formed by connecting exactly these two
nodes.

To support this `reroot` operation and coincidentally also the other queries conveniently,
we need to make a setup where the virtual tree's root splay tree is made up of exactly
those nodes from $r$ to $u$, and $u$ is the splay root. This is precisely what `access()`
does.

To reroot the tree we now invert the binary tree below $u$, lazily.

### Interface

We support all of the following. Below *sum* should be understood as *aggregate*.

- Link $O(\log N)$: `link(u, v)`
    - Standard link-cut operation. $u$ is the root of a tree and $v$ is a node.
    - Set the parent of $u$ to $v$.
- Cut $O(\log N)$: `cut(u, v)`
    - Standard link-cut operation: $u$ and $v$ are nodes and $v$ is the parent of $u$.
    - Remove the link from $u$ to $v$ and let $u$ become a new tree root.
- Point updates $O(1)$: `update_node(u, new_value)`
    - Change the value of $u$ to `new_value`.
- Subtree queries $O(\log N)$: `query_subtree(u)`
    - Compute aggregate of `value(w)` for every node $w$ in $u$'s subtree, inclusive.
- Subtree updates $O(\log N)$: `update_subtree(u, increment)`
    - Update lazily `value(w)` for every node $w$ in $u$'s subtree, inclusive.
- Subtree size $O(\log N)$: `subtree_size(u)`
    - Compute the size of $u$'s subtree, inclusive.
- Path queries $O(\log N)$: `query_path(u, v)`
    - Compute aggregate of `value(w)` from $u$ to $v$, where $v$ is an ancestor of $u$.
- Path updates $O(\log N)$: `update_path(u, v, increment)`
    - Update lazily `value(w)` from $u$ to $v$, where $v$ is ancestor of $u$.
- Path length $O(\log N)$: `path_length(u, v)`
    - Compute the length of the path from $u$ to $v$, where $v$ is an ancestor of $u$.
- Reroot $O(\log N)$: `reroot(u)`
    - Make $u$ the new root of its tree, *flipping* the orientation of all parent edges.
- Lowest common ancestor $O(\log N)$: `lca(u, v)`
    - Compute the lowest common ancestor of two nodes $u$ and $v$ (possibly inexistent).

---

## FFT and NTT

Designing this fucking shit is *hard*. Still working on improving constant factors.
We produce an FFT library tuned for polynomial multiplication with high level operations
for multiply, transform and inverse transform.

We want the library to handle a lot of types.

Consider FFT multiplication. We want:

- `vector<T> x vector<T> -> vector<T>`
  - `T=int` (no overflows)
  - `I=long` (no overflows)
  - `I=uint` (with wrapping)
  - `I=double` (assumed safe)
  - `I=complex` (assumed safe)
  - `I=modnum<PRIME>` (primitive root will be computed)

Consider NTT multiplication. We want:

- `vector<I> x vector<I> (modulo MOD) -> vector<O>`
  - `I=modnum<PRIME>` (NTT; no overflows)
  - `I=int, runtime MOD` (SPLIT FFT; no overflows)
  - `I=long, runtime MOD` (SPLIT FFT; no overflows)
  - `I=uint, runtime MOD` (SPLIT FFT; with wrapping)
  - `I=modnum<PRIME>` (primitive root will be computed)

We do not handle *two inputs* of different types or input and output of different types.
This helps simplify the implementation a bit, as it is already template-heavy.

---

## Divide and conquer 3D hull

Here is our goal:
- Implement the algorithm in the algebraic comparison model, with no numeric calculations
  in the integer coordinates case.
- Handle collinear points and coplanar points, computing 1D and 2D hulls correctly also.
- Do not check for null cross products or collinear points explicitly anywhere.
- With 64-bit integers, support coordinates up to 5e8 or so. Requires:
  - Must use 128-bit dot products
  - Do not compute double cross products (i.e. you can cross at most once)

Here is our simplification:
- Faces are not triangulated, and face-interior or edge-interior points are forgotten.

So, dividing the points in two halfs and solving recursively, then merging, we want
the two hulls to not overlap.

We can sort the points lexicographically (there are no coincident points) and this
property is guaranteed.

### Finding the supporting vertices and first supporting bridge

Pick some huge α (e.g. 10^10000).
Sorting the points lexicographically is essentially equivalent to sorting them
along the direction of SORT=(α²,α,1) with a plane sweep. There are no ties.

To find the supporting line when merging the two hulls, we want to project to 2D and use
the same trick we used for delaunay triangulation.
Pick some vector SUPP perpendicular to SORT. If we project all the points along SUPP
there are no ties (i.e. no coincident points) because the plane sweep also had no ties.

We can pick SUPP=(-2,α,α²).
For an arbitrary vector u=(x,y,z), dot(u,SUPP) = α²z + αy - 2x
To determine whether this is negative/positive/zero, we simply perform a lexicographical
comparison of v=(z,y,-x) with (0,0,0). Clearly, the dot is zero iff u=(0,0,0).

Given a fixed point p and two vectors u and v out of p, we can sort them around SUPP
by evaluating dot(cross(p,u,v),SUPP). With this we can walk around the projected 2D hull
and find the supporting vertices.

Just like in delaunay, after finding the supporting vertices we "twist" the hanging edges
so they point 'inwards' between the hulls.

Unlike delaunay we must skip collinear edges. Suppose we have a 1d merge like
`b -- a    d -- c` the supporting edges are A=ba and D=cd. We advance collinear supporting
edges but prevent advancing if the edge's vertex is already the leftmost on A or rightmost
on D (we can check this with ranks).

### Wrapping

Now, the main wrapping loop is very scary for several reasons:

- It is not immediately clear how it will handle degenerate cases (1D and 2D hulls).
- It is not immediately clear when we should stop.
- It is not immediately clear how to merge coplanar faces in the two hulls.

Let's focus on the second problem. The answer is:

- We're done when A and D are the same edge (A == D->mate == the first supporting E edge)
- We call this event the **crossing**.

Here's the intuition. Picture a 3D/3D->3D hull merge.
You've added the first supporting edge E and now you start wrapping.
The edges A and D keep moving forward along the horizon on their own sides.
Eventually they loop back to the first edge, and there they will try to 'cross' this edge.
If we're careful with our advancing/cutting/retreating predicates, they will meet on E.
Clearly, we should stop then (and not add the last bridge over E).

Now picture a 2D/2D->2D hull merge.
You'll add the first low E, then wrap until the second high E' (which you add).
Then wrap back down on the opposite face to low E.
A and D will both try to cross E, and there you stop.
You've added two bridges, as you wanted.

Now picture a 1D/1D->1D hull merge. You'll add the first E (which is the entire hull).
After you cut the dangling edges, you get A == D->mate == E and you're done.
You've added one bridge, as you wanted, and you never advanced.

Now picture a large 1D/2D->3D hull merge. On the left you've got a single edge that will
get 'engulfed' by the right hull. Initially you just add one E. You'll rotate and keep
adding more edges without cutting the one single edge. Once D wraps around completely,
A and D will be coplanar but A!=D->mate and you will cut A because it lies below the
plane, and then you're done.

### More gotchas

When merging two 2D hulls (or two coplanar faces), because you merge by 'going around'
instead of 'going inside' like in D&C triangulation, at some point you might realize
one of the sides went too far ahead and needs to 'back off' as the merged face would
otherwise become concave.

When advancing along a face and cutting backward edges, remember to check they are still
coplanar! While you're merging the hull *the points on a face are not coplanar*.

You need to 'rotate upwards' before you add the initial E, and then also just before you
add any other E after. For example, when you're wrapping 3D/3D back to the first E, you're
not supposed to add any more edges, but if you don't rotate upwards you will miss the
crossing event because of some dangling edges that need to be cut, and you'll add one edge
too many.

When A and D are coplanar, be mindful that one of the edges might be collinear with the
current bridge, so that the coplanarity is in some sense 'degenerate'. This is definitely
the case in a 1D/1D->1D hull merge! To handle this without checking cross products,
just pick the one `up` vector with the largest norm (or their sum).

If possible, don't go about explicitly removing the engulfed edges because that is very
complicated. Instead, just put them in a pool and release the entire pool afterwards.
if you at least reuse the edges you explicitly cut the memory complexity isn't too bad,
because the edges you've forgotten are tied to *vertices you've forgotten*.

Once you've reached the crossing you might have to cut some extra edges from one side
in a way that **doesn't follow the convexity invariants** the previous cuts followed,
simply because ones of the edges has crossed over and broken convexity for its side.

Notice there are merges that don't cut any edges! So if you ever have an unconditional cut
the merge code *must* be wrong.

That doesn't mean good code should go about rushing to these events.

### Debugging

I debugged this crap as follows:

Have a `hull_debug` that keeps track of the hull after each merge/base-case, maintaining
a history of these versions. Then dump all the hulls into an OBJ file, with each version's
hulls shifted over along y by some value. Then load it in blender to inspect.

Write ~20 unit test cases with easy sensible test cases, then very degenerate edge cases
such as 1D/1D->1D, 2D/2D->2D with collinear points around the bridges, 1D/2D->3D merges,
1D/3D->3D with 1D fully engulfed, 2D/2D->2D with no cuts, 3D/3D->3D with no cuts,
3D cube and hypercube for quad faces only, 1D/1D->3D tetrahedron, 1D/2D->2D collinear.

Test each case with coordinates rotated to verify the supporting line projection is
working properly. When stable write the full quadratic hull verifier.

Test on the following generated distributions:

- Cube surface (only on the faces, edges and corners)
- Cube wireframe (only on the edges and corners)
- Sphere surface (all points on the hull, almost all triangles)
- Elliptic paraboloid (all points on the hull, delaunay reduction)

---

## Simplex

Consider a linear program with $N=3$ variables and $M=2$ constraints.
Adding dual (slack) variables we can write it as follows:

$$\text{Primal Program}\\
\begin{matrix}
\max & c_1x_1 &+& c_2x_2 &+& c_3x_3 && \\
(1) & a_{11}x_1 &+& a_{12}x_2 &+& a_{13}x_3 &+& y_1 &=& b_1 \\
(2) & a_{21}x_1 &+& a_{22}x_2 &+& a_{23}x_3 &+& y_2 &=& b_2
\end{matrix}$$

We don't make any assumptions on $A$, $B$ or $C$, but we do assume $N>0$ and $M>0$.

There are $N$ **primal variables** $x_1,\dots,x_N$ and $M$ **dual variables** $y_1,\dots,y_M$.
Initially all dual variables are **basic** and all primal variables are **non-basic**.
Essentially this means $x_j=0$ and $y_i=b_i$ for each $i,j$.
In the $M$ equations above, every monomial except the last one with coefficient 1 has
value $0$, so the equation reads off as $y_i=b_i$.

If we enforce that all non-basic variables have value $0$ and that each basic variable
appears in exactly one row on the basis column, the program is just one gigantic linear
system, and the values of all basic variables are completely determined.

The coefficients in the objective row let us see how *increasing* the value of a non-basic
variable (by 1) will change the optimum value. If this coefficient is positive the optimum
increases, otherwise it decreases. This is true *by definition* at the beginning, and
will be kept so after pivoting.

After a bunch of pivots, the **potential** $\phi$ of the tableau/program is equal to the number
of basic primal variables, which is equal to the number of non-basic dual variables.
This is always $\leq\min(N,M)$, starts at $0$, and each pivot changes this by at most 1.

Denote by $T$ the average time it takes to pivot.

### Basic duality theory

The dual program is the following, *by definition*:

$$\text{Dual Program}\\
\begin{matrix}
\min & b_1y_1 &+& b_2y_2  && \\
(1) & a_{11}y_1 &+& a_{21}y_2 &-& x_1 &=& c_1 \\
(2) & a_{12}y_1 &+& a_{22}y_2 &-& x_2 &=& c_2 \\
(3) & a_{13}y_1 &+& a_{23}y_2 &-& x_3 &=& c_3
\end{matrix}$$

Note the minus signs on the basic variables! We could flip all coefficient signs in this
program and get another $\max$ program.

Linear programming duality essentially asserts the following:

- Primal feasibility is equivalent to dual optimality
- Dual feasibility is equivalent to primal optimality
- Weak duality: A primal feasible solution is a lower bound on the dual optimal solution
- Weak duality: A dual feasible solution is an upper bound on the primal optimal solution
- Strong duality: The primal and dual optimums are the same

What does this mean?

- **Primal feasibility** means that $b_i\geq 0$ for all rows $i$. In other words, all basic
variables in the primal have non-negative values.

- **Primal optimality** means that $c_i\leq 0$ for all columns $i$. In other words, pivoting
on any non-basic variable in the primal will not increase the value of the primal optimal solution.

- **Dual feasibility** means that $c_j\leq 0$ for all rows $j$. In other words, all
basic variables in the dual have non-negative values.

- **Dual optimality** means that $b_i\geq 0$ for all columns $i$. In other words, pivoting
on any non-basic variable in the dual will not decrease the value of the optimal dual solution.

Recall that by convexity, any local optimum of an LP is a global optimum.

The first two claims for LP duality follow immediately from the definition.

The trick of primal-dual simplex is to exploit this symmetry. You can think of it any way
you like:

- We will always pivot with the goal of reaching primal feasibility or dual feasibility. By duality theory we "solve" the program.
- We will always pivot with the goal of reaching primal optimality or dual optimality. By duality theory we "solve" the program.

In theory it wont't even matter which one we work towards first!

Best way seems to be to think of reaching optimality (in the dual, then the primal), by
performing pivots where you pick a column first based on the objective row, then a feasible
row to leave the basis based on the value column.

### Primal pivots

Suppose $x_1$ is to enter the basis and $y_1$ is to leave. In the first equation above,
we want $x_1$ to go to the basis column, and $y_1$ to take $x_1$'s place in the first
column. We can divide row (1) by $a_{11}$:

$$\begin{matrix}
\max & c_1x_1 &+& c_2x_2 &+& c_3x_3 && \\
(1) & x_1 &+& \frac{a_{12}}{a_{11}}x_2 &+& \frac{a_{13}}{a_{11}}x_3 &+& \frac{1}{a_{11}}y_1 &=& \frac{b_1}{a_{11}} \\
(2) & a_{21}x_1 &+& a_{22}x_2 &+& a_{23}x_3 &+& y_2 &=& b_2
\end{matrix}$$

$$\begin{matrix}
\max & c_1x_1 &+& c_2x_2 &+& c_3x_3 && \\
(1) & \frac{1}{a_{11}}y_1 &+& \frac{a_{12}}{a_{11}}x_2 &+& \frac{a_{13}}{a_{11}}x_3 &+& x_1 &=& \frac{b_1}{a_{11}} \\
(2) & a_{21}x_1 &+& a_{22}x_2 &+& a_{23}x_3 &+& y_2 &=& b_2
\end{matrix}$$

Now substitute for $x_1$ in the coefficients row and row (2):

$$\begin{matrix}
\max & c_1(\frac{b_1}{a_{11}}-\frac{1}{a_{11}}y_1 - \frac{a_{12}}{a_{11}}x_2 - \frac{a_{13}}{a_{11}}x_3) &+& c_2x_2 &+& c_3x_3 && \\
(1) & \frac{1}{a_{11}}y_1 &+& \frac{a_{12}}{a_{11}}x_2 &+& \frac{a_{13}}{a_{11}}x_3 &+& x_1 &=& \frac{b_1}{a_{11}} \\
(2) & a_{21}(\frac{b_1}{a_{11}}-\frac{1}{a_{11}}y_1 - \frac{a_{12}}{a_{11}}x_2 - \frac{a_{13}}{a_{11}}x_3) &+& a_{22}x_2 &+& a_{23}x_3 &+& y_2 &=& b_2
\end{matrix}$$

Expanding gives

$$\begin{matrix}
\max & -\frac{c_1}{a_{11}}y_1 &+& (c_2-\frac{c_1a_{12}}{a_{11}})x_2 &+& (c_3-\frac{c_1a_{13}}{a_{11}})x_3 &+& \frac{c_1b_1}{a_{11}} \\
(1) & \frac{1}{a_{11}}y_1 &+& \frac{a_{12}}{a_{11}}x_2 &+& \frac{a_{13}}{a_{11}}x_3 &+& x_1 &=& \frac{b_1}{a_{11}} \\
(2) & -\frac{a_{21}}{a_{11}}y_1 &+& (a_{22}- \frac{a_{21}a_{12}}{a_{11}})x_2 &+& (a_{23}- \frac{a_{21}a_{13}}{a_{11}})x_3 &+& y_2 &=& b_2-\frac{b_1a_{21}}{a_{11}}
\end{matrix}$$

That is how we pivot and maintain a tableau with the same shape.

Because all our variables should be non-negative - **primal feasibility**, we *should* have
$$y_2=b_2-\frac{b_1a_{21}}{a_{11}}\geq 0\Leftrightarrow \frac{b_2}{a_{21}}\geq\frac{b_1}{a_{11}}$$
If we *chose* to enter variable $x_1$, we should pick for the leaving variable $y_i$ the
one which minimizes $\frac{b_i}{a_{i1}}$, so we can maintain primal feasibility.
We do this *even if* we actually don't yet have primal feasibility!

The new value of $x_1$ is $\frac{b_1}{a_{11}}$, and the coefficient of $x_1$ was $c_1$.
So our optimum value increased by $\frac{c_1b_1}{a_{11}}$.

To work towards **primal optimality** we should pick $c_1>0$. There are many pivot rules:

- We can check the entire tableau and pick the column variable
$\text{argmax}_j\left(c_j\min_i\frac{b_i}{a_{ij}}\right)$.
  - We'll call this **largest-immediate-gain**, $O(N+AM)$.
- We can also just pick the largest $c_i$ then find the corresponding $j$.
  - This is **dantzig-rule**, $O(N+M)$.
- We can group the non-basic variables in blocks of size $B$ and before each pivot run either
  of the previous rules on each block, until we find a valid entering variable. The blocks
  are searched in cyclic order. A decent pick may be $B=\sqrt N$ or $B=N^{2/3}$. This "smooths" out the program.
  - This is **block-search**, $O(B+AM)$ or $O(B+M)$ on average.
- There are many other heuristics one can use, like backing off variables that get pivoted
  too often (keep them out for a while), mixing rules, pricing variables, etc.

Where $A$ is the number of non-basic variables with positive objective row ($c_i>0$).
The second one can be exploited by e.g. Klee-Minty cube, and the first one by other stuff.
The third one is slow in general.

### Dual pivots

Dual pivots are the same thing in the dual program. The code is *literally* the exact same thing,
and the *exact* same change occurs to the optimum value. This is the gist of the proof
of strong duality (based on simplex).

It is not at all obvious at a glance that the transformation is equivalent, however.
This is why strong duality is not trivial.

### Impossibility and unboundedness

By weak duality, the primal is infeasible if the dual is unbounded, and vice-versa.

#### 1. Conditional limits

Suppose we are pivoting the primal for optimality, already having primal feasibility.
Then we have $b_i\geq 0$ for all rows $i$. This is the case if we began by pivoting the
dual for optimality, or if the initial tableau was just feasible right away.

After a pivot on column $j$, row $i$, a random $b_k$ coefficient becomes $b_k-\frac{c_jb_i}{a_{ij}}$.

Using the **steepest-edge** rule on column $j$ and $c_j>0$.
If for all rows $i$ we have $a_{ji}\leq 0$
then there is no upper bound on the entering variable: all the $b_k$ coefficients will
*not decrease*, so we can make the new basic variable as large as we want
and after the pivot we *still* have primal feasibility. This just means that the primal
is **unbounded**, and therefore the dual is **infeasible**.

A similar argument would apply when pivoting the dual for optimality, already having
dual feasibility. Suppose we have $c_j\leq 0$ for all $j$.
If we have a dual column $i$ such that $b_i<0$ and for all $j$, $a_{ji}\leq 0$, then
there is no upper bound to how much we can increase the variable in column $i$. Therefore
the dual is **unbounded**, and therefore the primal is **infeasible**.

#### 2. Unconditional limits

Now we argue that the feasibility assumptions that we made in the previous section
are actually not necessary, and the same results hold regardless of it.
The crucial idea is that *feasibility does not depend on the objective row*.

Suppose we are pivoting the dual for optimality, and do not yet have dual feasibility.
This is equivalent to *pivoting the primal for feasibility*. But in this case we do
not really care about the primal's objective row column. So we can make this primal objective row
whatever we want, in particular we can just make it all negative, so that we automatically
get primal optimality and therefore dual feasibility. Then the argument in the previous
section holds.

Note: This is circular if the proof of strong duality depends on the first section.

### Adding a primal constraint

Suppose we have some tableau, with $0$ or positive potential $\phi$, optimized or not, and wish
to add a new primal constraint/dual variable.

We create a new dual variable $y_{M+1}$ for it, which will start off **basic**.

If we know *how* to express the new constraint in terms of $y_{M+1}$ and the current non-basic
variables we can just add it to the tableau directly.
But usually we have the constraint expressed in terms of the primal variables, some of which
are basic and should not appear in this new constraint row:

For example:

$$\text{Current tableau}\quad
\begin{matrix}
\max & c_1y_1 &+& c_2x_2 &+& c_3y_2 &&&& \text{optimum} \\
(1) & a_{11}y_1 &+& a_{12}x_2 &+& a_{13}y_2 &+& x_1 &=& b_1 \\
(2) & a_{21}y_1 &+& a_{22}x_2 &+& a_{23}y_2 &+& x_3 &=& b_2
\end{matrix}$$

$$\text{Want to add:}\quad
\begin{matrix}
(3) & d_1x_1 &+& d_2x_2 &+& d_3x_3 &+& y_3 &=& b_3
\end{matrix}$$

The algorithm is simple, simply substitute $x_1$ and $x_3$ into the constraint's left
hand side. We get:

$$d_1x_1 + d_2x_2 + d_3x_3 + y_3 = b_3 \Leftrightarrow$$

$$d_1(b_1 - a_{11}y_1 - a_{12}x_2 - a_{13}y_2) + d_2x_2 + d_3(b_2 - a_{21}y_1 - a_{22}x_2 - a_{23}y_2) + y_3 = b_3 \Leftrightarrow$$

$$(-d_1a_{11}-d_3a_{21})y_1 + (d_2-d_1a_{12}-d_3a_{22})x_2 + (-d_1a_{13}-d_3a_{23})y_2 + y_3 = b_3 - d_1b_1-d_3b_2 \Leftrightarrow$$

To do this efficiently we maintain the rows/columns position for each $x_j$ and $y_i$
and vice-versa as we pivot the tableau. In total this takes $O(n\phi)$ time to insert
the constraint.

If the initial tableau was primal optimal then so is the new one. But primal feasibility
is maintained iff the value of $y_{M+1}$, $b_{M+1}$, is non-negative.

### Adding a primal variable

Just add the equivalent constraint to the *dual program*. The process is very similar,
but because the basic variables are negated in the dual program's row, the monomial $-d_ja_{ij}$
above has its sign flipped to $d_ja_{ij}$. The primal variable added is $x_{N+1}$ and
starts off non-basic. In total this takes $O(m\phi)$ time to insert the variable.

If the initial tableau was primal feasible then so is the new one. But primal optimality
is maintained iff the objective row coefficient of $x_{N+1}$, $c_{N+1}$, is non-positive.

### Removing a primal/dual variable/constraint

To remove a primal variable, if it is basic then pivot it out (arbitrarily).
Then just remove its column in the resulting tableau, and reoptimize.

To remove a dual variable, if it is non-basic then pivot it in (arbitrarily).
Then just remove its row in the resulting tableau, and reoptimize.

### Updating a primal constraint bound

Consider the primal constraint $a_{i1}x_1+\dots+a_{iN}x_N+y_i=b_i$. We wish to
modify the **bound** from $b_i$ to $b_i+\triangle b$.

Look at the pivot formula in the previous sections. The bound vector $b$ does not affect
the dual prices $A$ of the main tableau. So if the variable $y_i$ is basic
we can just update $b_i$ directly, in $O(1)$.
The optimum of the tableau does not change.

If the variable $y_i$ is non-basic, consider some row $r$ in which we have

$$\dots+a_{ri}y_i+\dots+z_r=b_r$$

where $z_r$ is the row's basic variable, which may be a primal or dual variable.
Consider pivoting $y_i$ into this row, we get

$$\dots+\frac{1}{a_{ri}}z_r+\dots+y_i=\frac{b_r}{a_{ri}}$$

Now perform the $\triangle b$ update for basic $y_i$:

$$\dots+\frac{1}{a_{ri}}z_r+\dots+y_i=\frac{b_r}{a_{ri}}+\triangle b$$

Now undo the pivot:

$$\dots+a_{ri}y_i+\dots+z_r=a_{ri}(\frac{b_r}{a_{ri}}+\triangle b)=b_r+a_{ri}\triangle b$$

Alternatively, think of the above transform as follows. The $y_i$ was non-basic, so the
constraint was *tight*. We changed the *slack* of constraint by $\triangle b$, so the
slack became $\triangle b$. We push this slack back down to $0$ in *every* row
of the tableau by replacing $y_i$ by $y_i-\triangle b$ and we get the same formula as above
for the new right bounds.

What about the optimum? Well $y_i$ actually **decreased** by $\triangle b$ after this
adjustment, and the objective row coefficient $c$ of $y_i$ says that a unit increase in
$y_i$ increases the optimum by $c$. So the optimum decreased by $c\triangle b$.

### Updating a primal variable coefficient

Just update a dual constraint in the dual tableau. The code is dualized, with the
signs adequately reversed.
The optimum increases by $b\triangle c$ for a change of $\triangle c$ if the primal
variable is basic with right bound $b$. This is perhaps more clear.

---

## Runge-Kutta, ODEs

You're modeling some time process or path that can be given as a function $y=D(t)$
where $t$ corresponds to the time dimension or the $x$-axis or similar, but $D$ is
given only as an ordinary differential equation (system), so you must solve for its path.

The equation is explicit if it can be written as $y'=f(t,y)$, and implicit if it can
only be written as $f(t,y,y')=0$.

The explicit kind can be solved with forward methods like Runge-Kutta: you know some
position $(t,y)$, you can approximate the next position by $(t_n,y_n)=(t+h,y+hf(t,y))$.

The implicit kind can be solved with forward methods too, where you take
$(t_n,y_n)=(t+h,y+hf(t,y))$ as the next position guess but now you need to solve the
equation $f(t_n,y_n,y_n')$ for $y_n'$, which you can do with newton's method usually, and
you can start your search with a guess that is just the last iteration's $y'$.

If you have several variables you should have different sets of partial differential
equations for each of them, for example for two variables $a$ and $b$ you may have
$$a'=\frac{da}{dx}=f(t,a,b)\wedge b'=\frac{db}{dx}=g(t,a,b)$$

and then you just perform Runge-Kutta for each of the variables *in parallel*.

**Euler-Lagrange equation**: consider a function $q$ and the set of smooth paths between
timepoints $a$ and $b$ such that $q(a)$ and $q(b)$ are fixed. Consider some function
$L(t,q(t),q'(t))$, which can be e.g. a cost or a reward. Then the extremals of
$$\int_a^bL(t,q(t),q'(t))dt$$
satisfy
$$\frac{\partial L}{\partial y}=\frac{d}{dx}\frac{\partial L}{\partial y'}$$

Expand out this equation for your particular $L$, then you can optimize the "cost function"
$L$ by just solving for the path between $a$ and $b$ such that $q(a)=x_a$ and $q(b)=x_b$
(the values you want there). If the resulting equation can be made explicit that makes RK
very fast, but it can be implicit too, that is fine.

If you do not know $q'(a)$ then you can search for it with binary/ternary search, until
you find the path such that $q(b)=x_b$.
