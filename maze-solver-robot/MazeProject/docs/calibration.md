# Ghid de Calibrare — Maze Solver Robot

## Parametri de calibrat

Toți parametrii sunt `#define` în secțiunea `USER CODE BEGIN PD` din `main.c`.

## 1. Viteze Motoare

```c
#define SPEED_RIGHT  48000    // Motor dreapta (CH1)
#define SPEED_LEFT   50000    // Motor stânga (CH2)
```

**Test:** Pune robotul pe podea dreaptă, pornește cu G.
- Deviază spre dreapta → crește `SPEED_RIGHT` cu 2000
- Deviază spre stânga → crește `SPEED_LEFT` cu 2000
- Repetă până merge drept pe 2-3 metri

## 2. Distanță Celulă

```c
#define CELL_DRIVE_MS  450    // Milisecunde mers pentru o celulă
```

**Test:** Pune robotul pe un culoar drept, marchează 25cm pe podea.
- Trimite G, observă cât parcurge la prima celulă
- Depășește 25cm → scade cu 50
- Nu ajunge la 25cm → crește cu 50

## 3. Viraj 90°

```c
#define TURN_90_MS   350      // Milisecunde rotire 90°
#define SPEED_TURN   30000    // Viteza de rotire
```

**Test:** Pune robotul drept, marchează direcția pe podea.
- Trimite G, lasă-l să facă un viraj (la perete frontal)
- Virează mai mult de 90° → scade `TURN_90_MS` cu 25
- Virează mai puțin de 90° → crește cu 25

## 4. Viraj 180°

```c
#define TURN_180_MS  700      // Milisecunde rotire 180°
```

De obicei = `TURN_90_MS × 2`. Ajustează independent dacă e nevoie.

## 5. Praguri Senzori

```c
#define WALL_FRONT   13       // Sub 13cm = perete în față (se oprește)
#define WALL_SIDE    6        // Sub 6cm = prea aproape lateral (corectează)
#define WALL_DETECT  12       // Sub 12cm = considerat perete pentru mapare
```

**Test:** Cu robotul oprit, ține mâna la 12cm de senzorul frontal.
- Trimite M, verifică dacă apare perete detectat
- Dacă detectează pereți inexistenți → scade `WALL_DETECT`
- Dacă nu detectează pereți existenți → crește `WALL_DETECT`

## Ordinea de calibrare recomandată

1. **Viteze motoare** — robotul trebuie să meargă drept pe podea liberă
2. **Viraj 90°** — trebuie să fie exact 90°
3. **Distanță celulă** — trebuie să parcurgă exact 25cm
4. **Praguri senzori** — trebuie să detecteze pereții corect
5. **Test integrat** — pune în labirint, trimite G, observă comportamentul
