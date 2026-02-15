# Dimmer + Switch (Ventilador y Luces)

**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**TA134 - Sistemas Embebidos**

**Autores**
- Ignacio Ezequiel Cavicchioli - Legajo 109428
- Francisco Javier Moya - Legajo 109899

**Estado del documento:** borrador inicial conciso (a refinar)  
**Fecha:** 14/02/2026

---

## Resumen

Se desarrolló un sistema embebido para control de luz y ventilador de línea (220 VAC), con mando local (botones y potenciómetro) y telemetría por Bluetooth.  
El firmware corre en una NUCLEO-F103RB y utiliza detección de cruce por cero para sincronizar disparo de TRIACs y realizar dimming.

El proyecto integra:
- Control local de luz ON/OFF.
- Control de nivel del ventilador por ADC.
- Estado y telemetría por Bluetooth (JSON por UART transparente).
- Configuración de funciones por DIP switches.
- Almacenamiento en flash de estado de luz y calibración ADC.

Se priorizó arquitectura modular, trazabilidad de estados por logs y mecanismos de prueba en banco.

---

## 1. Introducción y objetivo

El objetivo del trabajo fue implementar un módulo de control doméstico de ventilador y luces, combinando:
- Interfaz física simple de pared.
- Comunicación inalámbrica de baja complejidad (Bluetooth HC-06).
- Seguridad eléctrica para manejo de 220 VAC (aislamiento y etapa de potencia separada).

La elección de Bluetooth responde a un compromiso entre simplicidad de integración, costo y disponibilidad de herramientas durante el desarrollo.

---

## 2. Alcance funcional actual

Estado funcional del firmware principal (`Software STM32/main`):
- Lectura de botones con debounce por máquina de estados.
- Lectura periódica de ADC (potenciómetro).
- Auto-calibración min/max de ADC para escalar 0-100% al rango real del potenciómetro.
- Guardado en flash de:
  - último estado de luz.
  - parámetros de calibración ADC.
- Control de TRIAC por timer (TIM2) sincronizado a cruce por cero (EXTI en PC2).
- Modo de falla (`ST_FAULT`) con corte de potencia.
- Logs por USART2 (ST-Link VCP) y telemetría BT por USART1.

Pendientes técnicos identificados:
- Verificación final de buzzer en hardware (medición en pin con osciloscopio).
- Ajuste fino y validación de dimming en todas las condiciones de carga real.

---

## 3. Arquitectura general

### 3.1 Hardware (bloques)

- NUCLEO-F103RB (control principal).
- Módulo Bluetooth HC-06 (UART).
- Entradas locales:
  - Botón ON (PC12).
  - Botón OFF (PC9).
  - Potenciómetro (PA0 ADC).
  - DIP switches (PC0, PC1, PB0, PA4).
- Detector de cruce por cero (PC2 EXTI).
- Etapa de potencia con TRIAC para:
  - canal ventilador (PB3).
  - canal luz (PB4).
- Indicadores:
  - LED (PB13).
  - buzzer PWM (PA8, TIM1_CH1).

TODO: agregar diagrama en bloques final con permalink.

### 3.2 Firmware (módulos)

- `task_adc.c`: adquisición de entradas (botones, dips, ADC), debounce y eventos.
- `task_system.c`: máquina de estados de sistema, política de operación, persistencia flash.
- `task_pwm.c`: generación de disparos de TRIAC, LED/buzzer y telemetría Bluetooth.
- `app.c`: scheduler cooperativo por tick (1 ms), callbacks EXTI/SysTick y utilidades de prueba.

---

## 4. Configuración y operación

### 4.1 DIP switches

- DIP1 (PC0): habilita Bluetooth.
- DIP2 (PC1): habilita buzzer.
- DIP3 (PB0): habilita LED.
- DIP4 (PA4): reservado; en modo test puede forzar `FAULT`.

### 4.2 Modos de prueba de firmware

Configurables en `Software STM32/main/app/inc/app.h`:
- `APP_TEST_MODE`
- `APP_TEST_SIMULATE_ZC`
- `APP_TEST_WAVE_100HZ_PIN`
- `APP_BT_AT_PROBE_ON_INIT`
- `APP_BT_AT_PROBE_STRICT`

Para operación final debe usarse `APP_TEST_MODE = 0`.

---

## 5. Decisiones de diseño relevantes

### 5.1 Sincronización y dimming

Se utiliza evento de cruce por cero para iniciar la temporización de disparo de TRIAC por semiciclo.  
En pruebas se observó que la temporización efectiva requiere contemplar retardo respecto del flanco detectado por el ZCD.

### 5.2 Persistencia en memoria flash

La persistencia quedó versionada para permitir compatibilidad:
- Layout anterior: estado de luz.
- Layout actual: estado de luz + calibración ADC (min/max).

En la versión actual se configuró `APP_FLASH_STORE_STRICT = 1`, por lo que un fallo de guardado crítico puede llevar a estado `FAULT`.

### 5.3 Bluetooth

La configuración del HC-06 se realizó por comandos AT con interfaz auxiliar USB-UART (Arduino), enviando comandos sin CR/LF y respetando retardos entre comandos.  
Configuración utilizada: nombre `Dimmer_BL` y PIN `1111`.

---

## 6. Fabricación y ensayo de hardware (resumen)

Observaciones de fabricación y banco de pruebas (a refinar con fotos):
- Se usó método de transferencia con PnP Blue y ataque químico.
- Se verificó ausencia de cortos con tester antes de energizar.
- Secuencia de validación: primero ZCD, luego etapa TRIAC, luego pruebas con 24 VAC.
- Se realizaron ajustes de resistencias y puentes en prototipo para adaptar condiciones de prueba.

Lecciones para próxima iteración:
- Simplificar topología de ZCD y/o desacoplarla por canal.
- Revisar huellas mecánicas (varistores y componentes de mayor tamaño).
- Evaluar versión más económica/compacta (menos canales, micro dedicado de potencia o placa separada).

TODO: insertar fotos del proceso de fabricación y pruebas eléctricas con permalink.

---

## 7. Estado de validación

Estado actual (resumen):
- Control de luz por botones: implementado en firmware, en validación final en banco de potencia.
- Dimming de ventilador: implementado, con ajuste de escalado por calibración ADC.
- Telemetría BT: implementada (envío de JSON).
- Buzzer: firmware implementado, pendiente confirmar señal/salida en hardware.

TODO: agregar tabla de pruebas con columnas `Prueba`, `Resultado`, `Evidencia`.

---

## 8. Uso de herramientas de IA en el proyecto

Se deja registro explícito según requerimiento docente:

- Se utilizó asistencia de IA para:
  - proponer estructura inicial de documentación (README, informe de avance, memoria técnica),
  - soporte en decisiones de firmware STM32 y organización de módulos,
  - soporte en herramientas de modelado de estado.
- Todo contenido generado fue revisado y corregido manualmente por el equipo.
- El detalle de referencias internas utilizadas por el grupo se encuentra en:
  - `listado de cosas hechas con IA.txt`

---

## 9. Próximos pasos

- Cerrar validación de hardware con evidencia instrumental:
  - formas de onda de ZCD,
  - pulsos de gate TRIAC,
  - verificación de buzzer.
- Completar documentación visual:
  - fotos de banco,
  - capturas de osciloscopio,
  - diagrama actualizado.
- Refinar la memoria con:
  - tabla de cumplimiento de requisitos final,
  - sección de resultados cuantitativos.

---

## Referencias internas del repositorio

- `README.md`
- `Informe_de_Avances.md`
- `Software STM32/main/app/inc/app.h`
- `Software STM32/pinout.txt`
- `Memoria técnica/cosas para memoria técnica - hardware/cosas a mencionar de software.txt`
- `Memoria técnica/cosas para memoria técnica - hardware/cosas a mencionar sobre fabricación.txt`
- `Memoria técnica/cosas para memoria técnica - hardware/guia de uso del firmware.txt`
- `Memoria técnica/cosas para memoria técnica - hardware/resumen pines y estados logicos.txt`
- `listado de cosas hechas con IA.txt`

