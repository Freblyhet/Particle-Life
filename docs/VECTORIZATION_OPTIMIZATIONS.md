# Vectorization and Matrix-Based Optimizations

## Overview
This document describes the vectorization and linear algebra optimizations applied to achieve better performance with large particle counts (800+).

## Key Optimizations Applied

### 1. **Compiler-Level Vectorization**
```cmake
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ffast-math -funroll-loops")
```

- **-O3**: Aggressive optimizations including auto-vectorization
- **-march=native**: Use all CPU instructions available (AVX2, SSE4.2, etc.)
- **-ffast-math**: Relaxed IEEE 754 compliance for faster math operations
- **-funroll-loops**: Unroll loops for better instruction-level parallelism

### 2. **Structure of Arrays (SoA) Pattern**
Instead of processing particles one at a time with scattered memory access:
```cpp
// OLD: Array of Structures (AoS)
for (each particle) {
    particle.x, particle.y, particle.vx, particle.vy
}
```

We use force arrays that allow vectorized operations:
```cpp
// NEW: Separated arrays allow SIMD
std::vector<float> fx(n), fy(n);  // Forces processed in batches
```

### 3. **OpenMP SIMD Directives**
```cpp
#pragma omp simd           // Tell compiler to vectorize this loop
#pragma omp parallel for   // Parallelize across CPU cores
```

The compiler can now process 4-8 particles simultaneously using SIMD instructions (AVX2).

### 4. **Branchless Distance Calculations**
```cpp
// Squared distance comparison (no sqrt needed for range check)
const float distSq = dx * dx + dy * dy;
const bool inRange = (distSq > 0.00001f) && (distSq < radiusSq);

if (inRange) {
    const float invDist = 1.0f / std::sqrt(distSq);  // Only one sqrt
    const float dist = distSq * invDist;             // Recover distance
}
```

### 5. **Cache-Friendly Memory Access**
- Pre-allocated force arrays avoid dynamic allocations
- Const references reduce memory copies
- Local accumulation of forces before writing back

## Mathematical Approach: Matrix Operations

### Force Calculation as Matrix Operations

For N particles, the ideal vectorized approach would be:

```
Position Matrix P: [N × 2]  (x, y coordinates)
Distance Matrix D: [N × N]  (pairwise distances)
Force Matrix F:    [N × N]  (pairwise forces)
Type Matrix T:     [N × 1]  (particle types)
Interaction Matrix I: [T × T] (force coefficients between types)
```

The force on particle i is:
```
F_i = Σ(j≠i) Force(D_ij, I[T_i][T_j]) * normalize(P_j - P_i)
```

### Why Full Matrix Operations Aren't Used Here

1. **Memory**: N×N matrix for 800 particles = 640,000 elements = 2.5MB
2. **Sparsity**: Spatial hashing means most entries are zero (particles only interact nearby)
3. **OpenGL 3.3**: Lacks compute shaders for true GPU matrix operations

### Current Hybrid Approach

We use **spatial hashing** to create sparse interaction lists, then:
- Process each particle's neighborhood in vectorized batches
- Compiler auto-vectorizes the inner loops with SIMD
- OpenMP parallelizes across CPU cores (if enabled)

## Performance Characteristics

### Before Optimizations
- 800 particles: ~20 FPS (slow)
- Each particle: O(800) distance checks = 640,000 operations
- Heavy sqrt usage: ~640,000 sqrt calls per frame

### After Vectorization
- 800 particles: ~60 FPS target
- Spatial hashing: O(k) checks where k ≈ 50-100 neighbors
- Single sqrt per interaction
- SIMD processes 4-8 operations simultaneously
- Better cache utilization

## SIMD Instruction Usage

On modern CPUs (with AVX2):
```
Without SIMD: Process 1 particle per instruction
With SIMD:    Process 4-8 particles per instruction (4x-8x speedup potential)
```

Example vectorized operations:
```
// These operations can be vectorized by the compiler:
float dx = px[i] - px[j];  // 8 subtractions at once (AVX2)
float dy = py[i] - py[j];  // 8 subtractions at once (AVX2)
float distSq = dx*dx + dy*dy;  // 8 FMA operations at once
```

## Future Optimizations

### GPU Compute Shaders (Requires OpenGL 4.3+)
For true matrix operations on GPU:
```glsl
// Compute shader processes all N particles in parallel
layout(local_size_x = 256) in;

void main() {
    uint i = gl_GlobalInvocationID.x;
    vec2 pos = positions[i];
    vec2 force = vec2(0.0);
    
    // Inner loop: GPU processes thousands of these in parallel
    for (uint j = 0; j < N; j++) {
        force += calculateForce(pos, positions[j]);
    }
    
    forces[i] = force;
}
```

### Eigen Library Integration
For explicit SIMD matrix operations:
```cpp
#include <Eigen/Dense>

Eigen::MatrixXf positions(N, 2);
Eigen::MatrixXf forces = computeForces(positions);  // Fully vectorized
```

## Benchmarking Results

Test with the ImGui performance display to see:
- FPS improvement
- Force calculations per frame
- Average update time

Expected improvements:
- 2-3x from sqrt elimination
- 1.5-2x from SIMD vectorization
- 2-4x from OpenMP parallelization (if multi-core)

**Total theoretical speedup: 6-24x**

## How to Enable/Disable Optimizations

### Compiler Optimizations
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release  # Enable
cmake .. -DCMAKE_BUILD_TYPE=Debug    # Disable
```

### OpenMP (requires linking with -fopenmp)
To fully enable OpenMP, add to CMakeLists.txt:
```cmake
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ParticleLife PUBLIC OpenMP::OpenMP_CXX)
endif()
```

## Verification

Check if optimizations are working:
```bash
# Check for SIMD instructions in compiled binary
objdump -d build/ParticleLife | grep -i "vmov\|vadd\|vmul"

# Profile the application
instruments -t "Time Profiler" build/ParticleLife
```

## References

- [Intel AVX Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/)
- [OpenMP SIMD Directives](https://www.openmp.org/spec-html/5.0/openmpsu42.html)
- [Fast Inverse Square Root](https://en.wikipedia.org/wiki/Fast_inverse_square_root)
- [Structure of Arrays vs Array of Structures](https://en.wikipedia.org/wiki/AoS_and_SoA)
