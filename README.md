# RISC-V-Parallel-SIMD-and-MIMD-Programming-and-Processing-Book
In this repository and book, we present a series of examples in **parallel programming and processing**. For each
example, we measure **execution time** and analyze the resulting **speedup** across different
algorithms and execution modes.

In the introductory part, we outline the content, objectives, methodology, and overall organization of
the book. Each chapter is structured as a laboratory session containing a sequence of practical
examples.

**Lab 1** introduces the essential elements of assembly programming on **RISC-V**.

**Labs 2 and 3** focus on vector processing (**SIMD**). We begin with simple kernels—vector addition, dot
product, and matrix multiplication—then move to more demanding tasks such as π approximation and
a basic FFT filter. Serial baselines are written in C, while vectorized versions are implemented in
RISC-V assembly using the vector extension.

In **Labs 4 and 5**, we turn to image processing, starting with simple image negation and advancing to
color-space conversion. Each task is implemented both serially and with vectorization to quantify
potential speedups. Image I/O (decode/encode) is handled with OpenCV. We also explore dynamic
image generation using OpenGL, rendering directly into video memory. As with static images, we
record runtimes and evaluate speedups.

**Lab 6** introduces OpenMP to leverage multicore (**MIMD**) parallelism with threads. We revisit several
workloads—π computation, matrix multiplication, and Mandelbrot rendering—and compare
performance across execution modes.

In **Lab 7**, we explicitly **contrast scalar** (**SISD**), vector (**SIMD**), and multicore (MIMD) implementations
using the π example. Finally, we demonstrate combined **MIMD×SIMD** approaches for π calculation
and matrix multiplication. In all cases, we report speedups as a function of problem size to highlight
scaling behavior and the practical benefits of each technique.

