# 🤖 Maze Solver Robot — ITEC Hackathon 2026

**Autonomous robot for solving and mapping a 10×10 maze**

Project developed during the **ITEC Embedded Hackathon** (~48h) by a team of 3.

![STM32F407G-DISC1](https://www.st.com/bin/ecommerce/api/image.PF252419.en.feature-description-include-personalized-no-498x305.jpg)

---

## 📋 Table of Contents

- [Description](#description)
- [Features](#features)
- [Hardware](#hardware)
- [Software Architecture](#software-architecture)
- [Wiring Diagram](#wiring-diagram)
- [Implemented Algorithms](#implemented-algorithms)
- [Bluetooth Commands](#bluetooth-commands)
- [Setup & Build](#setup--build)
- [Calibration](#calibration)
- [Project Structure](#project-structure)
- [Demo](#demo)
- [Team](#team)

---

## Description

The robot solves a 10×10 cell maze (25cm per cell) in two phases:

1. **Autonomous exploration (DFS)** — the robot traverses the maze using Depth-First Search with backtracking, mapping all walls detected by the ultrasonic sensors
2. **Map-based solving** — the jury specifies a start and stop cell, the robot computes the shortest path (BFS) or longest path (DFS brute-force) and follows it autonomously

Communication with the user is done via **Bluetooth (HC-05)** using the **Serial Bluetooth Terminal** app on Android.

---

## Features

| Feature | Status | Details |
|---|---|---|
| Robot movement through maze | ✅ | DC motors with L298N, lateral correction using sensors |
| Autonomous exploration (DFS) | ✅ | DFS with stack + automatic backtracking |
| Maze mapping (cartography) | ✅ | Internal 16×16 map, walls on 4 directions |
| Map visualization on phone | ✅ | ASCII text map over Bluetooth |
| Start/stop configuration from phone | ✅ | Commands `S2,3` and `E7,5` via BT |
| Shortest path (BFS) | ✅ | Flood Fill + path reconstruction |
| Longest path | ✅ | DFS brute-force on mapped graph |
| Autonomous navigation on computed path | ✅ | Follows cell sequence automatically |
| Return Home | ✅ | Traverses DFS stack in reverse |
| Odometry data | ✅ | Hall sensor — wheel rotations + distance traveled |
| Bluetooth communication | ✅ | HC-05 @ 9600 baud, commands + telemetry |
| Manual mapping (push mapping) | ✅ | F/L/R/B commands from phone |
| Filtered sensors (anti-noise) | ✅ | Last valid value, 50cm limit |
| Stuck detection (Hall) | ✅ | Alert if wheels are not spinning |

---

## Hardware

### Components

| Component | Model | Role |
|---|---|---|
| Microcontroller | STM32F407G-DISC1 | Main board, ARM Cortex-M4 @ 168MHz |
| Motor driver | L298N Dual H-Bridge | PWM control for DC motors |
| Motors | 2× DC with gearbox | Differential drive |
| Distance sensors | 3× HC-SR04 | Ultrasonic: front, left, right |
| Hall sensor | YS-27 + magnet | Odometry — wheel rotations |
| Bluetooth | HC-05 Master/Slave | Communication with phone |
| Power supply | 6× AA → 9V + buck converter 5V | L298N + STM32 + sensors |
| Chassis | 2-motor robot kit | Mobile platform |

### STM32F407G-DISC1 Pinout

```
┌─────────────────────────────────────────────┐
│  MOTORS (L298N)                             │
│  ENA → PB4 (TIM3_CH1) = RIGHT Motor        │
│  ENB → PB5 (TIM3_CH2) = LEFT Motor         │
│  IN1 → PE7  IN2 → PE8  (right direction)   │
│  IN3 → PE9  IN4 → PE10 (left direction)    │
├─────────────────────────────────────────────┤
│  ULTRASONIC SENSORS (HC-SR04)               │
│  FRONT: Trig=PC0  Echo=PB0 (5V tolerant)   │
│  LEFT:  Trig=PC2  Echo=PB1 (5V tolerant)   │
│  RIGHT: Trig=PC4  Echo=PE6 (5V tolerant)   │
├─────────────────────────────────────────────┤
│  HALL SENSOR (YS-27)                        │
│  Signal → PA1 (polling, not interrupt)      │
│  VCC → 3.3V   GND → GND                    │
├─────────────────────────────────────────────┤
│  BLUETOOTH (HC-05)                          │
│  STM32 TX (PD5) → HC-05 RXD                │
│  STM32 RX (PD6) → HC-05 TXD                │
│  USART2 @ 9600 baud                        │
├─────────────────────────────────────────────┤
│  LEDs (on Discovery board)                  │
│  PD12 (green)  = front wall detected        │
│  PD13 (orange) = left wall detected         │
│  PD14 (red)    = right wall detected        │
│  PD15 (blue)   = heartbeat / waiting        │
├─────────────────────────────────────────────┤
│  BUTTON                                     │
│  PA0 (USER) = START/STOP toggle             │
└─────────────────────────────────────────────┘
```

### Power Supply

```
6×AA (9V) ──→ L298N VCC (motor power)
            └──→ Buck Converter → 5V ──→ STM32 (5V pin)
                                       ──→ HC-SR04 VCC
                                       ──→ HC-05 VCC
L298N 5V jumper: REMOVED (external power supply)
YS-27 Hall: powered from STM32 3.3V
```

---

## Software Architecture

```
┌──────────────────────────────────────────────────────┐
│                    main.c                            │
├──────────────┬───────────────┬────────────────────────┤
│  SENSORS     │   MOTORS      │    MAZE                │
│  ──────────  │   ────────    │    ────────            │
│  Citeste_    │   Mers_       │    Maze_Init()         │
│  Senzor()    │   Celula()    │    Maze_Scan()         │
│  Citeste_    │   Viraj_S()   │    Maze_Status()       │
│  Filtrat()   │   Viraj_D()   │    maze_walls[16][16]  │
│  Citeste_    │   Viraj_180() │    maze_visit[16][16]  │
│  Toti()      │   Roteste_    │                        │
│              │   La()        │                        │
├──────────────┴───────────────┴────────────────────────┤
│                 ALGORITHMS                            │
│  ─────────────────────────                            │
│  DFS_Step()        — exploration with backtracking    │
│  BFS_Shortest()    — shortest path (flood fill)       │
│  BFS_Longest()     — longest path (DFS brute-force)   │
│  Return_Home()     — return via DFS stack             │
│  Run_Path()        — follow computed path             │
├───────────────────────────────────────────────────────┤
│                 BT COMMUNICATION                      │
│  ─────────────────────────                            │
│  Process_BT()  — command parsing (UART polling)       │
│  BT_Map()      — transmit map data                    │
│  BT_Visual()   — transmit ASCII visual map            │
│  BT_Path()     — transmit computed path               │
│  Manual_Cmd()  — manual mapping commands              │
└───────────────────────────────────────────────────────┘
```

---

## Wiring Diagram

```
                    ┌─────────────┐
                    │  STM32F407  │
                    │  DISC1      │
          ┌─────────┤             ├─────────┐
          │   PE7-10│  Motor Dir  │PB4,PB5  │
          │         │  (GPIO OUT) │(TIM3 PWM)│
          │         │             │         │
          │   PC0,2,4  Trig OUT  │         │
          │   PB0,1    Echo IN   │         │
          │   PE6      Echo IN   │         │
          │         │             │         │
          │   PA1   │  Hall IN    │         │
          │   PA0   │  Button IN  │         │
          │         │             │         │
          │   PD5   │  UART TX ───┼──→ HC-05 RXD
          │   PD6   │  UART RX ←──┼─── HC-05 TXD
          │         └─────────────┘         │
          │                                 │
          │         ┌─────────────┐         │
          └────────→│   L298N     │←────────┘
          IN1-IN4   │  Motor      │  ENA, ENB
                    │  Driver     │
                    │             │
                    │ OUT1,2  OUT3,4
                    └──┬──┬───┬──┬┘
                       │  │   │  │
                    Motor R  Motor L

    ┌──────┐    ┌──────┐    ┌──────┐
    │HC-SR04│    │HC-SR04│    │HC-SR04│
    │FRONT  │    │LEFT   │    │RIGHT  │
    └──────┘    └──────┘    └──────┘
```

---

## Implemented Algorithms

### 1. DFS (Depth-First Search) — Exploration

The algorithm explores the maze by preferring unvisited cells. At each cell:

1. Scans walls with the 3 ultrasonic sensors
2. Updates the internal map (`maze_walls[][]`)
3. Looks for unvisited neighbors (visit_count == 0)
4. If found → saves return direction on stack → moves there
5. If not → **BACKTRACK** = pops from stack, goes back
6. When stack is empty = **EXPLORATION COMPLETE**

```
Current cell: (2,3) facing North
Sensors: F=25cm L=8cm R=45cm
Detected walls: [NW] (north + west)
Free neighbors: East(unvisited), South(visited×1)
Decision: → East (prefers unvisited)
Stack: push(West) = return direction
```

### 2. BFS (Breadth-First Search) — Shortest Path

Flood Fill from start to end on the mapped maze:

1. Initialize all cells with distance -1
2. Start = distance 0, add to queue
3. BFS: for each cell in queue, explore neighbors without walls
4. When end is reached → reconstruct path via `bfs_parent[][]`
5. Reverse the path (it's built from end to start)

### 3. Longest Path

DFS brute-force exploring all possible paths and retaining the longest:

1. Start from source with depth 0
2. At each cell, mark as visited and recursively explore neighbors
3. When end is reached, compare depth with known maximum
4. On return, unmark the cell (backtracking)
5. Result: longest path without loops

### 4. Lateral Correction — Straight Driving

While driving through a corridor:
- If both walls visible (<20cm): proportional centering based on difference
- If one wall <6cm: aggressive correction (±8000 PWM)
- Safety: front wall <8cm → instant stop

---

## Bluetooth Commands

Recommended app: **Serial Bluetooth Terminal** (Android)

### Button commands (single char)

| Command | Action |
|---------|--------|
| `G` | Start autonomous DFS exploration |
| `X` | Emergency stop — stops motors + sends map |
| `M` | Display visual map on terminal |
| `P` | Compute SHORTEST path (BFS) |
| `Q` | Compute LONGEST path |
| `R` | RUN — robot follows computed path |
| `H` | Return Home — follows DFS stack back |

### Manually typed commands

| Command | Action |
|---------|--------|
| `S2,3` + Send | Set START cell to (2,3) |
| `E7,5` + Send | Set END cell to (7,5) |
| `DN` + Send | Set robot direction to North |
| `DE` + Send | Set robot direction to East |
| `DS` + Send | Set robot direction to South |
| `DW` + Send | Set robot direction to West |

### Manual mapping commands

| Command | Action |
|---------|--------|
| `F` | Forward — pushed robot one cell forward |
| `L` | Left — rotated robot 90° left |
| `B` | Back — rotated robot 180° |

### App button configuration

```
M1 = G    M2 = M    M3 = X    M4 = P
M5 = R    M6 = H    M7 = Q
```

### Example Bluetooth output

```
=== MAZE ROBOT v3 ===
G=explore X=stop M=map
>>>GO<<<
(0,0)N v1 F:25 S:8 D:12 [NW] H:0 dist:0cm #1
->E
(1,0)E v1 F:45 S:6 D:8 [SW] H:3 dist:23cm #2
->N
(1,1)N v1 F:8 S:12 D:25 [NE] H:6 dist:47cm #3
<<S
(1,0)S v2 F:45 S:8 D:6 [SW] H:9 dist:70cm #3
>>> DONE! <<<

--- MAP ---
+---+---+
|   | S |
+   +   +
| ^ |   |
+---+---+
Robot:(0,0)N Cells:3 Hall:9 Dist:70cm
```

---

## Setup & Build

### Requirements

- **STM32CubeIDE** (or STM32CubeMX + arm-none-eabi-gcc)
- **Serial Bluetooth Terminal** (Android) — for BT communication
- **STM32F407G-DISC1** board with integrated **ST-LINK V2**

### Steps

1. **Clone the repository:**
   ```bash
   git clone https://github.com/mUstiuQ/maze-solver-robot.git
   ```

2. **Open in STM32CubeIDE:**
   - File → Import → Existing Projects into Workspace
   - Select the project folder

3. **Build:**
   - Project → Build All (Ctrl+B)

4. **Flash:**
   - Run → Run As → STM32 C/C++ Application

5. **Bluetooth connection:**
   - Pair HC-05 with your phone (PIN: `1234`)
   - Open Serial Bluetooth Terminal
   - Connect to HC-05
   - Configure buttons M1-M7

### CubeMX Configuration

```
Clock: HSE 8MHz → PLL → SYSCLK 168MHz
APB1: 42MHz (timer clock 84MHz)
APB2: 84MHz

TIM3: PSC=167, Period=65535, CH1+CH2 PWM (PB4, PB5)
TIM5: PSC=0 (overridden to 41 in code), Period=max, 1μs resolution
USART2: 115200 in CubeMX, re-init to 9600 in code (PD5 TX, PD6 RX)
```

---

## Calibration

These values need to be adjusted on the actual maze:

```c
#define SPEED_RIGHT  48000    // Right motor PWM (increase if drifting left)
#define SPEED_LEFT   50000    // Left motor PWM
#define SPEED_TURN   30000    // Turn speed PWM
#define TURN_90_MS   350      // 90° turn duration (ms)
#define TURN_180_MS  700      // 180° turn duration (ms)
#define CELL_DRIVE_MS 450     // Drive time for one 25cm cell (ms)
#define WALL_FRONT   13       // Front wall detection threshold (cm)
#define WALL_SIDE    6        // Lateral correction threshold (cm)
#define WALL_DETECT  12       // Wall mapping threshold (cm)
```

### Calibration procedure

1. **Straight driving:** Place robot in a straight corridor, start with G. If it drifts right → increase `SPEED_RIGHT`. If it drifts left → increase `SPEED_LEFT`.

2. **Cell distance:** Measure how many cm it travels per `CELL_DRIVE_MS`. Adjust until it travels exactly 25cm.

3. **90° turn:** Start the robot, let it make a turn. If it turns too much → decrease `TURN_90_MS`. If too little → increase.

4. **Sensor thresholds:** If it detects non-existent walls → decrease `WALL_DETECT`. If it misses real walls → increase.

---

## Project Structure

```
maze-solver-robot/
├── README.md                    # This file
├── LICENSE                      # MIT License
├── docs/
│   ├── pinout.md               # Detailed pinout
│   ├── algorithms.md           # Algorithm descriptions
│   └── calibration.md          # Calibration guide
├── src/
│   └── main.c                  # Complete source code
├── hardware/
│   ├── wiring_diagram.md       # Wiring diagram
│   └── BOM.md                  # Bill of Materials
└── images/
    ├── robot_photo.jpg         # Robot photo
    ├── maze_map.jpg            # Generated map example
    └── terminal_output.jpg     # BT terminal screenshot
```

---

## Competition Flow

### Phase 1 — Exploration
```
1. Place robot in maze corner, facing North
2. Press PA0 or send G via BT
3. Robot autonomously explores the entire maze
4. When finished: >>> DONE! <<<
5. Send M → map appears on phone
6. Show map to jury
```

### Phase 2 — Solving
```
1. Jury specifies: start=(2,3) stop=(7,5)
2. Type on phone: S2,3 [Send]
3. Type: E7,5 [Send]
4. Press P (shortest) or Q (longest)
5. Computed path appears on screen
6. Place robot in start cell
7. Type: DN [Send] (facing North)
8. Press R → robot follows path automatically
9. >>> ARRIVED! <<<
```

---

## Demo

### Robot-generated map (Bluetooth output)
```
--- MAP ---
+---+---+   +   +   +   +
| 2  2| |         . .|
+   +   +   +   +   +   +
| 2  2|2|   .   . . .|
+   +   +   +   +   +   +
| 2  2       .   . . .|
+   +   +   +   +   +   +
|2|     .   .   . . .|
+   +   +   +   +   +   +
|2|   .   .   . . .|
+   +   +   +   +   +   +
| |   .   .   . .|
+---+   +   +   +   +   +
|       .   .   . .|
+   +   +   +   +   +   +
Robot:(7,15) E Cells:44 Hall:15 Dist:117cm
```

---

## Team

Project developed during **ITEC Embedded Hackathon 2026** — Timișoara, Romania

🏆 **3rd Place** — Embedded Development Challenge

---

## License

MIT License — see [LICENSE](LICENSE) file
