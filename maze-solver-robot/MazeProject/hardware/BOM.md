# Bill of Materials (BOM)

| # | Componentă | Model | Cantitate | Rol |
|---|-----------|-------|-----------|-----|
| 1 | Microcontroller | STM32F407G-DISC1 | 1 | Placă de bază, Cortex-M4 168MHz |
| 2 | Driver motoare | L298N Dual H-Bridge | 1 | Control PWM motoare DC |
| 3 | Motor DC cu reductor | Din kit robot | 2 | Tracțiune diferențială |
| 4 | Senzor ultrasonic | HC-SR04 | 3 | Detecție pereți (față, stânga, dreapta) |
| 5 | Senzor Hall | YS-27 | 1 | Odometrie — rotații roată |
| 6 | Magnet | Neodimiu mic | 1 | Montat pe roată pentru Hall |
| 7 | Modul Bluetooth | HC-05 Master/Slave | 1 | Comunicare wireless cu telefonul |
| 8 | Baterii | AA 1.5V | 6 | Alimentare 9V |
| 9 | Buck converter | LM2596 sau similar | 1 | Conversie 9V → 5V |
| 10 | Breadboard | Standard | 1 | Conexiuni temporare |
| 11 | Fire jumper | M-M, M-F | ~30 | Conexiuni |
| 12 | Șasiu robot | Kit 2 motoare | 1 | Platformă + roți |
| 13 | Suport baterii | 6×AA | 1 | Alimentare |

## Notă alimentare

- L298N primește 9V direct de la baterii pe VCC
- Jumperul de 5V de pe L298N: **SCOS**
- Buck converter: alimentat de la 9V, output 5V → STM32, senzori, HC-05
- YS-27 Hall: alimentat de la 3.3V STM32 (NU 5V)
