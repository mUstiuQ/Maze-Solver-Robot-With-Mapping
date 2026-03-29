# Algoritmi — Maze Solver Robot

## 1. DFS (Depth-First Search) — Explorare Autonomă

### Descriere
Robotul explorează labirintul celulă cu celulă, preferând mereu celule nevizitate. Când ajunge într-o fundătură, se întoarce automat (backtracking) folosind o stivă.

### Structuri de date
```c
uint8_t maze_walls[16][16];   // Pereții fiecărei celule (biți: N=0x01, E=0x02, S=0x04, W=0x08)
uint8_t maze_visit[16][16];   // Contor vizite (0=necunoscut, 1+=vizitat)
uint8_t dfs_stack[256];       // Stiva direcțiilor de întoarcere
```

### Pseudocod
```
DFS_Step():
    1. Scanează senzorii → actualizează maze_walls[x][y]
    2. Marchează celula ca vizitată
    3. Pentru fiecare direcție (N, E, S, W):
        - Dacă nu e perete ȘI vecinul e nevizitat → candidat
    4. Dacă există candidat:
        - Push pe stivă direcția opusă (de întoarcere)
        - Rotește robotul spre candidat
        - Merge o celulă
    5. Dacă NU există candidat dar stiva nu e goală:
        - Pop de pe stivă → direcția de backtrack
        - Rotește și merge înapoi
    6. Dacă stiva e goală:
        - EXPLORARE COMPLETĂ
```

### Complexitate
- Timp: O(V + E) unde V = celule, E = pasaje
- Spațiu: O(V) pentru stivă
- Garantează vizitarea tuturor celulelor accesibile

## 2. BFS (Breadth-First Search) — Drum Cel Mai Scurt

### Descriere
Flood Fill de la celula de start. Fiecare celulă primește o distanță (număr de pași de la start). Drumul se reconstruiește urmând părinții de la end la start.

### Pseudocod
```
BFS_Shortest(start, end):
    1. flood[start] = 0, adaugă start în coadă
    2. Cât timp coada nu e goală:
        a. Scoate celula curentă din coadă
        b. Dacă e END → reconstruiește drumul → return
        c. Pentru fiecare vecin fără perete:
            - Dacă nevizitat în flood: flood[vecin] = flood[curent] + 1
            - Salvează parent[vecin] = curent
            - Adaugă în coadă
    3. Reconstrucție: urmează parent[] de la end la start, inversează
```

### Complexitate
- Timp: O(V + E)
- Spațiu: O(V) pentru coadă + flood + parent
- Garantează drumul cel mai scurt

## 3. Longest Path — Drum Cel Mai Lung

### Descriere
DFS brute-force care explorează TOATE căile posibile de la start la end și o reține pe cea mai lungă. Folosește backtracking pentru a încerca fiecare ramură.

### Pseudocod
```
DFS_Longest(cx, cy, end, depth):
    1. Dacă (cx,cy) == end:
        - Dacă depth > best_length → salvează drumul curent
        - Return
    2. Pentru fiecare vecin fără perete și nevizitat:
        - Marchează vizitat
        - DFS_Longest(vecin, end, depth+1)
        - Demarchează (backtrack)
```

### Complexitate
- Timp: O(V!) worst case (NP-hard general, dar labirintul e mic)
- Spațiu: O(V) pentru stiva recursivă
- Pe un labirint 10×10 (100 celule) rulează instant

## 4. Corecție Laterală — Mers Drept

### Descriere
În timpul mersului pe culoar, robotul citește senzorii laterali și ajustează vitezele motoarelor pentru a rămâne centrat.

### Logică
```
Dacă ambii pereți vizibili (< 20cm):
    eroare = distanța_stânga - distanța_dreapta
    Dacă eroare > 2:   încetinește dreapta, accelerează stânga
    Dacă eroare < -2:  încetinește stânga, accelerează dreapta
    
Dacă doar un perete < 6cm:
    Corecție agresivă spre direcția opusă (±8000 PWM)

Safety: perete frontal < 8cm → STOP instant
```

## 5. Mapare Pereți

### Conversia relativ → absolut
Senzorii citesc față/stânga/dreapta relativ la orientarea robotului. Conversia la N/E/S/W absolut:

```
abs_front = robot_dir
abs_left  = (robot_dir + 3) % 4
abs_right = (robot_dir + 1) % 4
```

### Propagare pereți
Un perete este partajat între două celule. Dacă celula (2,3) are perete Nord, atunci celula (2,4) are perete Sud:

```
if (walls & WALL_N) maze_walls[x][y+1] |= WALL_S;
if (walls & WALL_S) maze_walls[x][y-1] |= WALL_N;
if (walls & WALL_E) maze_walls[x+1][y] |= WALL_W;
if (walls & WALL_W) maze_walls[x-1][y] |= WALL_E;
```
