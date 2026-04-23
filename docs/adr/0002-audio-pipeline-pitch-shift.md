# ADR 0002 — Pipeline de audio: banco en flash, mezcla y pitch-shift

> Fecha: 2026-04-23
> Estado: **Aceptada**
> Ámbito: firmware (micro), componente `sound` + nuevos `storage/flash_ext` y `audio_bank`
> Autores: @vabarcaPWR
> Depende de: [ADR 0001 — Audio storage, GPS & pin map](./0001-audio-storage-and-pinmap.md)

---

## 1. Contexto

El ADR 0001 cierra el hardware de audio: MAX98357A conectado por I2S al ESP32-C3
Zero, con `SHDN` controlable (`GPIO 1`) y banco de audio soldado como flash SPI
NOR W25Q128 (16 MB). Queda por decidir **cómo organizar la capa de software**
que reproduce contenido, cómo mezclarlo con los tonos sintéticos del vario ya
existentes y qué capacidades de pitch-shift se ofrecen.

Requisitos del sistema:

- **Tonos del vario** (generados en runtime): onda sinusoidal/cuadrada con
  pitch y cadencia variables en función de `vario_cms`. Ya implementados en
  `sound/src/max98357/` (Phase 10.5, backend I2S + `tone_model`).
- **Alarmas pregrabadas** cortas (<1 s): "ganando altura", "corriente detectada",
  "batería baja".
- **Voces pregrabadas** más largas: lectura de altitud/velocidad vertical,
  avisos de navegación cuando se integre el GPS.
- **Jingles** de bienvenida / despedida / pairing.
- **Mezcla** entre tonos del vario y clips (el beep no debe silenciarse del
  todo durante una alarma; en el peor caso, atenuarse).
- **Pitch variable** para dar variedad expresiva a voces y alarmas (ej.
  reproducir la alarma de ascenso un tono más agudo que la de descenso, o
  pronunciar números con entonación).

Restricciones:

- ESP32-C3 single-core @160 MHz, sin FPU. Comparte núcleo con BLE NimBLE,
  EKF, AHRS, GPS parser y tareas de sensores.
- RAM usable para nuevo código: ~100 KB libres según §9.1 del architecture doc.
- Latencia de arranque de un clip ≤ 50 ms (alarmas).
- Reproducción **sin cortes** (la flash NOR con ring buffer de 4 KB ya lo
  garantiza, ver ADR 0001 §2.2).

## 2. Decisión

### 2.1 Arquitectura del pipeline de audio

Cadena productor-consumidor con mezcla en punto flotante escalada a int16:

```
┌───────────────┐   ┌─────────────┐   ┌──────────────┐   ┌───────────┐
│  tone_model   │──►│             │   │              │   │           │
│ (vario beep)  │   │             │   │              │   │           │
├───────────────┤   │   mixer     │──►│   limiter    │──►│  I2S DMA  │──► MAX98357A
│ clip_player   │──►│ (float Q1.15)│  │ (soft-clip)  │   │  (int16)  │
│ + pitch_shift │   │             │   │              │   │           │
└───────▲───────┘   └─────────────┘   └──────────────┘   └───────────┘
        │
        ▼
┌───────────────┐   ┌─────────────┐
│  audio_bank   │──►│ flash_ext   │──► SPI2 (W25Q128)
│ (clip index)  │   │ (W25Q128 drv)│
└───────────────┘   └─────────────┘
```

### 2.2 Nuevos componentes

Siguiendo el patrón factory/backend ya establecido (ver
`docs/architecture/firmware-architecture.md` §2).

#### `bus_drivers/spi/` (infraestructura compartida)

- Sin factory ni Kconfig de selección.
- API: `spi_bus_init()`, `spi_bus_get_host()`.
- Host: `SPI2_HOST` (FSPI). Pines definidos por Kconfig (ver ADR 0001 §2.5).

#### `storage/flash_ext/` (factory/backend)

- Contrato: `flash_ext_t` con `{init, read, write_page, erase_sector, get_size,
  get_name}`.
- Backend inicial: `w25q128` dividido en conductor + model + hardware.
  - `hardware`: comandos SPI (`0x03` read, `0x02` page program, `0x20` 4 KB
    erase, `0xB9` power-down, `0xAB` release power-down, `0x9F` JEDEC ID).
  - `model`: validación de direcciones, chequeo de sectores, cálculo de
    tiempos de espera tras escrituras/borrados.
  - `conductor`: orquesta inicialización del bus, verificación de JEDEC ID
    al arranque, gestión del pin de power-down.
- Partición lógica en la flash (sin FAT/LittleFS para el MVP):

  ```
  0x000000 ─ 0x000FFF  (4 KB)   : header mágico + versión + CRC
  0x001000 ─ 0x001FFF  (4 KB)   : tabla de descriptores de clips
  0x002000 ─ 0xFFFFFF (~15.99 MB): payload de clips (raw PCM / IMA ADPCM)
  ```

#### `audio_bank/` (factory/backend sobre `flash_ext`)

- Lee el header y la tabla de descriptores al arranque, los cachea en RAM
  (~1 KB para 64 clips).
- Descriptor por clip:

  ```c
  typedef struct {
      uint8_t  clip_id;         // identificador lógico
      uint32_t offset;          // offset en flash (bytes)
      uint32_t length;          // longitud en bytes
      uint16_t sample_rate_hz;  // frecuencia base de grabación
      uint8_t  format;          // 0=PCM16, 1=IMA-ADPCM
      uint8_t  flags;           // bit0 = has_psola_markers
      uint32_t markers_offset;  // offset tabla PSOLA (0 si no aplica)
      uint16_t markers_count;   // número de marcadores
      uint8_t  reserved[2];
  } audio_clip_desc_t;          // 20 bytes
  ```

- API:
  - `audio_bank_get(uint8_t id, audio_clip_desc_t *out)`
  - `audio_bank_open_stream(uint8_t id, audio_stream_t **out)` — abre un
    lector con ring buffer de 4 KB propio del clip.

#### Extensión de `sound/src/max98357/`

Nuevas subcarpetas dentro del backend existente (respetando conductor-model-hardware):

- `clip_player/` — model puro: decodificador PCM16 passthrough + decodificador
  IMA-ADPCM (table lookup, sin coma flotante). Testable con Ceedling.
- `pitch/` — model puro: implementaciones seleccionables por Kconfig.
- `mixer/` — model puro: mezcla lineal float → saturación.

### 2.3 Estrategia de pitch-shift

Se ofrecen **dos métodos complementarios**, activables por flags del clip y por
llamada al reproducir:

#### Método A — Re-sampling por cambio de tasa I2S (default)

- Multiplica la frecuencia de lectura del clip por un factor de pitch.
- El reloj I2S no se cambia (se mantiene el sample rate de salida fijo); lo que
  cambia es el **avance del puntero de lectura** en el clip (interpolación lineal
  entre samples).
- **Cambia pitch y tempo a la vez.** Ideal para clips cortos (<1 s) donde el
  cambio de duración es imperceptible o deseable (jingles).
- Coste CPU: ~0.5 % por clip activo.
- Rango práctico: 0.5× a 2.0× (±1 octava) sin artefactos audibles.

#### Método B — WSOLA (Waveform-Similarity Overlap-Add) en tiempo fijo

- Mantiene el tempo original alterando solo el pitch.
- Implementación clásica de 2 KLOC en C puro, ventana 20–40 ms, solapamiento
  50%.
- Coste CPU: ~10–15 % a 22 kHz mono en C3 @160 MHz.
- Rango útil: ±6 semitonos sin degradación audible.
- Activable por Kconfig (`CONFIG_SOUND_PITCH_WSOLA=y`). Por defecto **off** en
  el MVP; se activa cuando el contenido lo justifique (voces largas).

#### Método C — TD-PSOLA (descartado para MVP)

Mejor calidad en voz pero requiere anotar offline los picos de pitch en cada
clip. Se **deja como extensión futura** si se demuestra que el banco de voces
crece y la calidad con WSOLA es insuficiente. El campo `markers_offset` del
descriptor ya reserva espacio.

### 2.4 Mezcla de tonos del vario + clips

- El `mixer` suma en float dos fuentes: `tone_model` (beep del vario) + hasta
  2 clips simultáneos.
- Al dispararse un clip con prioridad **alta** (alarma): el `tone_model` se
  atenúa a **-12 dB** durante la reproducción. Al terminar, recupera nivel
  en 100 ms (rampa lineal).
- Al dispararse un clip con prioridad **baja** (ambient, info): sin atenuación
  del beep.
- El limitador final aplica soft-clipping (`tanh`) para evitar saturación en
  int16.

### 2.5 Gestión del pin `SHDN`

- `SHDN = HIGH` cuando haya **cualquier fuente activa** (tono o clip).
- `SHDN = LOW` tras **500 ms de silencio total** (ahorro ~5 mA, ver tabla de
  consumos en roadmap §Phase 12).
- Transiciones `LOW → HIGH` preceden a I2S en **≥ 5 ms** (tiempo de
  estabilización del charge-pump del MAX98357A, per datasheet).

## 3. Alternativas consideradas

### A. Filesystem completo (FATFS o LittleFS) sobre la flash

Pros: flexibilidad para añadir/quitar clips sin reflasheo de la tabla.
Contras: ~30 KB de RAM extra en runtime (FATFS buffers), overhead de lectura,
API más compleja para un caso de uso de solo-lectura. **Descartado**: el banco
es sustancialmente estático; actualizaciones son raras (BLE OTA o reflasheo).

### B. Phase-vocoder por FFT para pitch-shift de máxima calidad

40 % CPU en C3, incompatible con BLE + EKF + GPS activos. **Descartado**.

### C. Solo pitch-shift por re-sampling (método A únicamente)

Más simple, pero obliga a que todas las voces largas cambien de duración cuando
se haga pitch-shift. **No** cumple el requisito de "reproducir números con
entonación manteniendo la cadencia". Por eso se incluye el método B.

### D. Códec hardware externo (VS1053, WM8731...)

Descargaría CPU y añadiría control de volumen hardware. **Descartado** por:
coste, pines adicionales, complejidad de PCB. El C3 tiene margen para el
pipeline software propuesto.

### E. MP3 decoding

Helix MP3 usa ~30 KB RAM y 20–30 % CPU a 128 kbps. Para alarmas cortas y
voces, IMA-ADPCM (4 KB RAM, <5 % CPU, compresión 4:1) es suficiente y más
eficiente. **Descartado** para MVP. Se puede reevaluar si el banco crece a
contenido largo tipo música.

## 4. Consecuencias

### Positivas

- Arquitectura compatible con el patrón factory/backend: `flash_ext`,
  `audio_bank`, `pitch`, `mixer` son módulos con contrato claro.
- Todos los modelos de audio (ADPCM decoder, WSOLA, mixer) son **testables
  con Ceedling** (pure math, sin ESP-IDF).
- El pitch-shift por re-sampling sale gratis (método A) y cubre el 80 % de los
  casos de uso.
- Activación incremental: el MVP arranca sin pitch-shift (`_NONE`), y WSOLA se
  habilita cuando el contenido lo requiera.
- Mezcla de tonos + clips sin sacrificar el feedback del vario.

### Negativas

- **CPU extra**: 5–15 % en escenarios con clip + WSOLA activo. Aceptable con
  las prioridades FreeRTOS del ADR 0001 §6 (I2S=10, EKF=6, GPS=5, BLE=4).
- **RAM**: ~10 KB (4 KB ring buffer por clip + 2 KB ventana WSOLA + 4 KB
  decoder ADPCM). Dentro del presupuesto (ver §9.1).
- **Complejidad del `sound` backend**: crece de ~500 LOC a ~2500 LOC (modelos
  + tests).
- **Gestión del banco** requiere herramienta offline (`scripts/micro/build-audio-bank.py`)
  que empaqueta los clips + tabla + CRC en un blob flasheable.
- **Actualización de clips** requiere flasheo parcial por USB (bootloader) o
  BLE OTA a la partición de flash externa (lento: 16 MB por BLE). Aceptable
  porque las actualizaciones son raras.

## 5. Plan de integración (fases)

1. **Phase X.1 — `bus_drivers/spi`**: infraestructura SPI2 compartida.
2. **Phase X.2 — `storage/flash_ext` backend `w25q128`**: read/write/erase,
   verificación JEDEC ID, power-down. Tests Ceedling con mock SPI.
3. **Phase X.3 — `audio_bank`**: parser del header + tabla, lookup por
   `clip_id`, stream reader. Herramienta offline de empaquetado.
4. **Phase X.4 — `sound/max98357/clip_player`**: decoder PCM16 passthrough +
   IMA-ADPCM. Mezcla ON/OFF con el `tone_model`.
5. **Phase X.5 — `sound/max98357/pitch`**: método A (re-sampling). Kconfig
   por clip (atributo en descriptor) y por llamada.
6. **Phase X.6 — `sound/max98357/mixer`**: mezcla 2 clips + tono + limitador.
   Gestión de prioridades y ducking del beep.
7. **Phase X.7 — gestión de `SHDN`**: auto-off tras silencio.
8. **Phase X.8 (opcional)** — `pitch/wsola`: pitch-shift a tempo fijo. Se
   activa por Kconfig si el contenido lo justifica.
9. **Phase X.9 (opcional)** — `pitch/td_psola`: si surge requisito de calidad
   superior en voz, se implementa usando los `markers` del descriptor.

## 6. Riesgos residuales

| Riesgo | Mitigación |
|--------|-----------|
| Contención CPU con BLE + GPS + WSOLA activos | Escenarios que requieran WSOLA activan un flag que reduce el rate de BLE notify a 4 Hz temporalmente. |
| Artefactos audibles en transiciones tono ↔ clip | Cross-fade de 20 ms en ambos lados (ya previsto en `mixer`). |
| Glitches en la tasa I2S al cambiar pitch (método A) | Método A **no** cambia la tasa I2S de salida: interpola en software sobre la tabla de samples del clip. |
| Corrupción del banco al flashear a medias | Header con CRC32 y versión; si falla la validación al arranque, el banco queda deshabilitado y el vario sigue funcionando solo con tonos sintéticos. |
| Escritura concurrente con lectura | `flash_ext` usa mutex; writes/erases serializan con reads. Irrelevante en runtime normal (solo escribimos durante OTA de banco). |

## 7. Criterios de aceptación

- Arranque del sistema con banco presente: log muestra JEDEC ID, versión y
  número de clips detectados.
- Reproducción de un clip PCM16 de 22 kHz mono con BLE conectado y vario
  emitiendo beeps: sin cortes audibles, sin glitches en el beep, sin drops
  de BLE LK8EX1 a 8 Hz.
- Reproducción del mismo clip a pitch +6 semitonos (método A): audible, sin
  aliasing agresivo, duración reducida en proporción (~0.71×).
- Reproducción de un clip IMA-ADPCM: calidad subjetiva aceptable (test de
  escucha con tres usuarios).
- `SHDN` pasa a `LOW` tras 500 ms de silencio, medido con osciloscopio.
- Consumo del amplificador en silencio prolongado cae a ≤ 5 µA (datasheet).
- Tests Ceedling ≥ 80 % cobertura en los modelos (ADPCM decoder, WSOLA, mixer).
- Arranque con banco corrupto: vario sigue operativo con solo tonos sintéticos,
  log `ESP_LOG_WARN`.

---

**Referencias cruzadas**:
- [ADR 0001 — Audio storage, GPS & pin map](./0001-audio-storage-and-pinmap.md)
- `docs/architecture/firmware-architecture.md` §4 (component catalog), §11 (hardware mapping)
- `docs/roadmap.micro.md` — fases X.1 a X.9 pendientes de cronograma concreto
- Verheggen et al., "WSOLA: Waveform-similarity overlap-add for time-scale modification", 1993
- Moulines & Charpentier, "Pitch-synchronous waveform processing techniques", 1990
- Winbond W25Q128JVSIQ datasheet, Maxim MAX98357A datasheet
