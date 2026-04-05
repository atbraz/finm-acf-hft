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

= Correctness Testing

Before measuring performance, we verify correctness with a two-tier test suite compiled into a single binary (`tests/correctness.cpp`).

== Hardcoded Tests

Ten deterministic tests with hand-computed expected values cover basic operation and edge cases:

#figure(
  table(
    columns: (2fr, 3fr),
    align: (left, left),
    table.header([*Test*], [*What it covers*]),
    [`test_multiply_mv_row_major`], [2#sym.times;2 row-major MV],
    [`test_multiply_mv_col_major`], [2#sym.times;2 col-major MV],
    [`test_multiply_mm_naive`], [2#sym.times;2 naive MM],
    [`test_multiply_mm_transposed_b`], [2#sym.times;2 transposed-B MM],
    [`test_multiply_mm_tiled`], [2#sym.times;2 tiled MM],
    [`test_mv_1x1`], [1#sym.times;1 scalar MV (both layouts)],
    [`test_mm_1x1`], [1#sym.times;1 scalar MM (all three variants)],
    [`test_mm_identity`], [3#sym.times;3 identity: $A dot I = A$],
    [`test_mv_non_square`], [2#sym.times;3 rectangular MV (both layouts)],
    [`test_mm_non_square`], [2#sym.times;3 #sym.times; 3#sym.times;2 rectangular MM (all three variants)],
  ),
  caption: [Hardcoded correctness tests],
)

== Randomized Cross-Validation

Rather than hand-computing expected outputs for large matrices, we exploit the invariant that different kernel implementations must agree on the same mathematical operation. A seeded PRNG (`std::mt19937`, seed 42) generates random matrices with values in $[-100, 100]$, ensuring deterministic reproducibility.

Floating-point tolerance scales with the inner product length $k$ as $k times 10^(-9)$, accounting for accumulation error that grows with the number of additions.

#figure(
  table(
    columns: (1.5fr, 1fr, 2.5fr),
    align: (left, center, left),
    table.header([*Suite*], [*Trials*], [*What it cross-validates*]),
    [`fuzz_mv`], [1100], [Row-major vs col-major MV across 22 dimension configs (square + non-square), 50 trials each],
    [`fuzz_mm`], [1300], [Naive vs transposed-B vs tiled MM across 27 dimension configs including tile-boundary stress sizes (31, 33, 63, 65, 95, 97), 50 trials each],
    [`fuzz_mm_tiles`], [600], [Tiled MM with 10 tile sizes (1, 2, 7, ..., 64) across 12 matrix sizes, 5 trials each, against naive as reference],
  ),
  caption: [Randomized cross-validation suites (3000 total trials)],
)

Tile-boundary sizes (e.g.~31, 33 around the default tile of 32) specifically target off-by-one bugs in the tiling remainder logic. Non-square dimensions (e.g.~$3 times 4 times 2$) catch errors that only manifest when $m eq.not k eq.not n$.

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

Two low-hanging optimisations were identified from the profiling data and compiler diagnostics.

== Pointer Aliasing: `__restrict__`

The `-Rpass-missed=slp-vectorizer` output for the unoptimised kernels reported _"vectorization was possible but not beneficial"_ and _"vectorization was impossible with available vectorization factors"_ on every inner loop. The root cause: without aliasing information, the compiler must assume that writes through `result` could modify the memory pointed to by `matrixA` or `matrixB`, preventing it from reordering or widening loads into SIMD instructions.

Adding `__restrict__` to all pointer parameters constitutes a promise to the compiler that no two `restrict`-qualified pointers in the same scope alias. For matrix multiplication with a separate output buffer this invariant always holds. The effect is immediate: after the annotation, `-Rpass=loop-vectorize` confirms all five inner loops vectorise at width~2 (128-bit NEON, two `double` values per register) with an interleave count of~4.

#figure(
  table(
    columns: (3fr, 2fr),
    align: (left, left),
    table.header([*Before (`-Rpass-missed`)*], [*After (`-Rpass`)*]),
    [vectorization was possible but not beneficial], [vectorized loop (width: 2, interleaved: 4)],
    [vectorization was impossible with available factors], [vectorized loop (width: 2, interleaved: 4)],
  ),
  caption: [Compiler vectorisation remarks before and after `__restrict__`],
)

== Tile Size Tuning

The tiled kernel's default tile size was 32. On the target M4 Pro (128~KB L1d per P-core, 128-byte cache lines), three $32 times 32$ tiles of `double` occupy $3 times 32^2 times 8 = 24"KB"$ --- only 19% of L1d. Increasing the tile to~64 raises the working set to $3 times 64^2 times 8 = 96"KB"$ (75% of L1d), improving data reuse within the cache before eviction while leaving headroom for stack and loop state.

== Combined Results

@tab:opt shows the effect at $n = 1024$ under `-O3`. The tiled kernel with both optimisations achieves a $6.2 times$ speedup over the naive baseline.

#figure(
  table(
    columns: (2fr, 1fr, 1fr),
    align: center,
    table.header([*Variant*], [*Mean (ms)*], [*Speedup*]),
    [`mm_naive`], [1177.5], [1.0#sym.times],
    [`mm_transposed_b`], [491.7], [2.4#sym.times],
    [`mm_tiled` (tile=64)], [191.1], [6.2#sym.times],
  ),
  caption: [Baseline vs optimised, $n = 1024$, `-O3`],
) <tab:opt>
