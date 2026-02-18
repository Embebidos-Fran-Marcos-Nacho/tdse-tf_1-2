**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**TA134 - Taller de Sistemas Embebidos**

Memoria del Trabajo Final:

***Dimmer + Switch* (Ventilador y Luces 220 VAC)**

**Autores**
- Ignacio Ezequiel Cavicchioli - Legajo 109428
- Francisco Javier Moya - Legajo 109899

*Trabajo realizado durante el verano del 2025.*

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
| 1.2 | Entrega N°1 | 17/02/2026 |

---

# Índice General

- [Capítulo 1: Introducción general](#capítulo-1-introducción-general)
- [Capítulo 2: Introducción específica](#capítulo-2-introducción-específica)
- [2.1 Requisitos (versión final)](#21-requisitos-versión-final)
- [Capítulo 3: Diseño e implementación](#capítulo-3-diseño-e-implementación)
- [Capítulo 4: Ensayos y resultados](#capítulo-4-ensayos-y-resultados)
- [4.9 Cumplimiento de requisitos](#49-cumplimiento-de-requisitos)
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

## 2.1 Requisitos (versión final)

| Grupo | ID | Descripción |
| --- | --- | --- |
| Control | 1.1 | El sistema permitirá encender y apagar las luces mediante un botón físico. |
|  | 1.2 | El sistema permitirá ajustar la velocidad del ventilador mediante un potenciómetro. |
|  | 1.3 | El sistema permitirá ver el estado del ventilador y las luces vía Bluetooth. |
| Bluetooth | 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. |
|  | 2.2 | El DIP switch permitirá seleccionar configuraciones o canales del módulo Bluetooth. |
| Indicadores | 3.1 | El sistema contará con LEDs que indiquen el estado del Bluetooth. |
|  | 3.2 | El sistema contará con un buzzer para señalizar eventos del sistema. |
| Memoria | 4.1 | El sistema deberá guardar en memoria flash el último valor de PWM utilizado. |
|  | 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. |
| Seguridad eléctrica | 5.1 | El sistema deberá operar de forma segura sobre cargas de 220 VAC. |
| Aplicación móvil | 6.1 | La aplicación dará información sobre los estados disponibles, que incluyen la velocidad del ventilador y el estado de luces. |
|  | 6.2 | El sistema deberá evitar conflictos entre el control físico y la comunicación Bluetooth, incluyendo conflictos de timings. |

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
| Flujo básico | Firmware arma trama binaria de 2 bytes y transmite por USART1 para que la app informe el estado del sistema. |
| Alternativas | Si BT deshabilitado, no se transmite. |

### Caso de uso 4: Recuperación tras falla

| Elemento | Definición |
| --- | --- |
| Disparador | Error de inicialización o forzado de `FAULT` por DIP4 (`PA4`). |
| Precondiciones | Sistema energizado. |
| Flujo básico | Corte de salidas de potencia, alarma visual/sonora según DIP, reintento de inicialización luego de timeout. |
| Alternativas | Si DIP4 vuelve a 0, salida de `FAULT` y retorno a `NORMAL`. |

Nota de trazabilidad de alcance:
- El informe de avances redefinió el alcance Bluetooth para visualización de estado (sin control remoto completo de actuadores).
- Los casos de uso y la app se documentan en consecuencia: recepción de telemetría y presentación de estado.

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
- Dominio lógico de 3.3 V (STM32 + entradas + comunicaciones).
- Dominio de potencia AC (TRIAC + ZCD + protecciones).

**Figura 3.1 - Diagrama en bloques general**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/imgs/diagrama%20en%20bloques.jpg)

*Epígrafe: Diagrama de bloques general.*



## 3.2 Diseño de hardware

### 3.2.1 Criterio de interconexión y montaje

Se trabajó con placas y conexiones soldadas para la integración funcional final (sin protoboard ni cables Dupont en el montaje objetivo), en línea con las pautas de entrega.

Se usaron dos placas:
- placa shield para interfaz y conexión con NUCLEO.
- placa dimmer para potencia, ZCD y protecciones.

### Etapa de conversión de niveles

**Figura 3.2 - Esquemático del conversor de niveles**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquema%20niveles.png)
*Epígrafe: Esquemático del conversor de niveles.*

Se requirió para unir la placa F103RB (3.3 V) con la placa diseñada. 

### Etapa de Triacs 

**Figura 3.3 - Esquemático de driver de TRIAC**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquem%20triac.png)
*Epígrafe: Esquemático de driver de TRIAC.*

Diseño tomado de las notas de aplicación que se encuentran en este mismo repositorio. 

### 3.2.2 Etapa ZCD (detección de cruce por cero)

La etapa de ZCD fue validada progresivamente en banco antes de integrar potencia. Se observó que:
- la salida detectada requiere compensación temporal aproximada de 500 us para ubicar el cruce real.
- las simulaciones resultaron consistentes con la tendencia medida.

**Figura 3.4 - Esquemático del ZCD**

![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquematico%20ZCD.png)
*Epígrafe: Esquemático del ZCD.*



**Figura 3.5 - Banco inicial de pruebas ZCD**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/banco%20de%20trabajo%20inicial.jpeg)
*Epígrafe: Banco de trabajo durante las verificaciones del ZCD con osciloscopio.*


**Figura 3.6 - Mediciones de pulsos ZCD (osciloscopio)**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos.jpeg)
*Epígrafe: Pulsos de salida del ZCD - cursor midiendo tiempo entre pulsos.*

Nótese que el ZCD actúa en cada cruce por cero, generando una señal de 100 Hz.

**Figura 3.7 - Medición de ancho de pulso del ZCD**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%201.jpeg)
*Epígrafe: Salida del ZCD con la senoidal aplicada - cursor midiendo ancho de pulso.*

**Figura 3.8 - Disparo previo al cruce real (senoidal negativa)**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%202.jpeg)
*Epígrafe: Salida del ZCD con la senoidal aplicada - cursor midiendo tiempo de disparo previo al cruce por cero real con senoidal negativa.*

**Figura 3.9 - Disparo previo al cruce real (senoidal positiva)**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%204.jpeg)
*Epígrafe: Salida del ZCD con la senoidal aplicada - cursor midiendo tiempo de disparo previo al cruce por cero real con senoidal positiva.*

El retardo fijo de disparo de los triacs se estimó tomando de referencia los tiempos de disparo del ZCD respecto del cruce real mostrados en estas imágenes. 


### 3.2.3 Etapa de potencia y protecciones

Según esquemático principal (`Hardware/placa dimmer/dimmer.kicad_sch`), el canal de potencia integra:
- TRIAC de potencia (`BTA06-600C`).
- Optoacoplador de disparo (`MOC3023M`).
- Elementos de protección (varistor, fusible, red RC/snubber opcional).

Notas de fabricación y prueba:
- Primero se validó el correcto funcionamiento del ZCD, luego se integraron TRIACs.
- Las primeras pruebas integradas se hicieron en 24 VAC. Esto conllevó una ligera y reversible modificación del circuito de ZCD. 

**Figura 3.10 - Ensayo de salida de optoacoplador**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/salida%20real%20del%20opto.jpeg)
*Epígrafe: Señal a la salida del 4N25 en configuración de emisor común/negador.*

**Figura 3.11 - Simulación de ZCD y salida de opto**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/simu%20ZCD%20proper.jpeg)
*Epígrafe: Simulación de la entrada y salida ideal del ZCD.*

Nótese que es muy parecida a la medida. 

**Figura 3.12 - Salida simulada del 4N25**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/simu%20salida%20del%20optoacoplador.jpeg)
*Epígrafe: Salida simulada del 4N25.*

No se parece mucho a la real, pero funcionó igual: la tensión dio para disparar los Schmitt trigger.


### 3.2.4 Fabricación de placas

Se documentó el proceso de fabricación con transferencia y ataque químico:
- Primero se imprimió el diseño sobre un papel PnP Blue.
- Luego se transfirió por medio de calor. 
- Se hicieron las correcciones manuales de transferencia.
- Por último, se realizó un control de continuidad previo a energizar.

Lecciones aprendidas para próxima iteración:
- Revisar diámetros de agujeros para componentes de potencia (varistores y componentes grandes).
- Simplificar topología de ZCD.
- Evaluar integración de control de dimming en una etapa dedicada.

**Figura 3.13 - Papel de transferencia con diseño impreso**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/p%20n%20p%20blue.jpeg)
*Epígrafe: Papel de transferencia con el diseño impreso.*

**Figura 3.14 - Transferencia previa a correcciones**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/trasferencia%20a%20cobre.jpeg)
*Epígrafe: Transferencia previa a correcciones.*

**Figura 3.15 - Transferencia corregida**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/correci%C3%B3n%20de%20desperfectos%20de%20trasnferencia.jpeg)
*Epígrafe: Transferencia corregida.*

**Figura 3.16 - Placa fabricada**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/cobre%20etched.jpeg)
*Epígrafe: Placa fabricada.*


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

**Figura 3.17 - Cableado final del prototipo**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/banco%20de%20trabajo%20desprolijo/banco%20final.jpeg)
*Epígrafe: Montaje final del prototipo durante ensayo integrado.*


**Figura 3.18 - Diagrama de conexión entre placas simplificado**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/00693ac864a65b0389699a47c52606a88d0adbb9/Diagrama%20de%20conexi%C3%B3n%20simplificado/conexionado.png)
*Epígrafe: Diagrama simplificado de conexión entre placas.*

**Figura 3.19 - Overview de placa shield y conexionado**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Diagrama%20de%20conexi%C3%B3n%20simplificado/f103rb.jpg)
*Epígrafe: Vista general y conexionado de la shield para F103RB.*

**Figura 3.20 - Conexionado de placa de TRIACs**
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Diagrama%20de%20conexi%C3%B3n%20simplificado/triacs.jpg)
*Epígrafe: Conexionado de la placa de TRIACs y cargas.*


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

**Figura 3.21 - Statechart general (Harel/Itemis)**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/Statechart.png)

**Figura 3.22 - Subestados de inicialización**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/State%20Init.png)

**Figura 3.23 - Estado normal**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/State%20Normal.png)

**Figura 3.24 - Estado de falla**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/State%20Fault_ST.png)

**Figura 3.25 - FSM de debounce de botón**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/ST_BTN.png)

### 3.3.3 Entradas y acondicionamiento lógico

- Debounce por máquina de estados para botones ON/OFF.
- Muestreo ADC periódico (`ADC_PERIOD_MS = 50 ms`).
- Escalado del potenciómetro usando límites de calibración manual:
  - mínimo: 696 cuentas.
  - máximo: 3194 cuentas.
- Filtro por deadband para evento de potenciómetro (`APP_ADC_PERCENT_EVENT_DEADBAND = 2%`) para evitar oscilaciones por ruido (ej. 99% <-> 100%).
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
- Envío periódico por tiempo (no por cambio), configurable con `APP_BT_TELEMETRY_PERIOD_MS` (actualmente `50 ms`).

Nota: actualmente la app se usa como receptor de estado, no como control remoto completo de actuadores.

### 3.3.7 Aplicación móvil

La app fue desarrollada en MIT App Inventor. Se documentan interfaz y bloques de procesamiento de bytes.

**Figura 3.26 - Pantalla principal app**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/566a7314061481abbec17f240388ee198cea82ee/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/captura%20app.jpeg)
*Epígrafe: Pantalla principal de la App.*


**Figura 3.27 - Bloques MIT App Inventor (parte 1)**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%201.png)
*Epígrafe: Bloques de inicialización de la pantalla principal.*

**Figura 3.28 - Bloques MIT App Inventor (parte 2)**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%202.png)
*Epígrafe: Lógica de actualización de datos y pantalla.*

**Figura 3.29 - Bloques MIT App Inventor (parte 3)**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%203.png)
*Epígrafe: Lógica de selección de dispositivo bluetooth.*

---

# Capítulo 4: Ensayos y resultados

## 4.1 Pruebas funcionales de hardware

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Integridad de placas (continuidad) | Validación previa a energización | ✅ |
| ZCD en banco | Detección de eventos y correlación con simulación | ✅ |
| Integración con 24 VAC | Prueba inicial de etapa integrada | ✅ |
| Observar integridad de dimming en 24 VAC (osciloscopio) | Se verificó por medio de osciloscopio | ✅ |

## 4.2 Pruebas funcionales de firmware

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Debounce botones ON/OFF | Eventos limpios sobre FSM | ✅ |
| Muestreo ADC + mapeo | Escalado operativo 0..100% | ✅ |
| FSM de sistema (`INIT/NORMAL/FAULT`) | Transiciones válidas en logs | ✅ |
| Persistencia flash | Lectura/escritura de estado y calibración | ✅ |
| Telemetría BT (2 bytes) | Trama enviada en forma periódica (`APP_BT_TELEMETRY_PERIOD_MS`) | ✅ |

## 4.3 Pruebas de integración

Se validó la interacción completa:
- entradas físicas.
- control de potencia.
- telemetría hacia app.

**Video de integración en funcionamiento**  

https://youtu.be/iv2bGrqrMtU



## 4.4 Medición y análisis de consumo

Metodología aplicada:
- medición de consumo total en la entrada de `5V` del sistema (NUCLEO + shield).
- alimentación desde fuente externa conectada a pines `5V` y `GND`.
- medición de corriente con multímetro en serie sobre la línea de `5V`.
- medición de tensión en bornes de entrada para estimar potencia (`P = V * I`).

Procedimiento realizado:
1. Desconectar USB/ST-Link para evitar doble alimentación.
2. Conectar fuente externa a `5V` y `GND`.
3. Ajustar la fuente para garantizar `5V` en el pin `5V` de la placa (compensando caídas en cables).
4. Intercalar amperímetro en serie en la línea de `5V`.
5. Medir tensión de entrada en paralelo sobre `5V-GND`.
6. Registrar datos en los modos:
   - inicialización.
   - normal sin módulo Bluetooth conectado.
   - normal con módulo Bluetooth conectado pero desactivado.
   - normal con Bluetooth activo enviando datos.
   - fault con alarma activa (buzzer + LED).
7. Debido a que el consumo oscila rápidamente en el tiempo, se tomó como referencia el valor pico observado en cada modo.

Alcance de la medición:
- Esta medición representa el consumo total a `5V` del conjunto montado.
- El riel de `3.3V` queda incluido indirectamente, ya que se genera desde `5V` mediante el regulador de la placa.

| Modo | I pico @5V [mA] | P pico @5V [W] | Observaciones |
| --- | ---: | ---: | --- |
| Normal sin módulo BT (desconectado) | 64 | 0.320 | Escenario de menor consumo; representa una forma válida de uso sin telemetría Bluetooth. |
| Normal con módulo BT conectado y desactivado | 104 | 0.520 | Aumento de consumo por presencia/alimentación del módulo Bluetooth. |
| Normal con BT activo enviando datos | 107 | 0.535 | Incremento leve respecto al modo BT desactivado. |
| Fault (buzzer + LED activos) | 145 | 0.725 | Peor caso medido en operación. |

Análisis:
- Potencia calculada como `P = V * I`, usando `V = 5V` y corriente pico medida en cada modo.
- El peor caso medido fue `145 mA` a `5V`, equivalente a `0.725 W`.
- El sistema se mantiene por debajo de `1 W`, por lo que puede alimentarse sin inconvenientes con fuentes comerciales 220VAC->5V de baja potencia.
- La diferencia entre BT desactivado y BT transmitiendo (`104 mA` -> `107 mA`) es baja, consistente con carga adicional moderada por comunicación.

## 4.5 Console and Build Analyzer

Resultado consolidado de herramientas de análisis de consola y build.

**Figura 4.1 - Console and Build Analyzer**  
![Imagen](https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/build%20console%20y%20analyzer.png)
*Epígrafe: Build console y build analyzer.*

## 4.6 Medición y análisis de WCET por tarea

El firmware instrumenta WCET por tarea en `app.c` usando `DWT->CYCCNT` y un modo de perfilado limpio (`[PROF]`) activado temporalmente durante ensayo:
- `WCETw` = WCET en ventana (steady-state, últimos 1000 ciclos)
- `WCETb` = WCET acumulado desde boot
- `Cavg` = tiempo promedio de ejecución

Metodología realizada:
1. Flashear build `Software STM32/main` en NUCLEO-F103RB.
2. Abrir consola serial (USART2, 115200 baud).
3. Ejecutar con trazas de test desactivadas (`APP_TEST_MODE = 0`) y perfil limpio activo durante la medición.
4. Dejar correr el sistema en estado idle (sin pulsaciones ni cambios ADC).
5. Registrar múltiples ventanas `[PROF]` (n~1010 por ventana).

Formato de log utilizado y significado de parámetros:
- `n`: cantidad de ciclos de scheduler medidos en la ventana.
- `ov`: cantidad de overruns (ciclos cuyo runtime total supera 1 ms).
- `qmax`: máximo backlog observado en la cola de ticks (`g_app_tick_cnt`) durante la ventana.
- `Cavg={adc,sys,pwm}`: tiempo promedio por tarea en la ventana (us).
- `WCETw={adc,sys,pwm}`: peor tiempo por tarea dentro de la ventana (us).
- `CPU={avg,peak}`: utilización total promedio y pico del scheduler en la ventana (%).
- `U={avg,wcet}`: factor de uso promedio y por peor caso reportado para la ventana.

Criterio de consolidación de resultados:
- Se tomaron 15 líneas consecutivas `[PROF]`.
- Para `Cavg típico` se reportó el rango estable observado.
- Para `WCETw máx observado` se tomó el máximo absoluto entre las 15 ventanas.
- Para `U` se reportó rango observado por ventana y cota conservadora adicional.

Es muy importante destacar que el uso de la consola eleva masivamente los WCET, por lo que se minimizó en las evaluaciones. 

**Resultados medidos (estado idle/estable, 15 ventanas):**

| Tarea | Período asumido [us] | Cavg típico [us] | WCETw máx observado [us] |
| --- | ---: | ---: | ---: |
| `task_adc_update` | 1000 | 64..66 | 268 |
| `task_system_update` | 1000 | 26 | 125 |
| `task_pwm_update` | 1000 | 46..48 | 292 |

**Observaciones:**
- No se observaron overruns (`ov=0`) en ninguna ventana.
- `qmax=10` se mantuvo estable en todas las ventanas registradas.
- Uso de CPU: `CPU avg` entre `13.6%` y `14.0%`; `CPU peak` entre `35.6%` y `38.0%`.


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
| `task_adc_update` | 268 | 1000 | 0.268 |
| `task_system_update` | 125 | 1000 | 0.125 |
| `task_pwm_update` | 292 | 1000 | 0.292 |
| **Total U (WCET-based, conservador)** | - | - | **0.685** |

**Interpretación:**
- En observación real por ventana, los logs mostraron `Uwcet` entre `46.5%` y `66.1%`, y `Uavg` entre `13.6%` y `13.9%`.
- El valor `0.685` es una cota conservadora construida con máximos individuales observados en ventanas distintas.
- **Conclusión**: El sistema opera con margen temporal holgado en estado estable (sin overruns), incluso considerando una cota conservadora.

## 4.8 Gestión de bajo consumo y justificación

En esta iteración del TP no se implementó una estrategia dedicada de bajo consumo a nivel firmware (por ejemplo, entrada explícita a modos `Sleep/Stop` ni escalado dinámico de frecuencia), ya que el objetivo principal fue priorizar robustez funcional, seguridad eléctrica y cierre de integración.

No obstante, se evaluó el impacto energético real del sistema y los resultados muestran que el consumo del conjunto está dominado principalmente por el hardware periférico y la plataforma de prototipado:
- El salto de consumo al conectar el módulo Bluetooth es significativo (`64 mA` -> `104 mA`), aun sin transmitir.
- La diferencia entre Bluetooth desactivado y transmitiendo es menor (`104 mA` -> `107 mA`).
- En falla, el mayor consumo se explica por actuadores/indicadores (`buzzer + LED`), no por carga computacional del CPU.

Esto es consistente con el factor de uso medido (`Uavg` alrededor de `14%` y cota conservadora `Uwcet = 0.685`): la carga temporal del microcontrolador no aparece como cuello de botella energético principal en el prototipo actual.

En una versión orientada a producto (placa dedicada, sin sobrecarga de NUCLEO y periféricos de laboratorio), sí corresponde aplicar optimización sistemática de consumo:
-Reducir frecuencia de reloj del MCU al mínimo compatible con temporización y control de TRIAC;
- Incorporar política de idle de bajo consumo (entrada a `Sleep` entre eventos periódicos/interrupts);
- Migrar de HC-06 (Bluetooth clásico) a BLE para telemetría de bajo consumo;
- Revisar arquitectura de hardware auxiliar (drivers, conversores, etapas de acondicionamiento y protecciones) para eliminar consumo no esencial.

Conclusión: para el alcance académico de esta entrega, el consumo observado está mayormente determinado por decisiones de hardware e instrumentación de prototipo. La optimización fina de bajo consumo queda planificada como mejora de próxima revisión de diseño.

## 4.9 Cumplimiento de requisitos

| ID | Requisito (versión final) | Hardware | Software | Estado final |
| --- | --- | :---: | :---: | :---: |
| 1.1 | El sistema permitirá encender y apagar las luces mediante un botón físico. | 🟢 | 🟢 | ✅ |
| 1.2 | El sistema permitirá ajustar la velocidad del ventilador mediante un potenciómetro. | 🟢 | 🟢 | ✅ |
| 1.3 | El sistema permitirá ver el estado del ventilador y las luces vía Bluetooth. | 🟢 | 🟢 | ✅ |
| 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. | 🟢 | 🟢 | ✅ |
| 2.2 | El DIP switch permitirá seleccionar configuraciones o canales del módulo Bluetooth. | 🟢 | 🔴 | 🔴 |
| 3.1 | El sistema contará con LEDs que indiquen el estado del Bluetooth. | 🟢 | 🟢 | ✅ |
| 3.2 | El sistema contará con un buzzer para señalizar eventos del sistema. | 🟢 | 🟢 | ✅ |
| 4.1 | El sistema deberá guardar en memoria flash el último valor de PWM utilizado. | 🟢 | 🟢 | ✅ |
| 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. | 🟢 | 🟢 | ✅ |
| 5.1 | El sistema deberá operar de forma segura sobre cargas de 220 VAC. | 🟡 | N/A | 🟡 |
| 6.1 | La aplicación dará información sobre los estados disponibles, que incluyen la velocidad del ventilador y el estado de luces. | N/A | 🟢 | ✅ |
| 6.2 | El sistema deberá evitar conflictos entre el control físico y la comunicación Bluetooth, incluyendo conflictos de timings. | N/A | 🟢 | ✅ |

Leyenda:
- 🟢 implementado
- 🟡 parcialmente cumplido / con alcance acotado en prototipo
- 🔴 no implementado / descartado
- ✅ cumplido

Observación sobre el requisito 5.1 (220 VAC):
- La validación final sobre red de 220 VAC queda planificada para la etapa posterior a la aprobación académica del trabajo.
- Esta decisión se toma para reducir el riesgo de daño de la placa durante la instancia de entrega y evaluación.

Observación sobre el requisito 2.2 (canales/configuración Bluetooth):
- En la implementación final no se desarrolló la selección de canales/configuraciones por DIP para Bluetooth.
- Se descartó por no ser necesario para el funcionamiento objetivo del sistema (telemetría de estado).

## 4.10 Comparación con sistemas similares

| Característica | Control IR/RF básico | Solución Wi-Fi comercial | Este proyecto |
| --- | :---: | :---: | :---: |
| Interfaz local de pared | No | Generalmente no | Sí |
| App móvil | No | Sí | Sí (telemetría) |
| Personalización firmware | No | No | Sí |
| Persistencia local | Variable | Sí | Sí |
| Costo de prototipo académico | N/A | Alto | Medio |

## 4.11 Documentación del desarrollo realizado

Material técnico disponible en repositorio:
- Código fuente STM32 (`Software STM32/main`).
- Esquemáticos y PCB (`Hardware/placa dimmer`, `Hardware/placa shield`).
- Diagramas de estado (`Diagrama de Harel`).
- App móvil (`app celular`).
- Memoria técnica y contenido gráfico (`Memoria técnica`).

---

# Capítulo 5: Conclusiones

## 5.1 Resultados obtenidos

Se obtuvo un prototipo funcional que integra:
- Control local de luz y ventilador.
- Sincronización con cruce por cero para disparo de TRIAC.
- Telemetría por Bluetooth HC-06.
- Persistencia en flash y manejo de falla segura.

También se estableció una base sólida de documentación técnica para cierre de entrega final.

El proyecto permitió conocer los Triacs como componentes de control de potencia, además de permitir ahondar en lo que es el desarrollo de sistemas embebidos a pequeña escala. 

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
