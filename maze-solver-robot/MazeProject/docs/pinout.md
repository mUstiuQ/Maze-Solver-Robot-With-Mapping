# Pinout STM32F407G-DISC1 — Maze Robot

## Motoare (L298N)

| Pin STM32 | Funcție | Conectat la |
|-----------|---------|-------------|
| PB4 | TIM3_CH1 (PWM) | L298N ENA → Motor Dreapta |
| PB5 | TIM3_CH2 (PWM) | L298N ENB → Motor Stânga |
| PE7 | GPIO OUT | L298N IN1 |
| PE8 | GPIO OUT | L298N IN2 |
| PE9 | GPIO OUT | L298N IN3 |
| PE10 | GPIO OUT | L298N IN4 |

### Direcție motoare

| Acțiune | PE7 | PE8 | PE9 | PE10 |
|---------|-----|-----|-----|------|
| Înainte | 0 | 1 | 0 | 1 |
| Înapoi | 1 | 0 | 1 | 0 |
| Rotire stânga | 0 | 1 | 1 | 0 |
| Rotire dreapta | 1 | 0 | 0 | 1 |

### Conexiuni motor fizic
- Motor dreapta: fir roșu = OUT2, fir negru = OUT1
- Motor stânga: fir roșu = OUT3, fir negru = OUT4

## Senzori Ultrasonici (HC-SR04)

| Senzor | Trig | Echo | Notă |
|--------|------|------|------|
| Front | PC0 | PB0 | PB0 = 5V tolerant |
| Left | PC2 | PB1 | PB1 = 5V tolerant |
| Right | PC4 | PE6 | PE6 = 5V tolerant |

**Important:** HC-SR04 (nu varianta P) scoate 5V pe Echo. Pinii Echo trebuie să fie 5V tolerant!

## Hall Sensor (YS-27)

| Pin | Conectat la | Notă |
|-----|-------------|------|
| Signal | PA1 | Polling (nu interrupt) |
| VCC | 3.3V STM32 | NU 5V! |
| GND | GND | |

- 1 magnet pe roată, diametru 2.5cm
- Circumferință = π × 2.5 = 7.85cm per puls
- Polling: citim PA1, detectăm falling edge

## Bluetooth (HC-05)

| Pin STM32 | Pin HC-05 | Funcție |
|-----------|-----------|---------|
| PD5 (TX) | RXD | STM32 transmite → HC-05 |
| PD6 (RX) | TXD | HC-05 transmite → STM32 |
| - | VCC | 5V de la buck converter |
| - | GND | GND comun |

- USART2 @ 9600 baud
- HC-05 PIN: 1234
- Acceptă doar O SINGURĂ conexiune la un moment dat

## LED-uri (pe Discovery board)

| Pin | Culoare | Funcție |
|-----|---------|---------|
| PD12 | Verde | Perete față detectat |
| PD13 | Portocaliu | Perete stânga detectat |
| PD14 | Roșu | Perete dreapta detectat |
| PD15 | Albastru | Heartbeat / așteptare |

## Buton

| Pin | Funcție |
|-----|---------|
| PA0 | USER button — START/STOP toggle |

Reconfigurat în cod ca GPIO_INPUT cu PULLDOWN (CubeMX îl setează ca EVT_RISING).
