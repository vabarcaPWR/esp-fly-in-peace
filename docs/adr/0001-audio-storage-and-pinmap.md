# ADR 0001 — Almacenamiento de audio, GPS y mapa de pines (ESP32-C3 Zero)

> Fecha: 2026-04-22
> Estado: **Aceptada**
> Ámbito: firmware (micro), hardware
> Autores: @vabarcaPWR
> Reemplaza / actualiza: targeting previo a "ESP32-C3 Super Mini" en `docs/roadmap.micro.md`
> y en `docs/architecture/firmware-architecture.md` §11.

---

## 1. Contexto

El firmware del vario pasa a tener como **target oficial el ESP32-C3 Zero**
(Waveshare). Se mantiene el SoC ESP32-C3FN4/FH4 (RISC-V single-core 160 MHz,
400 KB SRAM, 4 MB flash, BLE 5.0), por lo que el código C y la arquitectura
factory/backend no cambian — solo cambia el mapa de pines y algunos valores
por defecto de Kconfig.

El Zero expone **15 GPIOs** (`0–10, 18, 19, 20, 21`) frente a los 13 del
Super Mini (`18/19` no accesibles). Esos 2 pines extra son la clave para
poder alojar simultáneamente:

1. **Sensores I2C** (BMP390 + MPU6050): 2 pines
2. **Amplificador MAX98357A** vía I2S: 3 pines + **1 pin de SHDN** (recuperado)
3. **LED WS2812** (integrado en el propio módulo Zero): 1 pin
4. **Almacenamiento de audio** en flash SPI NOR **W25Q128 (16 MB)**: 4 pines
5. **GPS Quectel M100 Mini** por UART1 NMEA 9600 baud: 2 pines
6. **Botón BOOT/usuario**: 1 pin

Total pines usados: **14 de 15**. Margen: 1 pin libre para futuras E/S.

## 2. Decisión

### 2.1 Target oficial: **ESP32-C3 Zero** (Waveshare)

- WS2812 integrada en placa en GPIO 8 → se reutiliza la del módulo.
- Antena PCB cerámica, RF más consistente que el Super Mini.
- 2 GPIOs extra (`18`, `19`) disponibles si se renuncia a la consola USB-CDC.
- El **Super Mini queda como plan B** para prototipado de bajo coste; funciona
  con un mapa reducido (sin SHDN) ya documentado en versiones previas.

### 2.2 Almacenamiento de audio: **W25Q128 (flash SPI NOR, 16 MB)**

Descartada la µSD por fragilidad mecánica del conector, latencias de GC variables
(50–250 ms) y consumo pico alto. La NOR soldada al PCB ofrece:

- Lecturas deterministas (~60 µs por página de 256 B).
- ~5 MB/s sostenidos a 40 MHz SPI estándar → margen ×100 sobre el audio.
- Consumo en lectura ~15 mA, deep-power-down <1 µA.
- Ring buffer de 4 KB suficiente (vs ≥16 KB con µSD).

### 2.3 **Recuperación del pin `SHDN` del MAX98357A** (`GPIO 1`)

Posible en el Zero **al disponer de `GPIO 18/19`** como GPIO libres. Esos dos
pines se asignan al UART del GPS, liberando del compromiso anterior que exigía
renunciar a SHDN.

### 2.4 Consola de desarrollo: vía **UART0 externo** + **BLE debug log**

Al asignar `GPIO 18/19` al GPS, se deshabilita la consola USB-CDC en tiempo de
ejecución. Contrapartidas y mitigaciones:

- **Flasheo**: se usa el bootloader ROM reclamando `18/19` para USB mediante
  `BOOT + RST` (modo download). Procedimiento estándar en ESP32-C3, no requiere
  hardware extra; el Zero mantiene ambos botones accesibles.
- **Logs en desarrollo**: se redirige la consola a **UART0 TX por `GPIO 20`**
  (RX no necesario para logs) mediante un cable USB-UART externo a 115200 baud.
- **Logs en campo**: canal de debug sobre **BLE NUS** (característica TX ya
  existente) con un nivel de log filtrado (`ESP_LOG_WARN` hacia arriba, o
  activable por comando de la app).

`sdkconfig` pasará a:

```
CONFIG_ESP_CONSOLE_UART_DEFAULT=y
CONFIG_ESP_CONSOLE_UART_NUM=0
CONFIG_ESP_CONSOLE_UART_TX_GPIO=20
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=n
```

### 2.5 Nuevo mapa de pines (ESP32-C3 Zero)

| GPIO | Función | Dir. | Strapping | Notas |
|------|---------|------|-----------|-------|
| 0 | **Flash SPI MOSI** | OUT | sí (boot=HIGH) | pullup 10 kΩ externo en DI del W25Q128 |
| 1 | **MAX98357A SHDN** ✓ recuperado | OUT | — | LOW = mute / deep-sleep del amp |
| 2 | I2S BCLK | OUT | sí | pullup 10 kΩ externo (requerido por strapping) |
| 3 | I2S WS | OUT | — | |
| 4 | I2S DOUT | OUT | — | |
| 5 | **Flash SPI SCK** | OUT | — | |
| 6 | I2C SDA | bidi | — | sin cambios |
| 7 | I2C SCL | OUT | — | sin cambios |
| 8 | WS2812 DIN | OUT | sí (HIGH en boot) | LED integrado del Zero |
| 9 | BOOT / botón usuario | IN | sí | pullup interno; BOOT + RST para flasheo |
| 10 | **Flash SPI MISO** | IN | — | pullup interno |
| 18 | **GPS UART1 RX** (GPS→ESP) | IN | — | antes USB D- |
| 19 | **GPS UART1 TX** (ESP→GPS) | OUT | — | antes USB D+ |
| 20 | **UART0 TX (consola dev, opcional)** | OUT | — | 115200 baud hacia adaptador USB-UART externo |
| 21 | **Flash SPI CS** | OUT | — | |

Resumen: **14 / 15 GPIOs usados**, 1 pin libre (`GPIO 20` reservado para
consola de desarrollo; en producción puede quedar sin conectar o servir para
un periférico futuro, p. ej. interrupción EXTI del sensor).

Si en producción se prefiere dejar `GPIO 20` completamente libre, la consola
queda solo por BLE NUS.

## 3. Alternativas consideradas

### A. Mantener Super Mini + renunciar a SHDN

Discutido en versión previa del ADR (commit anterior). Solo 13 GPIOs útiles
obligaban a atar SHDN a 3V3 permanentemente (~2–3 mA extra en idle, sin
posibilidad de deep-sleep real del amplificador). **Descartado** ahora que
el Zero es el target.

### B. Mantener USB-CDC nativo en el Zero

Ocuparía `18/19` para consola y dejaría el proyecto en la misma situación
que el Super Mini (13 útiles, sin SHDN). **Descartado**: pierde el beneficio
principal del Zero.

### C. Flash SPI en modo QSPI (4 líneas)

+40 MB/s pero requiere 6 pines. No cabe con GPS + botón + I2S + I2C + LED.
**Descartado**: el audio no necesita ese ancho de banda (WAV estéreo 44.1 kHz
16-bit = 176 kB/s; SPI estándar a 40 MHz ya entrega ~5 MB/s).

### D. Consola USB-CDC conmutable en runtime

El C3 no permite compartir `18/19` entre USB-JTAG y GPIO simultáneamente.
Podría alternarse con un stub, pero añade complejidad y bugs potenciales
durante el switch. **Descartado**.

## 4. Consecuencias

### Positivas

- **SHDN del MAX98357A controlable por software** → deep-sleep real del
  amplificador (~0.5 µA vs 2–3 mA). Mejora autonomía.
- **Flash NOR soldada**: robustez mecánica (sin conector µSD), latencias
  deterministas, reproducción de audio sin cortes.
- **WS2812 integrada** en el Zero → un componente menos en el PCB final.
- **Antena RF cerámica** del Zero → alcance BLE más predecible con XCTrack.
- **Arquitectura sin cambios**: mismos componentes factory/backend, solo
  nuevos subcomponentes (`bus_drivers/spi`, `storage/flash_ext`, `audio_bank`,
  `gps`) que encajan en el patrón existente.
- **Margen de 1 pin** para futuras ampliaciones.

### Negativas

- **Pérdida de consola USB-CDC** en runtime. Flasheo requiere `BOOT + RST`
  (procedimiento estándar, no añade hardware). Logs en dev via UART0 externo
  o BLE.
- **Cambio de target** implica ajustar `sdkconfig.defaults`, documentación,
  y scripts de `flash/monitor`. Impacto acotado.
- **Capacidad de audio 16 MB** (no cambia respecto a la versión anterior):

  | Formato | Bitrate | Duración en 16 MB |
  |---------|---------|-------------------|
  | WAV mono 16 kHz 16-bit | 32 kB/s | ~8 min |
  | WAV mono 22.05 kHz 16-bit | 44 kB/s | ~6 min |
  | ADPCM IMA 4-bit mono 22 kHz | 11 kB/s | ~24 min |
  | MP3 64 kbps mono | 8 kB/s | ~32 min |
  | Opus 24 kbps mono | 3 kB/s | ~85 min |

- **Actualización de contenido** requiere flasheo por bootloader o BLE OTA a
  partición dedicada (no hay tarjeta extraíble).

- **GPIO 0 como MOSI** obliga a pullup externo en DI de la flash para garantizar
  boot normal.

## 5. Plan de integración (resumen, no vinculante)

1. Actualizar `micro/sdkconfig.defaults`:
   - `CONFIG_ESP_CONSOLE_UART_DEFAULT=y`, TX = GPIO 20, USB-JTAG = `n`.
   - Nuevos Kconfigs para flash SPI, GPS UART y SHDN (ya existente en `sound`).
2. Añadir subcomponente `bus_drivers/spi/` (patrón infraestructura compartida).
3. Nuevo componente `storage/flash_ext` con patrón factory/backend:
   - Backend `w25q128` (conductor + model + hardware).
   - API: `flash_ext_read/write_sector/erase_sector`.
4. Nuevo componente `audio_bank` sobre `flash_ext`:
   - Tabla de offsets + descriptores de clips al inicio de la flash.
   - API: `audio_bank_get_clip(id)` → `{offset, len, format}`.
5. Extender backend `sound/max98357` con modo "clip playback" alimentado por
   ring buffer de 4 KB desde `audio_bank`.
6. Nuevo componente `gps` con patrón factory/backend:
   - Backend `m100_mini` sobre UART1 (GPIOs 18/19).
   - Parser NMEA mínimo (GGA + RMC) en el modelo.
7. Canal BLE de logs: nueva característica GATT (o reuso de NUS TX con prefijo
   de nivel) para `ESP_LOG_*` filtrado.
8. Actualizar `docs/architecture/firmware-architecture.md` §11 (tabla Zero).
9. Actualizar `docs/roadmap.micro.md` (target y fases nuevas: flash_ext,
   audio_bank, gps, BLE log).
10. Revisar `scripts/micro/flash.sh` y `monitor.sh` para el nuevo flujo
    (download mode + UART externo opcional).

Formatos de audio recomendados:
- **Alarmas cortas y tonos**: PCM 16-bit 22.05 kHz mono (WAV raw).
- **Voces largas**: ADPCM IMA 4-bit 22 kHz (decoder trivial).
- **MP3 descartado** en MVP (30 KB RAM + 20–30 % CPU con Helix, sin beneficio
  claro para este volumen de contenido).

## 6. Riesgos residuales

| Riesgo | Mitigación |
|--------|-----------|
| Contención CPU (BLE + I2S + GPS + EKF en single core) | Prioridades: I2S=10, EKF=6, GPS=5, BLE sender=4. Tarea lectora de `flash_ext` bloqueante con prioridad media. |
| Acoplo digital SPI ↔ I2S en el PCB | Separar masas, desacoplo local en flash y MAX98357, layout compacto. |
| GPIO 0 leído LOW en boot | Pullup 10 kΩ obligatorio en DI de la flash. Verificar con osciloscopio en bring-up. |
| Flasheo menos cómodo (BOOT+RST) | Documentar procedimiento; opcional: pad de test para hold automático con PCB adapter. |
| Sin logs por USB en campo | Canal BLE de debug; UART0 en `GPIO 20` expuesto como test-point para dev. |
| Ruido del rail en escritura flash | 100 nF + 10 µF junto al W25Q128. |

## 7. Criterios de aceptación

- Sistema arranca con la nueva asignación sin entrar en modo download.
- BMP390, MPU6050, WS2812, MAX98357A (con SHDN controlable), W25Q128 y GPS
  M100 Mini coexisten operativos simultáneamente.
- `SHDN = LOW` reduce el consumo del amplificador al valor de datasheet
  (medido con el sistema en idle BLE conectado).
- Reproducción de un clip WAV 22 kHz mono de 5 s sin cortes audibles mientras
  BLE notifica LK8EX1 a 8 Hz.
- GPS entrega fix NMEA estable a 9600 baud sin pérdida de tramas durante audio.
- Canal BLE de debug log entrega `ESP_LOG_WARN` y `_ERROR` a la app Flutter.
- Flasheo OTA por USB (modo download via BOOT+RST) verificado en ≤ 3 min para
  binario completo.

---

**Referencias cruzadas**:
- [ADR 0002 — Audio pipeline, pitch-shift & mixing](./0002-audio-pipeline-pitch-shift.md)
- `docs/architecture/firmware-architecture.md` §11 Hardware Mapping (pendiente actualizar)
- `docs/roadmap.micro.md` header + fases de integración
- Datasheet W25Q128JVSIQ (Winbond)
- Datasheet Quectel M100 Mini
- ESP32-C3 TRM §5 (USB-JTAG) y §6 (GPIO & IO MUX)
