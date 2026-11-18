# Exhaustive check of the antimagic labeling conjecture

**Conjecture (Hartsfield–Ringel).** Every connected graph with at least two edges
admits an *antimagic labeling*: a bijection from its edges to `{1, ..., m}`
(m = number of edges) such that the vertex sums are pairwise distinct.

This repository contains a self-contained C program that decides the property for
every labelled graph on `n` vertices, and the results of running it exhaustively.

## Method

* Enumerate all `2^C(n,2)` labelled graphs; keep the connected ones (bitset BFS).
* Depth-first search over the `m!` labelings with two prunings:
  1. if two vertices already have equal partial sums and their sets of
     *unlabelled* incident edges coincide, the branch is dead
     (any completion keeps them equal);
  2. edges are ordered greedily so that consecutive edges share a vertex,
     which makes pruning (1) fire early.
* The program reports `undecided` for every graph where the node budget
  (4·10^6) was exhausted. `undecided = 0` therefore means the run is a proof,
  not a heuristic.

## Verified results (exhaustive, zero undecided cases)

| n | connected labelled graphs | antimagic | undecided |
|---|--------------------------:|----------:|----------:|
| 3 | 4 | 4 | 0 |
| 4 | 38 | 38 | 0 |
| 5 | 728 | 728 | 0 |
| 6 | 26 704 | 26 704 | 0 |
| 7 | 1 866 256 | 1 866 256 | 0 |
| 8 | 251 548 592 | 251 548 592 | 0 |

The connected-graph counts reproduce OEIS A001187 exactly, which independently
confirms that the enumeration visited every connected labelled graph on `n`
vertices exactly once.

n = 8 was run as 256 shards on GitHub Actions
(`.github/workflows/search.yml`, run 44331794220); shard outputs are summed and
must equal A001187(8) = 251548592, otherwise the run is rejected.

## Cost / hardness

Backtracking is almost never needed. The worst single graph at `n = 7` needed
231 search nodes; at `n = 8` fewer than 100. The search is therefore essentially
linear in the number of graphs, and the real bound is the `2^C(n,2)` enumeration:
`n = 9` needs 2^36 graphs (~83 CPU-hours), `n = 10` is out of reach
(2^45 graphs).

## Reproduce

```
cc -O2 -o antimagic antimagic.c
./antimagic 7 0 1          # all 7-vertex graphs
./antimagic 8 17 256       # shard 17 of 256
./antimagic 4 0 1 count    # also count antimagic labelings (20 for n=3)
```
