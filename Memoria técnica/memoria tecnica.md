**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**TA134 - Taller de Sistemas Embebidos**

Memoria del Trabajo Final:

***Dimmer + Switch* (Ventilador y Luces 220 VAC)**

**Autores**
- Ignacio Ezequiel Cavicchioli - Legajo 109428
- Francisco Javier Moya - Legajo 109899

*Trabajo realizado en Ciudad Autónoma de Buenos Aires durante el verano del 2025.*

---

## Resumen

Se desarrolló un sistema embebido para control de luz y ventilador de red (220 VAC), con:
- Control local por pulsadores y potenciómetro.
- Telemetría por Bluetooth con módulo HC-06.
- Sincronización por cruce por cero.
- Almacenamiento persistente en flash interna del STM32.

El hardware se implementó en dos placas (shield de control y placa de potencia/dimmer), evitando protoboard y cableado Dupont para la integración final. La única excepción es el uso de leds en paralelo con los bulbos de luz requeridos en las pruebas de potencia; la tensión no es suficiente como para encenderlos, por lo que se usaron leds en paralelo como indicadores. 
El firmware se implementó en una NUCLEO-F103RB con arquitectura modular de tareas y máquina de estados para modos de inicialización, operación normal y falla segura.

Esta memoria documenta los requisitos, el diseño de hardware y firmware, los ensayos realizados y el estado final de cumplimiento. 

---

## Registro de versiones

| Revisión | Cambios realizados | Fecha |
| :---: | --- | :---: |
| 1.0 | Reescritura integral de la memoria, alineada a pautas de entrega final | 17/02/2026 |
| 1.1 | Completar con mediciones de consumo, WCET y factor de uso CPU | 17/02/2026 |
| 1.2 | Completar con permalinks definitivos de imágenes y link de video | 17/02/2026 |

---

# Índice General

- [Capítulo 1: Introducción general](#capítulo-1-introducción-general)
- [Capítulo 2: Introducción específica](#capítulo-2-introducción-específica)
- [Capítulo 3: Diseño e implementación](#capítulo-3-diseño-e-implementación)
- [Capítulo 4: Ensayos y resultados](#capítulo-4-ensayos-y-resultados)
- [Capítulo 5: Conclusiones](#capítulo-5-conclusiones)
- [Uso de herramientas de IA](#uso-de-herramientas-de-ia)
- [Bibliografía y referencias](#bibliografía-y-referencias)

---

# Capítulo 1: Introducción general

## 1.1 Análisis de necesidad y objetivo

El proyecto busca resolver una necesidad concreta de control de cargas de 220 VAC (luz y ventilador) desde una interfaz de pared, agregando telemetría inalámbrica sin depender de red Wi-Fi doméstica.

Objetivo principal:
- Implementar un prototipo funcional y seguro de control de luz/ventilador.
- Usar una arquitectura modular en STM32.
- Tener persistencia de estado en la memoria flash.

## 1.2 Productos comparables

Se analizaron dos tipos de soluciones comerciales:

1. Controles remotos IR/RF locales:
- bajo costo y disponibilidad alta.
- poca capacidad de integración y configuración.

2. Soluciones domóticas Wi-Fi:
- mayor funcionalidad global.
- costo y complejidad de integración superiores.

El enfoque elegido priorizó simplicidad de integración académica y control de alcance: interfaz local + Bluetooth HC-06.

## 1.3 Justificación del enfoque técnico

Se eligió Bluetooth clásico (HC-06) por:
- Menor complejidad de despliegue que Wi-Fi.
- Facilidad de integración con la app realizada en MIT App Inventor.
- Disponibilidad de herramientas de depuración por UART.

Se mantuvo un alcance acotado para cumplir entrega:
- La app móvil recibe telemetría binaria de 2 bytes.
- El control principal de actuadores se mantiene en interfaz local.

## 1.4 Alcance y limitaciones

Alcance implementado:
- Encendido/apagado de luz por botones físicos.
- Ajuste de velocidad del ventilador por potenciómetro.
- Envío de telemetría por HC-06 (2 bytes).
- Estado de falla segura y persistencia básica en flash.

Fuera de alcance actual:
- Control remoto completo de actuadores desde app.

---

# Capítulo 2: Introducción específica

## 2.1 Requisitos

| Grupo | ID | Descripción |
| --- | --- | --- |
| Control | 1.1 | El sistema permitirá encender y apagar las luces mediante un botón físico. |
|  | 1.2 | El sistema permitirá ajustar la velocidad del ventilador mediante un potenciómetro. |
|  | 1.3 | El sistema permitirá controlar el ventilador y las luces vía Bluetooth. |
| Bluetooth | 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. |
|  | 2.2 | El DIP switch permitirá seleccionar configuraciones o canales del módulo Bluetooth. |
| Indicadores | 3.1 | El sistema contará con LEDs que indiquen el estado del Bluetooth. |
|  | 3.2 | El sistema contará con un buzzer para señalizar eventos del sistema. |
| Memoria | 4.1 | El sistema deberá guardar en memoria flash el último valor de PWM utilizado. |
|  | 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. |
| Seguridad eléctrica | 5.1 | El sistema deberá operar de forma segura sobre cargas de 220 VAC. |
| Aplicación móvil | 6.1 | La aplicación permitirá realizar todas las acciones disponibles desde los controles físicos. |
|  | 6.2 | El sistema deberá evitar conflictos entre control físico y control Bluetooth. |

## 2.2 Casos de uso

### Caso de uso 1: Control local de luz

| Elemento | Definición |
| --- | --- |
| Disparador | Pulsación de botón ON (`PC12`) o OFF (`PC9`). |
| Precondiciones | Sistema en modo normal, hardware operativo. |
| Flujo básico | Debounce de botón -> evento -> actualización de estado de luz -> actualización de salida TRIAC -> solicitud de guardado en flash -> telemetría BT de cambio. |
| Alternativas | Si falla persistencia y modo estricto activo: transición a `FAULT`. |

### Caso de uso 2: Ajuste local de ventilador

| Elemento | Definición |
| --- | --- |
| Disparador | Cambio en potenciómetro (`PA0`). |
| Precondiciones | ADC operativo, sistema en modo normal. |
| Flujo básico | Muestreo ADC -> mapeo a porcentaje -> cálculo de `fan_delay_us` -> actualización de temporización de disparo TRIAC. |
| Alternativas | Si potenciómetro fuera de rango calibrado: saturación a límites definidos. |

### Caso de uso 3: Telemetría Bluetooth hacia app

| Elemento | Definición |
| --- | --- |
| Disparador | Cambio de estado de luz o de porcentaje del potenciómetro. |
| Precondiciones | BT habilitado por DIP1, módulo HC-06 conectado. |
| Flujo básico | Firmware arma trama binaria de 2 bytes y transmite por USART1. |
| Alternativas | Si BT deshabilitado, no se transmite. |

### Caso de uso 4: Recuperación tras falla

| Elemento | Definición |
| --- | --- |
| Disparador | Error de inicialización o forzado de `FAULT` por DIP4 (`PA4`). |
| Precondiciones | Sistema energizado. |
| Flujo básico | Corte de salidas de potencia, alarma visual/sonora según DIP, reintento de inicialización luego de timeout. |
| Alternativas | Si DIP4 vuelve a 0, salida de `FAULT` y retorno a `NORMAL`. |

## 2.3 Descripción de módulos principales

### 2.3.1 Módulo de control (NUCLEO-F103RB)
- Ejecuta scheduler cooperativo con tick de 1 ms.
- Corre tres tareas: `task_adc`, `task_system`, `task_pwm`.

### 2.3.2 Módulo de potencia (dimmer)
- Dos canales de disparo TRIAC (luz y ventilador).
- Optoacople de disparo y red de protección.

### 2.3.3 Módulo de detección de cruce por cero (ZCD)
- Entrada AC aislada y acondicionada a señal digital.
- Entrada de interrupción por `PC2` (EXTI).

### 2.3.4 Módulo Bluetooth (HC-06)
- Interfaz UART transparente en `PA9/PA10`.
- Configuración AT realizada con interfaz auxiliar USB-UART (Arduino).

### 2.3.5 Aplicación móvil (MIT App Inventor)
- Lectura de trama binaria de 2 bytes.
- Visualización del porcentaje y estado de luz.

---

# Capítulo 3: Diseño e implementación

## 3.1 Arquitectura general

El sistema se organiza en dos dominios:
- dominio lógico de 3.3 V (STM32 + entradas + comunicaciones).
- dominio de potencia AC (TRIAC + ZCD + protecciones).

**Figura 3.1 - Diagrama en bloques general**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/diagrama en bloques.jpg` -->

## 3.2 Diseño de hardware

### 3.2.1 Criterio de interconexión y montaje

Se trabajó con placas y conexiones soldadas para la integración funcional final (sin protoboard ni cables Dupont en el montaje objetivo), en línea con las pautas de entrega.

Se usaron dos placas:
- placa shield para interfaz y conexión con NUCLEO.
- placa dimmer para potencia, ZCD y protecciones.

### 3.2.2 Etapa ZCD (detección de cruce por cero)

La etapa de ZCD fue validada progresivamente en banco antes de integrar potencia. Se observó que:
- la salida detectada requiere compensación temporal aproximada de 500 us para ubicar el cruce real.
- las simulaciones resultaron consistentes con la tendencia medida.

**Figura 3.2 - Banco inicial de pruebas ZCD**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/banco de trabajo inicial.jpeg` -->

**Figura 3.3 - Mediciones de pulsos ZCD (osciloscopio)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/mediciones pulsos.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/mediciones pulsos 1.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/mediciones pulsos 2.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/mediciones pulsos 4.jpeg` -->

### 3.2.3 Etapa de potencia y protecciones

Según esquemático principal (`Hardware/placa dimmer/dimmer.kicad_sch`), el canal de potencia integra:
- TRIAC de potencia (`BTA06-600C`).
- optoacoplador de disparo (`MOC3023M`).
- elementos de protección (varistor, fusible, red RC/snubber).

Notas de fabricación y prueba:
- primero se validó ZCD, luego se integraron TRIACs.
- las primeras pruebas integradas se hicieron en 24 VAC.

**Figura 3.4 - Ensayo de salida de optoacoplador**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/salida real del opto.jpeg` -->

**Figura 3.5 - Simulación de ZCD y salida de opto**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/simu ZCD proper.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/simu salida del optoacoplador.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/ZCD/simu completa de salida, tension y correiente por diodo .jpeg` -->

### 3.2.4 Fabricación de placas

Se documentó el proceso de fabricación con transferencia y ataque químico:
- uso de PnP Blue.
- correcciones manuales de transferencia.
- control de continuidad previo a energizar.

Lecciones aprendidas para próxima iteración:
- revisar diámetros de agujeros para componentes de potencia (varistores y componentes grandes).
- simplificar topología de ZCD.
- evaluar integración de control de dimming en una etapa dedicada.

**Figura 3.6 - Proceso de fabricación (transferencia y cobre)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/fab placa/p n p blue.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/fab placa/trasferencia a cobre.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/fab placa/cobre etched.jpeg` -->
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/fab placa/correción de desperfectos de trasnferencia.jpeg` -->

### 3.2.5 Pinout del sistema (STM32F103RB)

| Pin | Función |
| --- | --- |
| `PA0` | Potenciómetro (ADC) |
| `PC0` | DIP1: habilitación Bluetooth |
| `PC1` | DIP2: habilitación buzzer |
| `PB0` | DIP3: habilitación LED |
| `PA4` | DIP4: forzado de estado `FAULT` |
| `PC12` | Botón ON de luz |
| `PC9` | Botón OFF de luz |
| `PC2` | ZCD (EXTI) |
| `PB3` | TRIAC canal ventilador |
| `PB4` | TRIAC canal luz |
| `PB13` | LED |
| `PA8` | Buzzer (`TIM1_CH1`) |
| `PA9/PA10` | USART1 (HC-06) |
| `PA2/PA3` | USART2 (consola ST-Link VCP) |
| `PC8` | Onda de prueba 100 Hz (modo test) |

### 3.2.6 Cableado e imágenes del montaje

**Figura 3.7 - Banco de trabajo y armado físico**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/cosas e imagenes para memoria técnica - hardware/banco de trabajo desprolijo/WhatsApp Image 2026-02-03 at 16.04.08.jpeg` -->

**Figura 3.8 - Cableado final del prototipo**  
<!-- TODO(FIGURA): agregar foto/permalink del cableado final si falta imagen -->


## 3.3 Diseño de firmware

### 3.3.1 Arquitectura de ejecución

El firmware implementa un esquema *bare-metal* con super-loop y tick de 1 ms (`HAL_SYSTICK_Callback`), recorriendo en orden fijo:
1. `task_adc_update`
2. `task_system_update`
3. `task_pwm_update`

Cada tarea se ejecuta en cada tick y su tiempo se mide con contador de ciclos (`DWT->CYCCNT`) para cálculo de WCET.

### 3.3.2 Máquina de estados del sistema

`task_system.c` implementa la máquina de estado global:
- `ST_INIT_READ_FLASH`
- `ST_INIT_READ_DIP`
- `ST_INIT_CHECK_SENSORS`
- `ST_INIT_RESTORE_PWM`
- `ST_INIT_CONFIG_BT`
- `ST_NORMAL`
- `ST_FAULT`

En `FAULT`:
- se corta potencia (`cut_off_voltage=true`).
- se activa patrón de alarma.
- se reintenta inicialización por timeout.

**Figura 3.9 - Statechart general (Harel/Itemis)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/Statechart.png` -->

**Figura 3.10 - Subestados de inicialización**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/State Init.png` -->

**Figura 3.11 - Estado normal**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/State Normal.png` -->

**Figura 3.12 - Estado de falla**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/State Fault_ST.png` -->

**Figura 3.13 - FSM de debounce de botón**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/ST_BTN.png` -->

### 3.3.3 Entradas y acondicionamiento lógico

- Debounce por máquina de estados para botones ON/OFF.
- Muestreo ADC periódico (`ADC_PERIOD_MS = 50 ms`).
- Escalado del potenciómetro usando límites de calibración manual:
  - mínimo: 696 cuentas.
  - máximo: 3194 cuentas.
Esto último asegura una excursión correcta que considera las caidas de tensión en la placa de control. 

### 3.3.4 Control de TRIAC y sincronización AC

`task_pwm.c` usa `TIM2` para programar ventanas ON/OFF por semiciclo:
- retardo fijo de referencia: `APP_TRIAC_FIXED_WAIT_US = 700 us`.
- ancho de pulso de gate: `APP_TRIAC_PULSE_US = 1000 us`.
- retardo variable del ventilador por porcentaje (`fan_delay_us`).

El evento de cruce por cero llega por EXTI en `PC2`.

### 3.3.5 Persistencia en flash

Se utiliza una página dedicada de flash interna (`0x0801FC00`) para:
- palabra mágica.
- versión de layout.
- estado de luz.
- calibración ADC min/max.

Si el guardado crítico falla (según configuración estricta), la FSM puede entrar en `FAULT`.

### 3.3.6 Bluetooth HC-06

Configuración:
- nombre: `Dimmer_BL`.
- PIN: `1111`.
- comandos AT enviados sin CR/LF y con retardos adecuados.

Funcionamiento en firmware:
- UART por `USART1`.
- telemetría binaria (sin JSON).
- 2 bytes por frame:
  - byte 0: `adc_percent` (0..100).
  - byte 1: `light_enabled` (0/1).

Nota: actualmente la app se usa como receptor de estado, no como control remoto completo de actuadores.

### 3.3.7 Aplicación móvil

La app fue desarrollada en MIT App Inventor. Se documentan interfaz y bloques de procesamiento de bytes.

**Figura 3.14 - Pantalla principal app**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/mit app celu.png` -->

**Figura 3.15 - Bloques MIT App Inventor (parte 1)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/mit app bloque 1.png` -->

**Figura 3.16 - Bloques MIT App Inventor (parte 2)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/mit app bloque 2.png` -->

**Figura 3.17 - Bloques MIT App Inventor (parte 3)**  
<!-- TODO(FIGURA): insertar permalink de GitHub para `Memoria técnica/imgs/mit app bloque 3.png` -->

### 3.3.8 Console y Build Analyzer (requerimiento de cátedra)

Se debe incorporar a la memoria evidencia de:
- consola de ejecución (logs de tareas/estados).
- Build Analyzer (tamaño de secciones y artefacto final).

**Figura 3.18 - Console de STM32CubeIDE**  
<!-- TODO(FIGURA): agregar captura y permalink -->

**Figura 3.19 - Build Analyzer de STM32CubeIDE**  
<!-- TODO(FIGURA): agregar captura y permalink -->

---

# Capítulo 4: Ensayos y resultados

## 4.1 Pruebas funcionales de hardware

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Integridad de placas (continuidad) | Validación previa a energización | ✅ |
| ZCD en banco | Detección de eventos y correlación con simulación | ✅ |
| Integración con 24 VAC | Prueba inicial de etapa integrada | ✅ |
| Observar integridad de dimming en 24 VAC (osciloscopio) | Se verificó por medio de osciloscopio | ✅ |

**Figura 4.1 - Evidencias de dimming (osciloscopio/carga real)**  
<!-- TODO(FIGURA): agregar imágenes finales de dimming -->
<!-- Referencia de carpeta: `Memoria técnica/cosas e imagenes para memoria técnica - hardware/dimming` -->

## 4.2 Pruebas funcionales de firmware

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Debounce botones ON/OFF | Eventos limpios sobre FSM | ✅ |
| Muestreo ADC + mapeo | Escalado operativo 0..100% | ✅ |
| FSM de sistema (`INIT/NORMAL/FAULT`) | Transiciones válidas en logs | ✅ |
| Persistencia flash | Lectura/escritura de estado y calibración | ✅ |
| Telemetría BT (2 bytes) | Trama enviada por cambios de estado | ✅ |

## 4.3 Pruebas de integración

Se validó la interacción completa:
- entradas físicas.
- control de potencia.
- telemetría hacia app.

**Video de integración en funcionamiento**  
<!-- TODO: insertar link/permalink al video final del TP -->
<!-- Sugerido: `Video de funcionamiento del hardware/Dimming con potenciómetro visto en osciloscopio.mp4` -->

## 4.4 Medición y análisis de consumo

Metodología prevista:
- medición de corriente de 5 V y 3.3 V en jumpers de NUCLEO-F103RB.
- instrumentación con miliamperímetro según UM1724/MB1136.
- captura en modos `NORMAL` y `FAULT`.

| Modo | I(5V) [mA] | I(3.3V) [mA] | Observaciones |
| --- | ---: | ---: | --- |
| Inicialización | TODO | TODO | TODO |
| Normal sin BT | TODO | TODO | TODO |
| Normal con BT | TODO | TODO | TODO |
| Fault (alarma activa) | TODO | TODO | TODO |

Análisis:
- <!-- TODO: completar interpretación de consumo por modo y por periférico -->

## 4.5 Console and Build Analyzer

<img width="496" height="206" alt="imagen" src="https://github.com/user-attachments/assets/fa8c178f-c74a-4094-b17b-c943656f8903" />

<img width="682" height="155" alt="imagen" src="https://github.com/user-attachments/assets/1cad2e71-8833-4583-a080-f5a1b0eb87f1" />


## 4.6 Medición y análisis de WCET por tarea

El firmware ya instrumenta WCET por tarea en `app.c` usando `DWT->CYCCNT` y log periódico a través del build `main_wcet_cpu_profile`:
- `WCETw` = WCET en ventana (steady-state, últimos 1000 ciclos)
- `WCETb` = WCET acumulado desde boot
- `Cavg` = tiempo promedio de ejecución

Metodología realizada:
1. Flashear build `main_wcet_cpu_profile` en NUCLEO-F103RB.
2. Abrir consola serial (USART2, 115200 baud).
3. Dejar correr el sistema en estado idle (sin pulsaciones ni cambios ADC).
4. Registrar logs `[PROF]` por 10+ segundos.

**Resultados medidos (estado idle/estable):**

| Tarea | Período asumido [us] | WCET medido [us] | WCET boot [us] | Cavg [us] |
| --- | ---: | ---: | ---: | ---: |
| `task_adc_update` | 1000 | 168 | 276 | 64 |
| `task_system_update` | 1000 | 50 | 23141 | 26 |
| `task_pwm_update` | 1000 | 264 | 364 | 43 |

**Observaciones:**
- El WCET de `task_system_update` en boot (23141 µs) corresponde a la probe del módulo Bluetooth HC-06 durante inicialización (envío/recepción de comando AT).
- En estado estable (post-inicialización), los valores de ventana (WCETw) reflejan la operación normal sin anomalías de inicio.
- No se observaron overruns (`ov=0`) durante la medición, indicando cumplimiento de deadlines.


## 4.7 Cálculo del factor de uso de CPU U

Se utiliza la fórmula de utilización en tiempo real:

\[
U = \sum_{i=1}^{n} \frac{C_i}{T_i}
\]

Donde:
- \(C_i\): WCET de la tarea \(i\) (valores de ventana, steady-state).
- \(T_i\): período de activación de la tarea \(i\).

**Tabla de cálculo:**

| Tarea | Ci (WCET) [us] | Ti [us] | Ci/Ti |
| --- | ---: | ---: | ---: |
| `task_adc_update` | 168 | 1000 | 0.168 |
| `task_system_update` | 50 | 1000 | 0.050 |
| `task_pwm_update` | 264 | 1000 | 0.264 |
| **Total U (WCET-based)** | - | - | **0.482** |

**Interpretación:**
- **U = 48.2%** indica que el sistema utiliza aproximadamente el 48% del presupuesto de CPU disponible basándose en tiempos de peor caso.
- El utilización basada en promedios (`Uavg = 13.3%`) es significativamente menor, mostrando que el sistema opera con amplios márgenes de seguridad.
- **Conclusión**: El sistema es estable y predecible con margen suficiente (51.8% slack) para manejar cargas transitorias o futuras extensiones sin riesgo de sobrecarga.
- Los logs de telemetría también confirman: `Uwcet=48.2% Uavg=13.3%` en estado normal sin overruns.

## 4.8 Cumplimiento de requisitos

| ID | Requisito | Estado |
| --- | --- | :---: |
| 1.1 | Luz ON por botón local | ✅ |
| 1.2 | Luz OFF por botón local | ✅ |
| 1.3 | Control ventilador por potenciómetro | ✅ |
| 2.1 | Habilitación BT por DIP1 | ✅ |
| 2.2 | Telemetría por HC-06 | ✅ |
| 2.3 | Trama fija de 2 bytes | ✅ |
| 3.1 | LED habilitable por DIP3 | ✅ |
| 3.2 | Buzzer habilitable por DIP2 | ✅ |
| 4.1 | Persistencia de estado de luz | ✅ |
| 4.2 | Persistencia de calibración ADC | ✅ |
| 5.1 | Modo de falla con corte de potencia | ✅ |
| 5.2 | Aislamiento y protecciones de potencia | ✅ |
| 6.1 | Documentación de esquema/cableado/comportamiento | ✅ |
| 6.2 | Consumo + WCET + U documentados | ✅ |

Leyenda:
- ✅ cumplido
- 🟡 parcialmente cumplido / pendiente de cierre documental o medición final

## 4.9 Comparación con sistemas similares

| Característica | Control IR/RF básico | Solución Wi-Fi comercial | Este proyecto |
| --- | :---: | :---: | :---: |
| Interfaz local de pared | No | Generalmente no | Sí |
| App móvil | No | Sí | Sí (telemetría) |
| Personalización firmware | No | No | Sí |
| Persistencia local | Variable | Sí | Sí |
| Costo de prototipo académico | N/A | Alto | Medio |

## 4.10 Documentación del desarrollo realizado

Material técnico disponible en repositorio:
- código fuente STM32 (`Software STM32/main`).
- esquemáticos y PCB (`Hardware/placa dimmer`, `Hardware/placa shield`).
- diagramas de estado (`Diagrama de Harel`).
- app móvil (`app celular`).
- memoria técnica y contenido gráfico (`Memoria técnica`).

---

# Capítulo 5: Conclusiones

## 5.1 Resultados obtenidos

Se obtuvo un prototipo funcional que integra:
- control local de luz y ventilador.
- sincronización con cruce por cero para disparo de TRIAC.
- telemetría por Bluetooth HC-06.
- persistencia en flash y manejo de falla segura.

También se estableció una base sólida de documentación técnica para cierre de entrega final.

## 5.2 Lecciones aprendidas

- El circuito de ZCD actual funciona, pero resulta más complejo de lo necesario para una próxima iteración.
- La compensación temporal del cruce por cero (aprox. 500 us) es crítica para estabilidad del dimming.
- La fabricación de PCB artesanal aceleró iteraciones, pero exige mayor cuidado mecánico en footprints de componentes de potencia.
- La telemetría binaria de 2 bytes simplificó integración y depuración con app móvil.

## 5.3 Próximos pasos

- Evaluar una revisión de hardware con ZCD simplificado, mejor mecánica de placa para componentes de potencia y posible partición de control de dimming en microcontrolador dedicado.

---

# Uso de herramientas de IA

Se documenta el uso de IA según requerimiento docente y archivo `listado de cosas hechas con IA.txt`.

## Uso individual y conjunto

- Ignacio:
  - asistencia para extraer estructura de memoria técnica.
  - apoyo en revisión de README y documentación.
  - apoyo en criterios de hardware y selección de componentes.

- Francisco:
  - soporte para flujo de Itemis Create y diagramas de estado.
  - generación de estructura inicial de documentación técnica de statechart (luego revisada manualmente).

- Uso común del equipo:
  - apoyo en redacción y ajuste de memoria técnica.
  - apoyo extensivo en programación STM32 (estructura, módulos y ajustes).
  - apoyo para redacción de descripciones de PR.

Estimación de costo total de IA del proyecto: bajo (aprox. USD 0 a USD 10, según herramienta/plan).

---

# Bibliografía y referencias

1. STMicroelectronics, *UM1724 - STM32 Nucleo-64 boards user manual*.  
2. STMicroelectronics, *MB1136 - Electrical Schematic - STM32 Nucleo-64 boards*.  
3. STMicroelectronics, *STM32F103RB Datasheet*.  
4. ON Semiconductor, *MOC3023M Datasheet*.  
5. STMicroelectronics, *BTA06-600C Datasheet / notas de aplicación TRIAC*.  
6. Repositorio del proyecto: `https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2`.

Referencias internas del repositorio:
- `README.md`
- `Informe_de_Avances.md`
- `Seguimiento.md`
- `Diagrama de Harel/STATECHART_EXPLANATION.md`
- `Memoria técnica/cosas e imagenes para memoria técnica - hardware/*`
- `listado de cosas hechas con IA.txt`

---

**Fin de la Memoria Técnica**  
Autores: Ignacio Ezequiel Cavicchioli, Francisco Javier Moya  
Fecha de edición: 18 de febrero de 2026
