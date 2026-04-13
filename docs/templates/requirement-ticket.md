# [ID-TICKET] Título del Requerimiento

---

## 1. Resumen de la Implementación

Breve descripción técnica de lo que se hizo.

- **Módulos afectados:** *(ej. HAL, Drivers, Application Layer)*
- **Periféricos involucrados:** *(ej. UART, ADC, SPI, PWM)*
- **Consumo de recursos:** *(¿Hubo cambios significativos en el uso de Flash o RAM?)*

---

## 2. Cambios Realizados

### 2.1. Nivel de Firmware

- **Cambio A:** Descripción detallada.
- **Cambio B:** Descripción detallada.
- **Nuevos archivos:** `src/driver_example.c`, `inc/driver_example.h`

### 2.2. Configuración de Hardware

- Cambios en el árbol de dispositivos (Device Tree / Pin Mux).
- Ajustes en registros de configuración.

---

## 3. Plan de Verificación (Testing)

### 3.1. Pruebas Unitarias / Estáticas

- **Análisis de código estático (MISRA/Lint) pasado:** Sí / No
- **Unit Tests ejecutados** *(ej. Unity, GoogleTest):* [Resultado]

### 3.2. Pruebas Funcionales (Hardware in the Loop — HIL)

| Caso de Prueba | Descripción                                | Resultado (Pasa/Falla) |
|----------------|--------------------------------------------|------------------------|
| TC-01          | Verificación de salida PWM con osciloscopio | OK                     |
| TC-02          | Lectura de sensor bajo condiciones X       | OK                     |
| TC-03          | Comportamiento ante interrupción de energía | OK                     |

### 3.3. Evidencias

- **Logs de consola:** *[Adjuntar o vincular log]*
- **Capturas:** *[Captura de pantalla de analizador lógico / osciloscopio]*

---

## 4. Notas de Integración

- **Dependencias:** *(ej. "Requiere actualización de la placa v1.2")*
- **Problemas conocidos:** *(Cualquier bug menor o limitación encontrada)*

---
---

# [TICKET-ID] Requirement Title

---

## 1. Implementation Summary

Brief technical description of what was done.

- **Affected modules:** *(e.g. HAL, Drivers, Application Layer)*
- **Peripherals involved:** *(e.g. UART, ADC, SPI, PWM)*
- **Resource usage:** *(Were there significant changes in Flash or RAM usage?)*

---

## 2. Changes Made

### 2.1. Firmware Level

- **Change A:** Detailed description.
- **Change B:** Detailed description.
- **New files:** `src/driver_example.c`, `inc/driver_example.h`

### 2.2. Hardware Configuration

- Device Tree / Pin Mux changes.
- Configuration register adjustments.

---

## 3. Verification Plan (Testing)

### 3.1. Unit / Static Tests

- **Static code analysis (MISRA/Lint) passed:** Yes / No
- **Unit tests executed** *(e.g. Unity, GoogleTest):* [Result]

### 3.2. Functional Tests (Hardware in the Loop — HIL)

| Test Case | Description                               | Result (Pass/Fail) |
|-----------|-------------------------------------------|---------------------|
| TC-01     | PWM output verification with oscilloscope | OK                  |
| TC-02     | Sensor reading under conditions X         | OK                  |
| TC-03     | Behavior during power interruption        | OK                  |

### 3.3. Evidence

- **Console logs:** *[Attach or link log]*
- **Screenshots:** *[Logic analyzer / oscilloscope screenshot]*

---

## 4. Integration Notes

- **Dependencies:** *(e.g. "Requires board update v1.2")*
- **Known issues:** *(Any minor bugs or limitations found)*
