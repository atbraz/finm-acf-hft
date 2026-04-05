#set document(
  title: "High-Performance Linear Algebra Kernels",
  author: "Team 11",
)
#set page(paper: "a4", margin: (x: 2.5cm, y: 3cm))
#set text(font: "New Computer Modern", size: 11pt)
#set heading(numbering: "1.")
#set par(justify: true)

#align(center)[
  #text(size: 16pt, weight: "bold")[High-Performance Linear Algebra Kernels]

  #v(0.4em)
  ACF — Team 11

  #v(0.2em)
  #datetime.today().display("[month repr:long] [day], [year]")
]

#v(1em)
#line(length: 100%)
#v(1em)

= Benchmarking Results

All timings measured with `std::chrono::high_resolution_clock`, 100 runs per case, reporting mean and standard deviation.

#figure(
  table(
    columns: (2fr, 1fr, 1fr, 1fr, 1fr),
    align: center,
    table.header(
      [*Function*], [*n*], [*Opt level*], [*Mean (ms)*], [*Std Dev (ms)*],
    ),
    // TODO: fill in after running just bench / just bench-O3
    [_to be filled_], [], [], [], [],
  ),
  caption: [Benchmark results across matrix sizes and optimisation levels],
)

= Cache Locality Analysis

== Matrix-Vector Multiplication: Row-Major vs Column-Major

// TODO: explain cache access patterns, which is faster and why

== Matrix-Matrix Multiplication: Naive vs Transposed B

// TODO: explain how transposed B improves spatial locality for B's access pattern

= Memory Alignment

// TODO: report alloc_aligned (64-byte) vs unaligned results; note conditions under which alignment matters

= Inlining and Compiler Optimisations

// TODO: compare -O0 vs -O3 timings; discuss assembly; discuss when inline helps vs hurts

= Profiling

== Setup

// TODO: describe Instruments / xcrun xctrace invocation on macOS

== Results

#figure(
  rect(width: 90%, height: 6cm, stroke: gray)[
    #align(center + horizon)[
      _Profiler screenshot — add to `discussions/figures/` and replace this block_
    ]
  ],
  caption: [Instruments Time Profiler — naive matrix-matrix multiplication],
)

// TODO: annotate hot paths identified in the call tree

= Optimisation Strategy

// TODO: document chosen optimisation (loop reordering / blocking / tiling), reasoning, and results

#figure(
  table(
    columns: (2fr, 1fr, 1fr),
    align: center,
    table.header([*Variant*], [*Mean (ms)*], [*Speedup*]),
    // TODO: fill in
    [_to be filled_], [], [],
  ),
  caption: [Baseline vs optimised, n=1024, -O3],
)
