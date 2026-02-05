# Documentación Técnica: Statechart de Control (Luz/Ventilador)

## Resumen

Este documento detalla la implementación de la lógica de control para el módulo de **Dimmer + Switch (TA134)**. El sistema está modelado en Itemis CREATE (YAKINDU) utilizando un enfoque estrictamente orientado a eventos (`@EventDriven`) para minimizar el consumo de CPU y latencia.

La arquitectura desacopla la lógica de negocio (máquina de estados) de los drivers de hardware (C/C++) mediante una interfaz abstracta de eventos y operaciones.

---

## 1. Configuración del Modelo de Ejecución

Para reproducir el comportamiento en el target, el statechart utiliza las siguientes directivas:

- **`@EventDriven`**: La máquina reacciona asincrónicamente a interrupciones y eventos de hardware.
- **`@SuperSteps(no)`**: Se deshabilita la semántica de "run-to-completion" infinita para evitar bucles de ejecución bloqueantes.
- **Gestión de Tiempo**: Se utilizan timers internos (`after 1s`) para timeouts de seguridad (Watchdog lógico).

---

## 2. Arquitectura de Estados (Top-Level)

El sistema se orquesta en un único estado raíz "**Blink**" que contiene tres sub-máquinas exclusivas.

### 2.1. Secuencia de Boot (Init_ST)

Estado compuesto secuencial. Actúa como un Self-Test de encendido. Bloquea la operación normal hasta que todos los periféricos reporten estado OK. Los periféricos incluyen los estados iniciales de la memoria flash, el bluetooth, el PWM, etc.  

#### Flujo de Validación:

El sistema espera eventos de confirmación (true) de las siguientes banderas para transicionar:

| Paso | Estado | Señal Esperada (Input) | Acción en Falla |
|------|--------|------------------------|-----------------|
| 1 | ST_READ_FLASH | `init_flash_ok` | Salto a Fault_ST |
| 2 | ST_READ_DIP | `init_dip_ok` | Salto a Fault_ST |
| 3 | ST_CHECK_SENSORS | `init_sensors_ok` | Salto a Fault_ST |
| 4 | ST_RESTORE_PWM | `init_pwm_ok` | Salto a Fault_ST |
| 5 | ST_CONFG_BT | `init_bt_ok` | Salto a Normal_ST |

**Nota para Simulación:** Debes forzar estas variables a `true` secuencialmente para salir del boot.

---

### 2.2. Modo Seguro (Fault_ST)

Estado de error recuperable.

**Safety First:** Al entrar (entry), se ejecuta `cut_off_voltage = true` para desenergizar triacs y relés inmediatamente.

**Feedback:** Loop infinito de 1Hz (500ms ON / 500ms OFF) activando LED y Buzzer.

**Watchdog Lógico:** Tras 10 segundos (`after 10s`) sin intervención, el sistema intenta un Soft Reset volviendo a Init_ST.

---

### 2.3. Runtime (Normal_ST)

Aquí reside la lógica principal. Se implementa mediante **Regiones Ortogonales (Paralelismo)** para separar el debounce del control lógico.

#### Región A: Lógica de Control (Main Loop)

Estado simple `ST_IDLE_MANUAL`. Implementa el patrón **Local-First con Notificación**:

- El sistema siempre prioriza la entrada física.
- El Bluetooth es pasivo: solo recibe notificaciones de cambio de estado.

**Transiciones (Self-Transitions):**

1. **Toggle Luz:** Al recibir `EV_SYS_PRESSED` (evento limpio), invierte el estado y dispara `EV_SEND_BT_UPDATE`.
2. **Ajuste PWM:** Al recibir `EV_POTE_CHANGED` (evento de cambio en ADC), actualiza el valor y dispara `EV_SEND_BT_UPDATE`.

#### Región B: Driver de Botón (Software Debounce)

Máquina de estados dedicada para filtrar ruido mecánico.

**Algoritmo:** Contador de histéresis (`tick`).

- **Input:** Eventos crudos `EV_BTN_PRESSED` (interrupción pin change).
- **Output:** Evento limpio `EV_SYS_PRESSED` (solo se dispara cuando `tick` llega a 0).

**Ventaja:** Permite ajustar la sensibilidad del botón sin tocar la lógica de control de la luz.

---

## 3. Definición de Interfaz (API)

Esta es la referencia para el mapeo con el código C (`main.c` / `hardware.c`).

### Inputs (Hardware → Statechart)

| Evento/Variable | Tipo | Descripción |
|-----------------|------|-------------|
| `ST_INIT_*_OK` | event | Flags de resultado positivo de inicialización de periféricos. |
| `ST_INIT_*_NOT_OK` | event | Flags de resultado negativo de inicialización de periféricos. |
| `EV_BTN_PRESSED` | event | Interrupción de flanco (ruidosa). |
| `EV_POTE_CHANGED` | event | Notificación del ADC (cambio > umbral de histéresis). |
| `EV_BT_CONNECTED` | event | Status del módulo HC-05 (actualmente solo informativo). |

### Outputs (Statechart → Hardware)

| Evento/Operación | Tipo | Acción Requerida en C |
|------------------|------|----------------------|
| `EV_SYS_PRESSED` | event | Evento interno (generalmente no requiere acción externa salvo debug). |
| `EV_SEND_BT_UPDATE` | out event | Trigger para enviar trama por UART. |
| `cut_off_voltage` | var | Si es `true`, poner pines de potencia en LOW. |
| `toggle_light` | var | Mapear al GPIO del Relé. |
| `pwm_val` | var | Mapear al registro OCR del Timer (PWM). |

---

## 4. Workflow de Simulación (How-To)

Para validar la lógica sin hardware, sigue estos pasos en Itemis:

### Paso 1: Iniciar la Simulación

1. **Run:** Click derecho en el canvas → **Run As** → **Statechart Simulation**.
2. Se verá el estado atrapado en `Init_ST`.

### Paso 2: Secuencia de Boot

1. En la vista **Simulation** (derecha), buscar los eventos:
   - `ST_INIT_*_OK`
   - `ST_INIT_*_NOT_OK`

2. Presionar `ST_INIT_*_OK` una por una y se transicionará por los estados hasta llegar a `Normal_ST`.

### Paso 3: Prueba de Botón (Debounce)

1. En Simulation, hacer clic repetidamente en `EV_BTN_PRESSED`.
2. Observar la región `ST_BTN`: se verá el estado pasar a `FALLING` y el contador `tick` decrementarse.
3. Se necestia de varios clics (simulando ruido/tiempo) para que se dispare finalmente `EV_SYS_PRESSED` y ver cambiar la variable `toggle_light` en la región superior.

### Paso 4: Prueba de Falla

1. Reiniciar la simulación.
2. En medio del init, presionar un `ST_INIT_*_NOT_OK`.
3. Verificar que entre a `Fault_ST` y que las variables `led_on` / `buzzer_on` oscilen.

---

## 5. Diagrama de Flujo de Estados

```
┌─────────────┐
│   ENTRADA   │
└──────┬──────┘
       │
       ▼
┌──────────────────────────────────────┐
│         Init_ST (Secuencial)         │
│  ┌────────────────────────────────┐  │
│  │ Flash → DIP → Sensores →       │  │
│  │ PWM → BT (validaciones)        │  │
│  └────────────────────────────────┘  │
└──────┬───────────────┬────────────────┘
       │ Todo OK       │ Cualquier Fallo
       │               │
       ▼               ▼
┌─────────────┐  ┌─────────────────────┐
│ Normal_ST   │  │  Fault_ST           │
│ ┌─────────┐ │  │  ┌───────────────┐  │
│ │ Control ├─┼──┼──┤ Bucle Alerta  │  │
│ │ & Botón │ │  │  │ (1Hz toggle)  │  │
│ └─────────┘ │  │  └───────────────┘  │
└─────────────┘  │                      │
                 │ (10s timeout)       │
                 └──────────┬───────────┘
                            │
                            └─→ Init_ST (reintentar)
```

---
## Referencias

- **YAKINDU Statechart Tools:** https://www.itemis.com/en/yakindu/statechart-tools/
- **Itemis CREATE:** Entorno integrado para modelado y generación de código.
- **SCT Code Generator:** Convierte modelos a C/C++/Java.

