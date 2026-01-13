# Why Particles Group Up When Forces Are Cleared

## The Mystery

When you click "Clear Forces" (setting all interaction forces to 0), you might expect particles to spread out evenly or stay where they are. Instead, they mysteriously **group up in pairs**!

## The Physics Explanation

### 1. Initial Conditions
When particles are created, they get **random initial velocities**:
```cpp
std::uniform_real_distribution<float> velDist(-0.0005f, 0.0005f);
p.vx = velDist(rng);  // Random velocity in X
p.vy = velDist(rng);  // Random velocity in Y
```

### 2. What Happens When Forces = 0

With all forces set to zero, the system follows these physics laws:

**Newton's First Law**: *An object in motion stays in motion*
- Each particle continues moving with its initial velocity
- No forces means no change in velocity (except friction)

**Conservation of Momentum**: *Total momentum is conserved*
- When two particles get close, their combined momentum is conserved
- They form a "two-body system" that orbits a common center of mass

### 3. Why Pairing Occurs

#### Step 1: Random Drift
Particles move in random directions with their initial velocities:
```
Particle A: vx = +0.0003, vy = -0.0002
Particle B: vx = +0.0004, vy = -0.0001
```

#### Step 2: Wrapping Boundaries
The simulation uses **wrapping boundaries** (like Pac-Man):
- When a particle exits one edge, it wraps to the opposite edge
- This means particles can "catch up" to each other from behind

#### Step 3: Friction Creates Weak Attraction
Even with forces = 0, there's still **friction** (typically 0.98):
```cpp
particles[i].vx *= config.friction;  // Gradually slows down
particles[i].vy *= config.friction;
```

When two particles get close:
1. Their relative motion slows due to friction
2. Numerical errors create tiny effective "forces"
3. They start orbiting each other
4. Friction gradually pulls them closer together

#### Step 4: Two-Body Problem
Once paired, particles form a classic **two-body orbital system**:
```
     • ← Particle A
    /  \
   ↓    ↑
    \  /
     • ← Particle B
```

The system conserves angular momentum, so they continue orbiting.

## The Math

For two particles with no external forces:

**Total Momentum (conserved)**:
```
P_total = m₁v₁ + m₂v₂ = constant
```

**Center of Mass Motion**:
```
v_cm = (m₁v₁ + m₂v₂) / (m₁ + m₂)
```

**Relative Motion**:
```
v_rel = v₁ - v₂
```

With friction `f < 1.0`:
```
v₁(t+dt) = v₁(t) * f
v₂(t+dt) = v₂(t) * f
```

The center of mass slows down, but the **relative angular momentum** creates orbiting behavior.

## Why Not Uniform Distribution?

You might think particles should space out evenly, but:

1. **No repulsion**: With forces = 0, particles have no reason to avoid each other
2. **Random velocities**: Initial motion is chaotic, not organized
3. **Wrapping boundaries**: Creates artificial "encounters" between particles
4. **No equilibrium**: There's no force field to create a uniform distribution
5. **Friction asymmetry**: Different particle pairs lose energy at different rates

## Demonstration: The Three Behaviors

### Behavior 1: "Clear Forces" (Original)
- Forces set to 0
- Velocities remain
- Result: **Particles pair up and orbit**

### Behavior 2: "Freeze All" (New!)
- Velocities set to 0
- Forces remain
- Result: **Particles stay frozen in place**

### Behavior 3: "Zero Forces + Freeze" (New!)
- Forces AND velocities set to 0
- Result: **Perfectly still particles**
- This is true "equilibrium"

## Real-World Analogy

Imagine:
- Turning off gravity (clearing forces)
- Astronauts floating in space (initial velocities)
- Air resistance in space (friction)

The astronauts would:
1. Continue floating in their initial directions
2. Eventually "bump into" each other (wrapping boundaries)
3. Form pairs that slowly rotate together (conservation of momentum)
4. Gradually slow down but maintain relative motion (friction + momentum conservation)

## Why Groups of 2 Specifically?

**Two-body systems are stable!**

In orbital mechanics:
- 2 bodies: Simple, stable orbits ✓
- 3+ bodies: Chaotic, unstable (the famous "three-body problem") ✗

With friction gradually removing energy:
- Larger groups break apart
- Pairs are the most stable configuration
- Each pair independently conserves its own momentum

## How to Test This

1. **Start simulation**: Observe normal behavior with forces
2. **Click "Clear Forces"**: Watch particles pair up over time
3. **Click "Freeze All"**: Everything stops immediately
4. **Click "Zero Forces + Freeze"**: Perfect stillness

## The Solution

If you want particles to **stay evenly distributed** when clearing forces:

**Option A**: Use "Zero Forces + Freeze" button
- Clears forces AND velocities
- Creates true static equilibrium

**Option B**: Add very weak repulsion
- Instead of force = 0, use force = -0.01
- Creates a "gas-like" behavior
- Particles bounce off each other gently

**Option C**: Disable friction temporarily
- Set friction = 1.0 (no energy loss)
- Particles maintain constant velocities
- More chaotic motion, less pairing

## Code Reference

### Where velocities are initialized:
`src/simulation/ParticleSystem.cpp` lines 76-78:
```cpp
std::uniform_real_distribution<float> velDist(-0.0005f, 0.0005f);
p.vx = velDist(rng);
p.vy = velDist(rng);
```

### Where friction is applied:
`src/simulation/ParticleSystem.cpp` lines ~370-372:
```cpp
particles[i].vx *= frictionFactor;  // Typically 0.98
particles[i].vy *= frictionFactor;
```

### New UI buttons:
`src/ui/Interface.cpp` lines ~373-395:
- "Freeze All" - Stops all motion
- "Zero Forces + Freeze" - Complete stillness

## Fun Fact: The N-Body Problem

What you're observing is related to one of physics' most famous unsolved problems:

- **2-body problem**: Solved analytically (Kepler orbits)
- **3-body problem**: No general solution (chaos theory)
- **N-body problem**: Approximations only (numerical simulation)

Your particle system with N particles and zero forces is effectively an N-body gravitational simulation, except:
- "Gravity" is replaced by initial momentum
- Friction adds energy dissipation
- Wrapping boundaries create artificial encounters

The pairing behavior you see is the system finding its **lowest energy configuration**: pairs of orbiting bodies!

## Summary

**Particles pair up because:**
1. They start with random velocities
2. Conservation of momentum creates orbital motion
3. Friction gradually removes energy
4. Two-body systems are the most stable configuration
5. Wrapping boundaries cause "collisions"

**To prevent pairing:**
- Use the new "Zero Forces + Freeze" button
- This sets both forces AND velocities to zero
- Creates true static equilibrium
