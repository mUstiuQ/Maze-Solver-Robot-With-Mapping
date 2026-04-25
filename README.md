# 🤖 Maze Solver Robot — ITEC Hackathon 2026

**Robot autonom pentru rezolvarea și cartografierea unui labirint 10×10**

Proiect dezvoltat în cadrul **ITEC Embedded Hackathon** (~48h) de o echipă de 3 persoane.

![STM32F407G-DISC1](https://www.st.com/bin/ecommerce/api/image.PF252419.en.feature-description-include-personalized-no-498x305.jpg)

---

## 📋 Cuprins

- [Descriere](#descriere)
- [Funcționalități](#funcționalități)
- [Hardware](#hardware)
- [Arhitectura Software](#arhitectura-software)
- [Schema de Conexiuni](#schema-de-conexiuni)
- [Algoritmi Implementați](#algoritmi-implementați)
- [Comenzi Bluetooth](#comenzi-bluetooth)
- [Configurare și Build](#configurare-și-build)
- [Calibrare](#calibrare)
- [Structura Proiectului](#structura-proiectului)
- [Demo](#demo)
- [Echipa](#echipa)

---

## Descriere

Robotul rezolvă un labirint de 10×10 celule (25cm per celulă) în două faze:

1. **Explorare autonomă (DFS)** — robotul parcurge labirintul folosind algoritmul Depth-First Search cu backtracking, cartografiind toți pereții detectați de senzorii ultrasonici
2. **Rezolvare pe baza hărții** — juriul specifică o celulă de start și o celulă de stop, robotul calculează drumul cel mai scurt (BFS) sau cel mai lung (DFS brute-force) și îl parcurge automat

Comunicarea cu utilizatorul se face prin **Bluetooth (HC-05)** folosind aplicația **Serial Bluetooth Terminal** pe Android.

---

## Funcționalități

| Funcționalitate | Status | Detalii |
|---|---|---|
| Deplasare robot prin labirint | ✅ | Motoare DC cu L298N, corecție laterală cu senzori |
| Explorare autonomă (DFS) | ✅ | DFS cu stivă + backtracking automat |
| Cartografiere (mapare) labirint | ✅ | Hartă internă 16×16, pereți pe 4 direcții |
| Vizualizare hartă pe telefon | ✅ | Hartă text ASCII pe Bluetooth |
| Configurare start/stop de pe telefon | ✅ | Comenzi `S2,3` și `E7,5` pe BT |
| Drum cel mai scurt (BFS) | ✅ | Flood Fill + reconstrucție path |
| Drum cel mai lung | ✅ | DFS brute-force pe graful mapat |
| Navigare autonomă pe path calculat | ✅ | Urmează secvența de celule automat |
| Return Home | ✅ | Parcurge stiva DFS invers |
| Date odometrice | ✅ | Hall sensor — rotații roată + distanță parcursă |
| Comunicare Bluetooth | ✅ | HC-05 @ 9600 baud, comenzi + telemetrie |
| Mapare manuală (push mapping) | ✅ | Comenzi F/L/R/B de pe telefon |
| Senzori filtrați (anti-zgomot) | ✅ | Ultimă valoare validă, limită 50cm |
| Detecție blocare (Hall) | ✅ | Alertă dacă roțile nu se rotesc |

---

## Hardware

### Componente

| Componentă | Model | Rol |
|---|---|---|
| Microcontroller | STM32F407G-DISC1 | Placă de bază, ARM Cortex-M4 @ 168MHz |
| Driver motoare | L298N Dual H-Bridge | Control PWM motoare DC |
| Motoare | 2× DC cu reductor | Tracțiune diferențială |
| Senzori distanță | 3× HC-SR04 | Ultrasonici: față, stânga, dreapta |
| Senzor Hall | YS-27 + magnet | Odometrie — rotații roată |
| Bluetooth | HC-05 Master/Slave | Comunicare cu telefonul |
| Alimentare | 6× AA → 9V + buck converter 5V | L298N + STM32 + senzori |
| Șasiu | Kit robot 2 motoare | Platformă mobilă |

### Pinout STM32F407G-DISC1

```
┌─────────────────────────────────────────────┐
│  MOTOARE (L298N)                            │
│  ENA → PB4 (TIM3_CH1) = Motor DREAPTA      │
│  ENB → PB5 (TIM3_CH2) = Motor STÂNGA       │
│  IN1 → PE7  IN2 → PE8  (direcție dreapta)  │
│  IN3 → PE9  IN4 → PE10 (direcție stânga)   │
├─────────────────────────────────────────────┤
│  SENZORI ULTRASONICI (HC-SR04)              │
│  FRONT: Trig=PC0  Echo=PB0 (5V tolerant)   │
│  LEFT:  Trig=PC2  Echo=PB1 (5V tolerant)   │
│  RIGHT: Trig=PC4  Echo=PE6 (5V tolerant)   │
├─────────────────────────────────────────────┤
│  HALL SENSOR (YS-27)                        │
│  Signal → PA1 (polling, nu interrupt)       │
│  VCC → 3.3V   GND → GND                    │
├─────────────────────────────────────────────┤
│  BLUETOOTH (HC-05)                          │
│  STM32 TX (PD5) → HC-05 RXD                │
│  STM32 RX (PD6) → HC-05 TXD                │
│  USART2 @ 9600 baud                        │
├─────────────────────────────────────────────┤
│  LED-URI (pe Discovery board)               │
│  PD12 (verde)    = perete față detectat     │
│  PD13 (portocaliu) = perete stânga detectat │
│  PD14 (roșu)     = perete dreapta detectat  │
│  PD15 (albastru) = heartbeat / așteptare    │
├─────────────────────────────────────────────┤
│  BUTON                                      │
│  PA0 (USER) = START/STOP toggle             │
└─────────────────────────────────────────────┘
```

### Alimentare

```
6×AA (9V) ──→ L298N VCC (motor power)
            └──→ Buck Converter → 5V ──→ STM32 (5V pin)
                                       ──→ HC-SR04 VCC
                                       ──→ HC-05 VCC
L298N 5V jumper: SCOS (alimentare externă)
YS-27 Hall: alimentat de la 3.3V STM32
```

---

## Arhitectura Software

```
┌──────────────────────────────────────────────────────┐
│                    main.c                            │
├──────────────┬───────────────┬────────────────────────┤
│  SENZORI     │   MOTOARE     │    LABIRINT            │
│  ──────────  │   ────────    │    ────────            │
│  Citeste_    │   Mers_       │    Maze_Init()         │
│  Senzor()    │   Celula()    │    Maze_Scan()         │
│  Citeste_    │   Viraj_S()   │    Maze_Status()       │
│  Filtrat()   │   Viraj_D()   │    maze_walls[16][16]  │
│  Citeste_    │   Viraj_180() │    maze_visit[16][16]  │
│  Toti()      │   Roteste_    │                        │
│              │   La()        │                        │
├──────────────┴───────────────┴────────────────────────┤
│                 ALGORITMI                             │
│  ─────────────────────────                            │
│  DFS_Step()        — explorare cu backtracking        │
│  BFS_Shortest()    — drum cel mai scurt (flood fill)  │
│  BFS_Longest()     — drum cel mai lung (DFS brute)    │
│  Return_Home()     — întoarcere pe stiva DFS          │
│  Run_Path()        — urmează path calculat            │
├───────────────────────────────────────────────────────┤
│                 COMUNICARE BT                         │
│  ─────────────────────────                            │
│  Process_BT()  — parsare comenzi (polling UART)       │
│  BT_Map()      — transmite date hartă                 │
│  BT_Visual()   — transmite hartă ASCII vizuală        │
│  BT_Path()     — transmite drumul calculat            │
│  Manual_Cmd()  — comenzi mapare manuală               │
└───────────────────────────────────────────────────────┘
```

---

## Schema de Conexiuni

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

## Algoritmi Implementați

### 1. DFS (Depth-First Search) — Explorare

Algoritmul explorează labirintul preferând celule nevizitate. La fiecare celulă:

1. Scanează pereții cu cei 3 senzori ultrasonici
2. Actualizează harta internă (`maze_walls[][]`)
3. Caută vecini nevizitați (visit_count == 0)
4. Dacă găsește → salvează direcția de întoarcere pe stivă → merge acolo
5. Dacă nu → **BACKTRACK** = scoate de pe stivă, se întoarce
6. Când stiva e goală = **EXPLORARE COMPLETĂ**

```
Celula curentă: (2,3) orientat Nord
Senzori: F=25cm S=8cm D=45cm
Pereți detectați: [NW] (nord + vest)
Vecini liberi: Est(nevizitat), Sud(vizitat×1)
Decizie: → Est (preferă nevizitat)
Stivă: push(Vest) = direcția de întoarcere
```

### 2. BFS (Breadth-First Search) — Drum Cel Mai Scurt

Flood Fill de la start spre end pe harta cartografiată:

1. Inițializează toate celulele cu distanță -1
2. Start = distanță 0, adaugă în coadă
3. BFS: pentru fiecare celulă din coadă, explorează vecinii fără perete
4. Când ajunge la end → reconstruiește drumul prin `bfs_parent[][]`
5. Inversează drumul (e construit de la end la start)

### 3. Longest Path — Drum Cel Mai Lung

DFS brute-force care explorează toate căile posibile și o reține pe cea mai lungă:

1. Pornește de la start cu adâncime 0
2. La fiecare celulă, marchează ca vizitată și explorează recursiv vecinii
3. Când ajunge la end, compară adâncimea cu maximul cunoscut
4. La întoarcere, demarchează celula (backtracking)
5. Rezultat: cel mai lung drum fără bucle

### 4. Corecție Laterală — Mers Drept

În timpul mersului pe culoar:
- Dacă ambii pereți vizibili (<20cm): centrare proporțională cu diferența
- Dacă un perete <6cm: corecție agresivă (±8000 PWM)
- Safety: perete frontal <8cm → stop instant

---

## Comenzi Bluetooth

Aplicație recomandată: **Serial Bluetooth Terminal** (Android)

### Comenzi de pe butoane (single char)

| Comandă | Acțiune |
|---------|---------|
| `G` | Start explorare autonomă DFS |
| `X` | Stop urgență — oprește motoare + trimite harta |
| `M` | Afișează harta vizuală pe terminal |
| `P` | Calculează drumul cel mai SCURT (BFS) |
| `Q` | Calculează drumul cel mai LUNG |
| `R` | RUN — robotul urmează drumul calculat |
| `H` | Return Home — se întoarce pe stiva DFS |

### Comenzi tastate manual

| Comandă | Acțiune |
|---------|---------|
| `S2,3` + Send | Setează celula START la (2,3) |
| `E7,5` + Send | Setează celula END la (7,5) |
| `DN` + Send | Setează direcția robotului la Nord |
| `DE` + Send | Setează direcția robotului la Est |
| `DS` + Send | Setează direcția robotului la Sud |
| `DW` + Send | Setează direcția robotului la Vest |

### Comenzi mapare manuală

| Comandă | Acțiune |
|---------|---------|
| `F` | Forward — am împins robotul o celulă |
| `L` | Left — am rotit robotul 90° stânga |
| `B` | Back — am rotit robotul 180° |

### Configurare butoane app

```
M1 = G    M2 = M    M3 = X    M4 = P
M5 = R    M6 = H    M7 = Q
```

### Exemplu output Bluetooth

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

--- HARTA ---
+---+---+
|   | S |
+   +   +
| ^ |   |
+---+---+
Robot:(0,0)N Cells:3 Hall:9 Dist:70cm
```

---

## Configurare și Build

### Cerințe

- **STM32CubeIDE** (sau STM32CubeMX + arm-none-eabi-gcc)
- **Serial Bluetooth Terminal** (Android) — pentru comunicare BT
- Board **STM32F407G-DISC1** cu **ST-LINK V2** integrat

### Pași

1. **Clonează repository-ul:**
   ```bash
   git clone https://github.com/YOUR_USERNAME/maze-solver-robot.git
   ```

2. **Deschide în STM32CubeIDE:**
   - File → Import → Existing Projects into Workspace
   - Selectează folderul proiectului

3. **Build:**
   - Project → Build All (Ctrl+B)

4. **Flash:**
   - Run → Run As → STM32 C/C++ Application

5. **Conectare Bluetooth:**
   - Împerechează HC-05 cu telefonul (PIN: `1234`)
   - Deschide Serial Bluetooth Terminal
   - Conectează-te la HC-05
   - Configurează butoanele M1-M7

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

## Calibrare

Aceste valori trebuie ajustate pe labirintul real:

```c
#define SPEED_RIGHT  48000    // PWM motor dreapta (crește dacă deviează stânga)
#define SPEED_LEFT   50000    // PWM motor stânga
#define SPEED_TURN   30000    // PWM pentru viraje
#define TURN_90_MS   350      // Durata viraj 90° (ms)
#define TURN_180_MS  700      // Durata viraj 180° (ms)
#define CELL_DRIVE_MS 450     // Timp mers o celulă de 25cm (ms)
#define WALL_FRONT   13       // Prag detecție perete frontal (cm)
#define WALL_SIDE    6        // Prag corecție laterală (cm)
#define WALL_DETECT  12       // Prag mapare perete (cm)
```

### Procedură de calibrare

1. **Mers drept:** Pune robotul pe culoar drept, pornește cu G. Dacă deviază dreapta → crește `SPEED_RIGHT`. Dacă deviază stânga → crește `SPEED_LEFT`.

2. **Distanță celulă:** Măsoară câți cm parcurge la un `CELL_DRIVE_MS`. Ajustează până parcurge exact 25cm.

3. **Viraj 90°:** Pornește robotul, lasă-l să facă un viraj. Dacă virează prea mult → scade `TURN_90_MS`. Dacă prea puțin → crește.

4. **Praguri senzori:** Dacă detectează pereți care nu există → scade `WALL_DETECT`. Dacă nu detectează pereți existenți → crește.

---

## Structura Proiectului

```
maze-solver-robot/
├── README.md                    # Acest fișier
├── LICENSE                      # MIT License
├── docs/
│   ├── pinout.md               # Pinout detaliat
│   ├── algorithms.md           # Descriere algoritmi
│   └── calibration.md          # Ghid calibrare
├── src/
│   └── main.c                  # Codul sursă complet
├── hardware/
│   ├── wiring_diagram.md       # Schema de conexiuni
│   └── BOM.md                  # Bill of Materials
└── images/
    ├── robot_photo.jpg         # Poză robot
    ├── maze_map.jpg            # Exemplu hartă generată
    └── terminal_output.jpg     # Screenshot terminal BT
```

---

## Fluxul la Concurs

### Faza 1 — Explorare
```
1. Pune robotul în colțul labirintului, orientat Nord
2. Apasă PA0 sau trimite G pe BT
3. Robotul explorează automat tot labirintul
4. La final: >>> DONE! <<<
5. Trimite M → harta apare pe telefon
6. Arată juriului harta
```

### Faza 2 — Rezolvare
```
1. Juriul specifică: start=(2,3) stop=(7,5)
2. Tastezi pe telefon: S2,3 [Send]
3. Tastezi: E7,5 [Send]
4. Apeși P (shortest) sau Q (longest)
5. Pe ecran apare drumul calculat
6. Pui robotul în celula de start
7. Tastezi: DN [Send] (orientat Nord)
8. Apeși R → robotul urmează drumul automat
9. >>> AJUNS! <<<
```

---

## Demo

### Hartă generată de robot (output Bluetooth)
```
--- HARTA ---
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

## Echipa

Proiect dezvoltat în cadrul **ITEC Embedded Hackathon 2026** — Timișoara

---

## Licență

MIT License — vezi fișierul [LICENSE](LICENSE)
