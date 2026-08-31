# lanelet2-algorithms

Reimplementing core [Lanelet2](https://github.com/fzi-forschungszentrum-informatik/Lanelet2)
HD-map algorithms in C++, then grading each implementation against the
library's own reference output.

Lanelet2 already provides Frenet conversion, map matching, routing, and map
validation. The point of this repo is not to duplicate those features for
their own sake — it's to build them from scratch as a way of actually
understanding the framework: its geometry primitives, its ownership model,
and the API surface that autonomous-vehicle stacks build on.

## Approach

Each milestone follows the same pattern:

1. Implement the algorithm against real map data (`mapping_example.osm`).
2. Compare output against the equivalent Lanelet2 library call on the same
   inputs, sweeping many elements and many sampled points.
3. Report mismatch counts and worst-case error, not just pass/fail.

## Milestones

- **0. Build and inspect** — link Lanelet2, load the map, dump an SVG of all
  lanelet bounds.
- **1. Frenet conversion** — Cartesian to (s, d) and back, stitched across
  consecutive lanelets.
- **2. Map matching** — pose and heading to candidate lanelets; linear scan
  vs. R-tree.
- **3. Routing graph** — successors, adjacency, Dijkstra/A*, lane-change cost.
- **4. Reachable set and corridor** — BFS with an arc-length budget.
- **5. Intersection conflicts** — centerline crossings vs. topological
  connectivity and regulatory elements.
- **6. Validators** — self-intersections, missing speed limits, disconnected
  components, inconsistent adjacency.
- **7. Map generation** — GPS trace to polyline fit to Lanelet2 export.

## Status

Milestone 0, in progress.

## Build

C++17, CMake + Ninja, GoogleTest. Setup instructions land in `SETUP.md` once
the Lanelet2 install route is confirmed.
