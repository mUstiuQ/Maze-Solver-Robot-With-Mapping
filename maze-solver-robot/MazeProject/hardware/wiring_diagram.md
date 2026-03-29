# Schema de Conexiuni (Wiring Diagram)

## Diagrama generală

```
    ┌──────────────────────────────────────────┐
    │            6×AA BATERII (9V)              │
    └──────────┬───────────────┬────────────────┘
               │               │
               ▼               ▼
    ┌──────────────┐   ┌──────────────┐
    │   L298N      │   │ Buck Conv.   │
    │   VCC=9V     │   │ IN=9V        │
    │              │   │ OUT=5V       │
    │  ENA ← PB4  │   └──┬─────┬────┘
    │  ENB ← PB5  │      │     │
    │  IN1 ← PE7  │      ▼     ▼
    │  IN2 ← PE8  │   ┌─────┐ ┌──────┐
    │  IN3 ← PE9  │   │STM32│ │HC-05 │
    │  IN4 ← PE10 │   │5V   │ │VCC=5V│
    │              │   │     │ │      │
    │ OUT1─┐ OUT3─┐│   │PD5──┼─┤RXD   │
    │ OUT2─┤ OUT4─┤│   │PD6──┼─┤TXD   │
    └──────┤──────┤┘   │     │ │      │
           │      │    │     │ └──────┘
       Motor R  Motor L│     │
                       │     │
    ┌──────┐ ┌──────┐ │PA1  │
    │HC-SR04│ │HC-SR04│ │◄───┤ YS-27 Hall
    │FRONT │ │LEFT  │ │     │ (3.3V!)
    │Trig←PC0 Trig←PC2     │
    │Echo→PB0 Echo→PB1     │
    └──────┘ └──────┘      │
    ┌──────┐               │
    │HC-SR04│               │
    │RIGHT │               │
    │Trig←PC4              │
    │Echo→PE6              │
    └──────┘               │
                    ┌──────┘
                    │PA0 = USER BUTTON (START/STOP)
                    │PD12-15 = LED-uri status
                    └──────
```

## Conexiuni detaliate

### L298N → STM32
```
L298N Pin    →    STM32 Pin    Funcție
─────────────────────────────────────
ENA          →    PB4          PWM motor dreapta (TIM3_CH1)
ENB          →    PB5          PWM motor stânga (TIM3_CH2)
IN1          →    PE7          Direcție motor dreapta
IN2          →    PE8          Direcție motor dreapta
IN3          →    PE9          Direcție motor stânga
IN4          →    PE10         Direcție motor stânga
GND          →    GND          Masă comună
```

### HC-SR04 → STM32
```
Senzor      Trig    Echo    VCC    GND
─────────────────────────────────────
Front       PC0     PB0     5V     GND
Left        PC2     PB1     5V     GND
Right       PC4     PE6     5V     GND
```

### HC-05 → STM32
```
HC-05 Pin    →    STM32 Pin
──────────────────────────
TXD          →    PD6 (USART2 RX)
RXD          →    PD5 (USART2 TX)
VCC          →    5V (buck converter)
GND          →    GND
```

### YS-27 → STM32
```
YS-27 Pin    →    STM32 Pin
──────────────────────────
Signal       →    PA1
VCC          →    3.3V (STM32)
GND          →    GND
```

## Atenționări

1. **HC-SR04 Echo = 5V!** Pinii Echo TREBUIE să fie 5V tolerant (PB0, PB1, PE6)
2. **L298N 5V jumper SCOS** — alimentăm extern prin buck converter
3. **YS-27 pe 3.3V** — NU conecta la 5V
4. **GND comun** — toate modulele trebuie să aibă masă comună
5. **HC-05 acceptă doar O conexiune** — telefonul SAU laptopul, nu ambele
