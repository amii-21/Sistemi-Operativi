# Canteen Simulation — OS & IPC Project

A multiprocess simulation of a student canteen, built in C as an exercise in OS-level programming and inter-process communication.

## What it simulates

Multiple concurrent processes represent different actors in the canteen (students, operators, supervisor). They coordinate using System V IPC primitives:

| Mechanism | Used for |
|---|---|
| Shared memory | Global state visible to all processes |
| Semaphores | Mutual exclusion and finite-resource management |
| Message queues | Async communication between users and operators |

The simulation runs from a configuration file and outputs per-day statistics at the end.

## Stack

- C
- IPC System V (`shmget`, `semget`, `msgget`)
- `fork` / `exec`
- Makefile with strict compilation flags (`-Wall -Wextra -pedantic`)

## How to build and run

```bash
git clone https://github.com/amii-21/Sistemi-Operativi.git
cd Sistemi-Operativi

make

# Scenario 1 — timeout: simulation stops after a fixed number of days
./main config_timeout.conf

# Scenario 2 — overload: simulation stops early if too many users go unserved in a day
./main config_overload.conf
```

---
