# Dimmer + Switch (Ventilador & Luces)  
Control de ventilador y luces de línea (220 V) desde pared y vía Bluetooth

<div align="center">

<img width="535"  alt="image" src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/08290a7a62c8a7d3fcd22fc57871dafbbf35ab15/logo-fiuba.png" />

**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**TA134 – Sistemas Embebidos**  
Curso 1 – Grupo 2

</div>

## Autores
- Ignacio Ezequiel Cavicchioli — Legajo 109428  
- Francisco Javier Moya — Legajo 109899  

**Fecha:** 25/01/2026  
**Cuatrimestre de cursada:** 2do cuatrimestre 2025  

*Trabajo realizado entre diciembre 2025 y febrero 2026.*

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

*Historial de revisiones del documento.*

La Tabla 0.1 resume el historial de revisiones y entregas de esta memoria.

| Revisión | Cambios realizados | Fecha |
| :---: | --- | :---: |
| 1.0 | Reescritura integral de la memoria, alineada a pautas de entrega final | 17/02/2026 |
| 1.1 | Completar con mediciones de consumo, WCET y factor de uso CPU | 17/02/2026 |
| 1.2 | Completar con permalinks definitivos de imágenes y link de video | 17/02/2026 |
| 1.2 | Entrega N°1 | 17/02/2026 |
| 1.3 | Correcciones según devolución de primer entrega| 19/02/2026 |
| 1.4 | Entrega N°2 | 19/02/2026 |

<em>Tabla 0.1 — Registro de versiones del documento.</em><br><br>

---



# Índice General

- [Capítulo 1: Introducción general](#capítulo-1-introducción-general)
  - [1.1 Análisis de necesidad y objetivo](#11-análisis-de-necesidad-y-objetivo)
  - [1.2 Productos comparables](#12-productos-comparables)
  - [1.3 Justificación del enfoque técnico](#13-justificación-del-enfoque-técnico)
  - [1.4 Alcance y limitaciones](#14-alcance-y-limitaciones)
- [Capítulo 2: Introducción específica](#capítulo-2-introducción-específica)
  - [2.1 Requisitos](#21-requisitos)
  - [2.2 Casos de uso](#22-casos-de-uso)
    - [2.2.1 Control local de luz](#221-control-local-de-luz)
    - [2.2.2 Ajuste local de ventilador](#222-ajuste-local-de-ventilador)
    - [2.2.3 Telemetría Bluetooth hacia app](#223-telemetría-bluetooth-hacia-app)
    - [2.2.4 Recuperación tras falla](#224-recuperación-tras-falla)
  - [2.3 Descripción de módulos principales](#23-descripción-de-módulos-principales)
    - [2.3.1 Módulo de control (NUCLEO-F103RB)](#231-módulo-de-control-nucleo-f103rb)
    - [2.3.2 Módulo de potencia (dimmer)](#232-módulo-de-potencia-dimmer)
    - [2.3.3 Módulo de detección de cruce por cero (ZCD)](#233-módulo-de-detección-de-cruce-por-cero-zcd)
    - [2.3.4 Módulo Bluetooth (HC-06)](#234-módulo-bluetooth-hc-06)
    - [2.3.5 Aplicación móvil (MIT App Inventor)](#235-aplicación-móvil-mit-app-inventor)
- [Capítulo 3: Diseño e implementación](#capítulo-3-diseño-e-implementación)
  - [3.1 Arquitectura general](#31-arquitectura-general)
  - [3.2 Diseño de hardware](#32-diseño-de-hardware)
    - [3.2.1 Criterio de interconexión y montaje](#321-criterio-de-interconexión-y-montaje)
    - [3.2.2 Etapa de conversión de niveles](#322-etapa-de-conversión-de-niveles)
    - [3.2.3 Etapa de TRIACs](#323-etapa-de-triacs)
    - [3.2.4 Etapa ZCD (detección de cruce por cero)](#324-etapa-zcd-detección-de-cruce-por-cero)
    - [3.2.5 Etapa de potencia y protecciones](#325-etapa-de-potencia-y-protecciones)
    - [3.2.6 Fabricación de placas](#326-fabricación-de-placas)
    - [3.2.7 Pinout del sistema (STM32F103RB)](#327-pinout-del-sistema-stm32f103rb)
    - [3.2.8 Cableado e imágenes del montaje](#328-cableado-e-imágenes-del-montaje)
  - [3.3 Diseño de firmware](#33-diseño-de-firmware)
    - [3.3.1 Arquitectura de ejecución](#331-arquitectura-de-ejecución)
    - [3.3.2 Máquina de estados del sistema](#332-máquina-de-estados-del-sistema)
    - [3.3.3 Entradas y acondicionamiento lógico](#333-entradas-y-acondicionamiento-lógico)
    - [3.3.4 Control de TRIAC y sincronización AC](#334-control-de-triac-y-sincronización-ac)
    - [3.3.5 Persistencia en flash](#335-persistencia-en-flash)
    - [3.3.6 Bluetooth HC-06](#336-bluetooth-hc-06)
    - [3.3.7 Aplicación móvil](#337-aplicación-móvil)
- [Capítulo 4: Ensayos y resultados](#capítulo-4-ensayos-y-resultados)
  - [4.1 Pruebas funcionales de hardware](#41-pruebas-funcionales-de-hardware)
  - [4.2 Pruebas funcionales de firmware](#42-pruebas-funcionales-de-firmware)
  - [4.3 Pruebas de integración](#43-pruebas-de-integración)
  - [4.4 Medición y análisis de consumo](#44-medición-y-análisis-de-consumo)
  - [4.5 Console and Build Analyzer](#45-console-and-build-analyzer)
  - [4.6 Medición y análisis de WCET por tarea](#46-medición-y-análisis-de-wcet-por-tarea)
  - [4.7 Cálculo del factor de uso de CPU (U)](#47-cálculo-del-factor-de-uso-de-cpu-u)
  - [4.8 Gestión de bajo consumo y justificación](#48-gestión-de-bajo-consumo-y-justificación)
  - [4.9 Cumplimiento de requisitos](#49-cumplimiento-de-requisitos)
  - [4.10 Comparación con sistemas similares](#410-comparación-con-sistemas-similares)
  - [4.11 Documentación del desarrollo realizado](#411-documentación-del-desarrollo-realizado)
- [Capítulo 5: Conclusiones](#capítulo-5-conclusiones)
  - [5.1 Resultados obtenidos](#51-resultados-obtenidos)
  - [5.2 Lecciones aprendidas](#52-lecciones-aprendidas)
  - [5.3 Próximos pasos](#53-próximos-pasos)
- [Capítulo 6: Uso de herramientas de IA](#capítulo-6-uso-de-herramientas-de-ia)
  - [6.1 Uso individual y conjunto](#61-uso-individual-y-conjunto)
- [Capítulo 7: Bibliografía y referencias](#capítulo-7-bibliografía-y-referencias)

---

# Capítulo 1: Introducción general

## 1.1 Análisis de necesidad y objetivo

El proyecto busca resolver una necesidad concreta de control de cargas de 220 VAC (luz y ventilador) desde una interfaz de pared, agregando telemetría inalámbrica sin depender de la red Wi-Fi doméstica.

Objetivos principales:
- Implementar un prototipo funcional y seguro de control de luz/ventilador.
- Usar una arquitectura modular en STM32.
- Tener persistencia de estado en la memoria flash.

## 1.2 Productos comparables

Se analizaron dos tipos de soluciones comerciales disponibles en la Argentina:

1. **Ventilador con control remoto IR/RF**  
   Existen en el mercado local ventiladores controlados por control remoto dedicado. 
   Este tipo de control funciona correctamente, pero presenta limitaciones importantes:
   - Solo tiene control remoto, no tiene control fijo. 
   - No ofrece conectividad con el celular.  
   - No guarda configuraciones ni estados previos del ventilador.  
   - El producto encontrado solo tiene 3 velocidades de ventilador (low, med, high). 

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/fbef0ce288a5bfc7994dd3f4e93a5714879ebca2/Memoria%20t%C3%A9cnica/imgs/solucion%20control%20remoto%201.png" width="600" />
<em>Figura 1.1 — Ventilador con control remoto IR/RF (referencia comercial).</em><br><br>

En la Figura 1.1 se observa el kit típico: un control remoto dedicado con soporte de pared y un módulo receptor que se instala en la caja de techo. Esta solución (encontrada en [Kit Corebay Fan-3 – Mercado Libre](https://www.mercadolibre.com.ar/kit-de-control-remoto-para-ventilador-de-techo-corebay-fan-3/p/MLA2061924708)) resuelve el control a distancia, pero no integra telemetría hacia el celular ni una interfaz fija de pared. Si se pierde el control remoto, o se olvida  en otro ambiente, el usuario tiene que buscarlo para poder controlar el ventilador y luz. 


2. **Controladores disponibles internacionalmente ([Kit universal – Amazon](https://www.amazon.com/-/es/Control-universal-ventilador-interruptor-atenuador/dp/B0D95Y3Z11)
)**  
   En el mercado internacional existen productos más avanzados, capaces de integrar control de luces y ventilador, conectividad Wi-Fi, y aplicaciones móviles.  
   Sin embargo:
   - Tienen costos significativamente más altos o no cuentan con disponibilidad local inmediata. 
   - En general los que usan wi-fi no tienen tecla y representan una amenaza a la seguridad de la red doméstica del usuario (si no está bien configurada).  

La Figura 1.2 muestra un producto más completo: combina control local (teclas de pared) con control remoto y/o aplicación móvil, normalmente mediante conectividad Wi‑Fi. Si bien aporta más funciones, su integración típica depende de la red WI-FI doméstica, excepto en el caso del control remoto, que nos parece el mejor.<br>

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/fbef0ce288a5bfc7994dd3f4e93a5714879ebca2/Memoria%20t%C3%A9cnica/imgs/solucion%20completa%202.jpg" width="600" />
<em>Figura 1.2 — Controlador integrado con teclas de pared y conectividad (referencia internacional).</em><br><br>

Para este proyecto se optó por utilizar una interfaz local combinada con un módulo Bluetooth clásico HC-06. Esta solución híbrida prioriza la simplicidad de integración, combinando la comodidad del control de pared con la telemetría inalámbrica por medio de bluetooth. La siguiente sección brinda más detalles sobre estas decisiones de diseño. 

## 1.3 Justificación del enfoque técnico

Se eligió Bluetooth clásico (HC-06) por:
- Menor complejidad de despliegue que Wi-Fi.
- Facilidad de integración con la app realizada en MIT App Inventor.
- Disponibilidad de herramientas de depuración por UART.

Se mantuvo un alcance acotado para cumplir entrega:
- La app móvil recibe telemetría binaria de 2 bytes.
- El control principal de actuadores se mantiene en interfaz local.

En una futura versión, el producto debería permitir el control por medio de la conexión inalámbrica, equiparandolo a la solución comercial mostrada más completa. 

## 1.4 Alcance y limitaciones

Alcance implementado:
- Encendido/apagado de luz por botones físicos.
- Ajuste de velocidad del ventilador por potenciómetro.
- Envío de telemetría por HC-06.
- Estado de falla segura y persistencia básica en flash.

Fuera de alcance actual:
- Control remoto completo de actuadores desde app. 
- Las pruebas de integración de potencia se realizaron con 24 VAC (banco), no con 220 VAC.
- La validación final sobre red de 220 VAC queda planificada como etapa posterior a la aprobación académica del trabajo, para reducir el riesgo durante la entrega.


Este tema se vuelve a detallar en la sección "4.9 Cumplimiento de requisitos", en la que se explica cada ítem y la razón de no haberse implementado, si corresponde. 


---

# Capítulo 2: Introducción específica

Esta sección contiene los requisitos originales y los modificados en el informe de avances, además de los casos de uso. 

## 2.1 Requisitos
En la Tabla 2.1 se listan los requisitos originalmente definidos al inicio del proyecto (versión base, incluida también en `README.md`). Durante la elaboración del informe de avances (primera semana de febrero de 2026), el alcance se ajustó para asegurar una integración completa a tiempo para la entrega; dichos cambios se pueden ver en la Tabla 2.2, y se resumen a cambios en el control local, telemetría y persistencia, reduciendo el alcance de funciones no críticas para la entrega.

Más adelante, en la sección "4.9 Cumplimiento de requisitos", se detalla si se cumplieron o no, y se da la razón en caso de no haberse implementado.

| Grupo | ID | Descripción |
|-------|-----|-------------|
| Control | 1.1 | El sistema permitirá encender y apagar las **luces** mediante un botón físico. |
|  | 1.2 | El sistema permitirá ajustar la **velocidad del ventilador** mediante un potenciómetro. |
|  | 1.3 | El sistema permitirá controlar el ventilador y las luces vía **Bluetooth**. |
| Bluetooth | 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. |
|  | 2.2 | El DIP switch permitirá seleccionar diferentes **configuraciones o canales** del módulo BT. |
| Indicadores | 3.1 | El sistema contará con **LEDs** que indiquen el estado de conexión del Bluetooth. |
|  | 3.2 | El sistema contará con un **buzzer** para señalizar eventos del Bluetooth. |
| Memoria | 4.1 | El sistema deberá guardar en **memoria flash interna** el último valor de PWM utilizado. |
|  | 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. |
| Seguridad | 5.1 | El sistema deberá operar de forma segura sobre cargas de **220 V**. |
| Aplicación | 6.1 | La aplicación móvil deberá permitir realizar todas las acciones disponibles desde los controles físicos (encendido/apagado de luces y ajuste de velocidad del ventilador). |
|  | 6.2 | El sistema deberá garantizar que el control físico y el control desde la aplicación sean intercambiables: cuando se utilice uno, el otro deberá quedar temporalmente inhabilitado para evitar conflictos de comando. |

<em>Tabla 2.1 — Requisitos iniciales del proyecto (versión original).</em><br><br>

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

<em>Tabla 2.2 — Requisitos ajustados en el informe de avances (alcance reducido por tiempos).</em><br><br>

## 2.2 Casos de uso

Esta sección detalla los varios casos de uso previstos para el sistema creado. 

### 2.2.1 Control local de luz

La Tabla 2.3 describe el caso de uso de control local de la luz mediante botones físicos.

| Elemento | Definición |
| --- | --- |
| Disparador | Pulsación de botón ON (`PC12`) o OFF (`PC9`). |
| Precondiciones | Sistema en modo normal, hardware operativo. |
| Flujo básico | Debounce de botón -> evento -> actualización de estado de luz -> actualización de salida TRIAC -> solicitud de guardado en flash -> telemetría BT de cambio. |
| Alternativas | Si falla persistencia y modo estricto activo: transición a `FAULT`. |

<em>Tabla 2.3 — Caso de uso: control local de luz.</em><br><br>

### 2.2.2 Ajuste local de ventilador

La Tabla 2.4 describe el caso de uso de ajuste local del ventilador mediante el potenciómetro.

| Elemento | Definición |
| --- | --- |
| Disparador | Cambio en potenciómetro (`PA0`). |
| Precondiciones | ADC operativo, sistema en modo normal. |
| Flujo básico | Muestreo ADC -> mapeo a porcentaje -> cálculo de `fan_delay_us` -> actualización de temporización de disparo TRIAC. |
| Alternativas | Si potenciómetro fuera de rango calibrado: saturación a límites definidos. |

<em>Tabla 2.4 — Caso de uso: ajuste local de ventilador.</em><br><br>

### 2.2.3 Telemetría Bluetooth hacia app

La Tabla 2.5 detallada a continuación describe el caso de uso de telemetría Bluetooth, utilizada para informar estado hacia la aplicación móvil.

| Elemento | Definición |
| --- | --- |
| Disparador | Cambio de estado de luz o de porcentaje del potenciómetro. |
| Precondiciones | BT habilitado por DIP1, módulo HC-06 conectado. |
| Flujo básico | Firmware arma trama binaria de 2 bytes y transmite por USART1 para que la app informe el estado del sistema. |
| Alternativas | Si BT deshabilitado, no se transmite. |

<em>Tabla 2.5 — Caso de uso: telemetría Bluetooth hacia app.</em><br><br>

### 2.2.4 Recuperación tras falla

La Tabla 2.6 de abajo describe el caso de uso de recuperación ante falla, incluyendo el modo `FAULT` y su salida controlada.

| Elemento | Definición |
| --- | --- |
| Disparador | Error de inicialización o forzado de `FAULT` por DIP4 (`PA4`). |
| Precondiciones | Sistema energizado. |
| Flujo básico | Corte de salidas de potencia, alarma visual/sonora según DIP, reintento de inicialización luego de timeout. |
| Alternativas | Si DIP4 vuelve a 0, salida de `FAULT` y retorno a `NORMAL`. |

<em>Tabla 2.6 — Caso de uso: recuperación tras falla.</em><br><br>

Nota de trazabilidad de alcance:
Los casos de uso y la app se modificaron respecto de los del "Readme" en base a los alcances/requisitos alterados en el informe de avances.

## 2.3 Descripción de módulos principales

En las siguientes subsecciones se detallan los múltiples módulos del sistema. 

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
- Dominio lógico de 3,3 V (STM32 + entradas + comunicaciones).
- Dominio de potencia AC (TRIAC + ZCD + protecciones).

En la Figura 3.1 se presenta el diagrama en bloques general, donde se identifica la separación entre el dominio de baja tensión (3,3 V) y el dominio de potencia en AC, junto con los principales enlaces de interconexión (ZCD, drivers de TRIAC y comunicación Bluetooth).

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/imgs/diagrama%20en%20bloques.jpg" width="600" />
<em>Figura 3.1 — Diagrama en bloques general.</em><br><br>


## 3.2 Diseño de hardware

Esta sección detalla la solución de Hardware diseñada en sus varias facetas, desde los esquemáticos hasta la verificación por mediciones. 

### 3.2.1 Criterio de interconexión y montaje

Se trabajó con placas y conexiones soldadas para la integración funcional final (sin protoboard ni cables Dupont en el montaje objetivo), en línea con las pautas de entrega.

Se usaron dos placas:
- placa shield para interfaz y conexión con NUCLEO.
- placa dimmer para potencia, ZCD y protecciones.

### 3.2.2 Etapa de Conversión de niveles

La Figura 3.2 muestra el conversor de niveles utilizado para adaptar señales entre la NUCLEO-F103RB (3,3 V) y la placa diseñada (5 V), evitando sobrevoltajes en entradas digitales. 

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquema%20niveles.png" width="600" /> 
<em>Figura 3.2 — Esquemático del conversor de niveles.</em><br><br>


### 3.2.3 Etapa de TRIACs

La Figura 3.3 presenta el driver de disparo de TRIAC basado en optoacoplador, elegido para aislar el dominio lógico y permitir el control de cargas de 220 VAC con disparos sincronizados. El diseño fue tomado de las notas de aplicación que se encuentran en este mismo repositorio en la sección de hardware. 


<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquem%20triac.png" width="600" />
<em>Figura 3.3 — Esquemático de driver de TRIAC.</em><br><br>


### 3.2.4 Etapa ZCD (detección de cruce por cero)

La sección de ZCD fue validada progresivamente en banco antes de integrar potencia. Se observó que:
- la salida detectada requiere compensación temporal aproximada de 500 $\mu s$ para ubicar el cruce real.
- las simulaciones resultaron consistentes con la tendencia medida.

En la Figura 3.4 se observa el circuito del detector de cruce por cero (ZCD), cuya salida se utiliza como referencia temporal para disparar los TRIACs con un retardo controlado.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/esquematico%20ZCD.png" width="600" />
<em>Figura 3.4 — Esquemático del ZCD.</em><br><br>


La Figura 3.5 documenta el banco de pruebas inicial del ZCD, usado para validar el acondicionamiento y la forma de onda antes de integrar la etapa de potencia.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/banco%20de%20trabajo%20inicial.jpeg" width="600" /> 
<em>Figura 3.5 — Banco inicial de pruebas ZCD.</em><br><br>


La Figura 3.6 muestra la señal de salida del ZCD medida con osciloscopio; se verifica que se genera un pulso por cada cruce por cero, resultando en una frecuencia de 100 Hz para red de 50 Hz. También se aprecia el espaciamiento entre pulsos medido con cursores del osciloscopio, que permite estimar la estabilidad temporal del detector.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos.jpeg" width="600" /> 
<em>Figura 3.6 — Mediciones de pulsos ZCD (osciloscopio).</em><br><br>


En la Figura 3.7 se mide el ancho de pulso del ZCD, que sirve para hacerse una idea de la simetría del disparo entre curces por cero consecutivos. En este caso se observa buena simetría entre pulsos, 

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%201.jpeg" width="600" /> 
<em>Figura 3.7 — Medición de ancho de pulso del ZCD.</em><br><br>

La Figura 3.8 evidencia el adelantamiento del pulso respecto del cruce por cero real para la semionda negativa, permitiendo estimar el retardo fijo de compensación.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%202.jpeg" width="600" /> 
<em>Figura 3.8 — Disparo previo al cruce real (senoidal negativa).</em><br><br>

La Figura 3.9 muestra el mismo fenómeno que la imagen 3.8 pero para la semionda positiva; ambas mediciones se utilizaron para fijar una compensación temporal conservadora para el disparo de los Triacs. En el código esto se ve reflejado en un tiempo invariante que siempre debe esperar antes de disparar. 

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/mediciones%20pulsos%204.jpeg" width="600" /> 
<em>Figura 3.9 — Disparo previo al cruce real (senoidal positiva).</em><br><br>



### 3.2.5 Etapa de potencia y protecciones

El esquemático principal (Hardware/placa dimmer/dimmer.kicad_sch) muestra los componentes que realizan el manejo de la potencia y su interconexión, además de incluir los elementos de protección básicos, como fusibles, varistores y snubbers RC. Cada uno de los 2 canales de potencia de la placa incluye:

- Un TRIAC de potencia (`BTA06-600C`) elegido por su corriente de disparo y corriente de canal máxima muy superior a 1 A. 
- Optoacoplador de disparo (`MOC3023M`), que aisla galvánicamente la lógica de 5 V de la parte de potencia.
- Elementos de protección (varistor, fusible, red RC/snubber opcional - puede no soldarse).

Ambos canales tienen la capacidad de hacer _dimming_ de su respectiva carga, aunque en este caso solo se usó para una. 

Notas de fabricación:
- Las pruebas se hicieron en 24 VAC. Esto conllevó una ligera y reversible modificación del circuito de ZCD. 

La Figura 3.10 muestra la señal a la salida del 4N25 (emisor común/negador) durante ensayo, confirmando niveles y forma de onda compatibles con el acondicionamiento digital.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/salida%20real%20del%20opto.jpeg" width="600" /> 
<em>Figura 3.10 — Ensayo de salida de optoacoplador.</em><br><br>

La Figura 3.11 presenta la simulación de la entrada/salida del ZCD y la etapa de opto, utilizada como referencia para contrastar con las mediciones. En este caso las simulaciones son acordes a fenómenos medidos en las figuras 3.7 a 3.9. 

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/30fe670b3a0bb71f531d25c21496764f675e7d96/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/simu%20ZCD%20proper.jpeg" width="600" /> 
<em>Figura 3.11 — Simulación de ZCD y salida de opto.</em><br><br>


En la Figura 3.12 se observa la salida simulada del 4N25. Aunque difiere de la señal real, alcanza los umbrales de funcionamiento por el clico de histéresis de los Schmitt triggers, y el diseño resultó funcional.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/ZCD/simu%20salida%20del%20optoacoplador.jpeg" width="600" /> 
<em>Figura 3.12 — Salida simulada del 4N25.</em><br><br>


### 3.2.6 Fabricación de placas

Se documentó el proceso de fabricación con transferencia y ataque químico:
1. Primero se imprimió el diseño sobre un papel PnP Blue.
2. Luego se transfirió por medio de calor. 
3. Se hicieron las correcciones manuales de transferencia.
4. Por último, se realizó un control de continuidad previo a energizar. Esto permite evitar un cortocircuito o arreglar una pista faltante. 

Lecciones aprendidas para la próxima iteración:
- Revisar diámetros de agujeros para componentes de potencia (varistores y componentes grandes): los componentes de _leads_ de mayor diámetro quedaron con _pads_ demasiado chicos. Esto no es un problema en un prototipo, donde el agujero se agranda con un torno de mano, pero es inaceptable en un producto final. 
- Simplificar topología de ZCD: se puede cambiar por un detector de 1 solo diodo, que actue cada ciclo y no por semiciclo. EL segundo disparo se puede resover por software estimando el período de la señal con un filtro de media móvil (o complementario). 
- Evaluar integración de control de dimming en una etapa dedicada: En un futuro la idea es que la solución comercial tenga su propio microcontrolador en la placa, y que solo requiera 220 VAC para funcionar.<br><br>

La Figura 3.13 muestra el papel de transferencia _p n p blue_ con el diseño impreso, paso previo al copiado del patrón a la placa cobreada.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/1030475e09d21a3204b19eb7996e9f11bb688033/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/p%20n%20p%20blue.jpeg" width="600" /> 
<em>Figura 3.13 — Papel de transferencia con diseño impreso.</em><br><br>

La Figura 3.14 registra la primera transferencia sobre cobre, donde se identificaron defectos a corregir antes del ataque químico.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/1030475e09d21a3204b19eb7996e9f11bb688033/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/trasferencia%20a%20cobre.jpeg" width="600" /> 
<em>Figura 3.14 — Transferencia previa a correcciones.</em><br><br>

La Figura 3.15 muestra la transferencia luego de correcciones manuales, mejorando continuidad y separación de pistas.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/1030475e09d21a3204b19eb7996e9f11bb688033/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/correci%C3%B3n%20de%20desperfectos%20de%20trasnferencia.jpeg" width="600" /> 
<em>Figura 3.15 — Transferencia corregida.</em><br><br>

La Figura 3.16 presenta la placa fabricada tras el ataque y limpieza, lista para perforado, soldado y pruebas de continuidad.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/663d795450e29c452e59a7ecae6f23108cb3e22d/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/fab%20placa/cobre%20etched.jpeg" width="600" /> 
<em>Figura 3.16 — Placa fabricada.</em><br><br>

El soldado de la placa se dió en etapas, lo que permitió asegurarse de que cada etapa funcione previo a soldar la siguiente. En nuestro caso todo funcionó a la primera, excepto por una resistencia mal soldada que se corregió fácilmente. La solución final se alimentó con un transformador de 220 VAC a 24 VAC de 48 Watts. 

### 3.2.7 Pinout del sistema (STM32F103RB)

La Tabla 3.1 lista el pinout relevante del sistema para entradas, salidas, DIP switches y comunicaciones. El _shield_ fabricado para este proyecto respeta todas estas conexiones, y además están reflejadas en el el header del main del firmware. 

| Pin | Función |
| --- | --- |
| `PA0` | Potenciómetro (ADC) |
| `PC0` | DIP1: habilitación Bluetooth |
| `PC1` | DIP2: habilitación buzzer |
| `PB0` | DIP3: habilitación LED |
| `PA4` | DIP4: forzado de estado `FAULT` |
| `PC12` | Botón ON de luz |
| `PC9` | Botón OFF de luz |
| `PC2` | ZCD (a circuito de detección de cruces por cero) (EXTI) |
| `PB3` | TRIAC canal ventilador |
| `PB4` | TRIAC canal luz |
| `PB13` | LED |
| `PA8` | Buzzer (`TIM1_CH1`) |
| `PA9/PA10` | USART1 (HC-06) |
| `PA2/PA3` | USART2 (consola ST-Link VCP) |
| `PC8` | Onda de prueba 100 Hz (modo test) |

<em>Tabla 3.1 — Pinout relevante del sistema (STM32F103RB).</em><br><br>

### 3.2.8 Cableado e imágenes del montaje

En esta parte del documento se detalla el montaje y conexionado de las placas. 

La Figura 3.17 muestra el cableado final del prototipo en banco; se destaca el uso de conexiones soldadas y el montaje sin protoboard en la integración objetivo.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/banco%20de%20trabajo%20desprolijo/banco%20final.jpeg" width="600" /> 
<em>Figura 3.17 — Cableado final del prototipo.</em><br><br>


La Figura 3.18 resume el conexionado simplificado entre placas, útil como referencia de integración (señales de control, alimentación y retornos).

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/00693ac864a65b0389699a47c52606a88d0adbb9/Diagrama%20de%20conexi%C3%B3n%20simplificado/conexionado.png" width="600" /> 
<em>Figura 3.18 — Diagrama de conexión entre placas simplificado.</em><br><br>

En la Figura 3.19 se observa el shield del NUCLEO-F103RB y su conexionado, donde se distinguen entradas (DIP, botones, ADC) y salidas hacia potencia.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Diagrama%20de%20conexi%C3%B3n%20simplificado/f103rb.jpg" width="600" /> 
<em>Figura 3.19 — Overview de placa shield y conexionado.</em><br><br>

La Figura 3.20 muestra el conexionado de la placa de TRIACs y las cargas, con especial atención a la separación entre el dominio de 220 VAC y el de control.

<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Diagrama%20de%20conexi%C3%B3n%20simplificado/triacs.jpg" width="600" /> 
<em>Figura 3.20 — Conexionado de placa de TRIACs.</em><br><br>


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

La Figura 3.21 presenta el statechart general del sistema; define el flujo de inicialización, operación normal y transición a falla segura.
<img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/Statechart.png" width="600" /> 
<em>Figura 3.21 — Statechart general (Harel/Itemis).</em><br><br>

En la Figura 3.22 se detallan los subestados de inicialización, donde se leen DIP, se verifican condiciones y se restaura configuración persistida.
> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/169b5aabae5e8c5a7af391914271a01397db4f61/Memoria%20t%C3%A9cnica/imgs/State%20Init.png" width="600" />
<em>Figura 3.22 — Subestados de inicialización.</em><br><br>

La Figura 3.23 muestra el estado normal, responsable de atender eventos de usuario, control de TRIAC y telemetría BT.
> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/State%20Normal.png" width="600" />
<em>Figura 3.23 — Estado normal.</em><br><br>

La Figura 3.24 describe el estado de falla, en el que se corta potencia y se señaliza la condición mientras se gestiona recuperación.
> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/State%20Fault_ST.png" width="600" />
<em>Figura 3.24 — Estado de falla.</em><br><br>

La Figura 3.25 muestra la FSM de debounce utilizada para los botones, evitando rebotes y generando eventos limpios hacia la lógica del sistema.
> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/3cb04d32ab982e06ec97e47ec6184a648ebf46cf/Memoria%20t%C3%A9cnica/imgs/ST_BTN.png" width="600" />
<em>Figura 3.25 — FSM de debounce de botón.</em><br><br>

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
- retardo fijo de referencia: `APP_TRIAC_FIXED_WAIT_US = 700 $\mu s$`.
- ancho de pulso de gate: `APP_TRIAC_PULSE_US = 1000 $\mu s$`.
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
- comandos AT enviados sin CR/LF y con retardos adecuados (desde un Arduino, previo a armado).

Funcionamiento en firmware:
- UART por `USART1`.
- telemetría binaria (sin JSON).
- 2 bytes por frame:
  - byte 0: `adc_percent` (0..100).
  - byte 1: `light_enabled` (0/1).
- Envío periódico por tiempo (no por cambio), configurable con `APP_BT_TELEMETRY_PERIOD_MS` (actualmente `50 ms`). Esto ayudó mucho a mejorar los WCET debido a que el uso de la consola parece tomar mucho tiempo.


### 3.3.7 Aplicación móvil

La app fue desarrollada en MIT App Inventor. Se documentan interfaz y bloques de procesamiento de bytes.

La Figura 3.26 muestra la pantalla principal de la app, donde se visualiza la velocidad del ventilador en un formato de procentaje según el ADC, y el estado de luz recibido por telemetría.

> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/566a7314061481abbec17f240388ee198cea82ee/Memoria%20t%C3%A9cnica/cosas%20e%20imagenes%20para%20memoria%20t%C3%A9cnica%20-%20hardware/captura%20app.jpeg" width="400" />
<em>Figura 3.26 — Pantalla principal app.</em><br><br>


La Figura 3.27 presenta los bloques de inicialización de la app, incluyendo configuración de Bluetooth y preparación de variables.

> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%201.png" width="600" />
<em>Figura 3.27 — Bloques MIT App Inventor (parte 1).</em><br><br>

La Figura 3.28 muestra la lógica de decodificación/actualización de los 2 bytes de telemetría y el refresco de UI.

> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%202.png" width="600" />
<em>Figura 3.28 — Bloques MIT App Inventor (parte 2).</em><br><br>

La Figura 3.29 detalla la lógica de selección del dispositivo Bluetooth, utilizada para vincularse al HC-06.

> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/65b6a1be5b7a1b68e959d041707e17e00ebe5659/Memoria%20t%C3%A9cnica/imgs/mit%20app%20bloque%203.png" width="600" />
<em>Figura 3.29 — Bloques MIT App Inventor (parte 3).</em><br><br>

---

# Capítulo 4: Ensayos y resultados

Esta sección presenta todos los ensayos requeridos como parte del proyecto, incluyendo el uso de CPU, WCET, etc. 

## 4.1 Pruebas funcionales de hardware

La Tabla 4.1 resume los ensayos funcionales de hardware realizados y su estado de validación.

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Integridad de placas (continuidad) | Validación previa a energización | ✅ |
| ZCD en banco | Detección de eventos y correlación con simulación | ✅ |
| Integración con 24 VAC | Prueba inicial de etapa integrada | ✅ |
| Observar integridad de dimming en 24 VAC (osciloscopio) | Se verificó por medio de osciloscopio | ✅ |

<em>Tabla 4.1 — Ensayos funcionales de hardware.</em><br><br>

## 4.2 Pruebas funcionales de firmware

La Tabla 4.2 resume los ensayos funcionales de firmware realizados y su estado de validación.

| Ensayo | Resultado | Estado |
| --- | --- | :---: |
| Debounce botones ON/OFF | Eventos limpios sobre FSM | ✅ |
| Muestreo ADC + mapeo | Escalado operativo 0..100% | ✅ |
| FSM de sistema (`INIT/NORMAL/FAULT`) | Transiciones válidas en logs | ✅ |
| Persistencia flash | Lectura/escritura de estado y calibración | ✅ |
| Telemetría BT (2 bytes) | Trama enviada en forma periódica (`APP_BT_TELEMETRY_PERIOD_MS`) | ✅ |

<em>Tabla 4.2 — Ensayos funcionales de firmware.</em><br><br>

## 4.3 Pruebas de integración

Se validó la interacción completa:
- entradas físicas.
- control de potencia.
- telemetría hacia app.

**Video de integración en funcionamiento**  

[Link al video
](https://youtu.be/iv2bGrqrMtU)<br>

## 4.4 Medición y análisis de consumo

Metodología aplicada:
- Medición de consumo total en la entrada de `5 V` del sistema (NUCLEO + shield).
- Alimentación desde fuente externa conectada a pines `5 V` y `GND`.
- Medición de corriente con multímetro en serie sobre la línea de `5 V`.
- Medición de tensión en bornes de entrada para estimar potencia (`P = V * I`).

Procedimiento realizado:
1. Desconectar USB/ST-Link para evitar doble alimentación.
2. Conectar fuente externa a `5 V` y `GND`.
3. Ajustar la fuente para garantizar `5 V` en el pin `5 V` de la placa (compensando las caídas de tensión en los cables).
4. Intercalar amperímetro en serie en la línea de `5 V`.
5. Medir tensión de entrada en paralelo sobre `5 V-GND`.
6. Registrar datos en los modos:
   - normal sin módulo Bluetooth conectado.
   - normal con módulo Bluetooth conectado pero desactivado.
   - normal con Bluetooth activo enviando datos.
   - fault con alarma activa (buzzer + LED).
7. Debido a que el consumo oscila rápidamente en el tiempo, se tomó como referencia el valor pico observado en cada modo.

Alcance de la medición:
- Esta medición representa el consumo total a `5 V` del conjunto montado.
- El riel de `3,3 V` queda incluido indirectamente, ya que se genera desde `5 V` mediante el regulador de la placa. Además, registrar el consumo de 3,3 V solo no tiene sentido para un sistema que requiere de 5 V para funcionar normalmente. 

La Tabla 4.3 resume los valores pico de corriente y potencia medidos en distintos modos de operación del sistema.

| Modo | I pico @ 5 V [mA] | P pico @ 5 V [W] | Observaciones |
| --- | ---: | ---: | --- |
| Normal sin módulo BT (desconectado) | 64 | 0.320 | Escenario de menor consumo; representa una forma válida de uso sin telemetría Bluetooth. |
| Normal con módulo BT conectado y desactivado | 104 | 0.520 | Aumento de consumo por presencia/alimentación del módulo Bluetooth. |
| Normal con BT activo enviando datos | 107 | 0.535 | Incremento leve respecto al modo BT desactivado. |
| Fault (buzzer + LED activos) | 145 | 0.725 | Peor caso medido en operación. |

<em>Tabla 4.3 — Consumo total medido a 5 V (valores pico).</em><br><br>

Análisis:
- La potencia se calculó como `P = V * I`, usando `V = 5 V` y corriente pico medida en cada modo.
- El peor caso medido fue `145 mA` a `5 V`, equivalente a `0.725 W`.
- El sistema se mantiene por debajo de `1 W`, por lo que puede alimentarse sin inconvenientes con fuentes comerciales 220VAC->5 V de baja potencia (o incluso las integradas para PCBs).
- La diferencia entre el HC-06 desactivado y transmitiendo (`104 mA` -> `107 mA`) es baja, consistente con carga adicional moderada por comunicación. Suponemos que el consumo no es menor al no transmitir porque está haciendo broadcast de su existencia constantemente. 

## 4.5 Console and Build Analyzer

La Figura 4.1 muestra el reporte de uso de memoria del build; se observa un uso bajo de RAM y FLASH (≈10,31% y ≈16,11%), dejando margen para futuras extensiones.

> <img src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/c2fc7354b11ef4655cebe90b4b788acc5695045a/Memoria%20t%C3%A9cnica/imgs/build%20console%20y%20analyzer.png" width="800" />
<em>Figura 4.1 — Console and Build Analyzer.</em><br><br>



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

Es decir, se usó el mismo programa pero con un modo de estimación del WCET. 

Formato de log utilizado y significado de parámetros:
- `n`: cantidad de ciclos de scheduler medidos en la ventana.
- `ov`: cantidad de overruns (ciclos cuyo runtime total supera 1 ms).
- `qmax`: máximo backlog observado en la cola de ticks (`g_app_tick_cnt`) durante la ventana.
- `Cavg={adc,sys,pwm}`: tiempo promedio por tarea en la ventana ($\mu s$).
- `WCETw={adc,sys,pwm}`: peor tiempo por tarea dentro de la ventana ($\mu s$).
- `CPU={avg,peak}`: utilización total promedio y pico del scheduler en la ventana (%).
- `U={avg,wcet}`: factor de uso promedio y por peor caso reportado para la ventana.

Criterio de consolidación de resultados:
- Se tomaron 15 líneas consecutivas `[PROF]`.
- Para `Cavg típico` se reportó el rango estable observado.
- Para `WCETw máx observado` se tomó el máximo absoluto entre las 15 ventanas.
- Para `U` se reportó rango observado por ventana y cota conservadora adicional.

Es muy importante destacar que el uso de la consola eleva masivamente los WCET, por lo que se minimizó en las evaluaciones. 

**Resultados medidos (estado idle/estable, 15 ventanas):**

La Tabla 4.4 resume los resultados consolidados de tiempo de ejecución por tarea (promedio y peor caso en ventana).

| Tarea | Período asumido [$\mu s$] | Cavg típico [$\mu s$] | WCETw máx observado [$\mu s$] |
| --- | ---: | ---: | ---: |
| `task_adc_update` | 1000 | 64..66 | 268 |
| `task_system_update` | 1000 | 26 | 125 |
| `task_pwm_update` | 1000 | 46..48 | 292 |

<em>Tabla 4.4 — Resultados de WCET por tarea (ventanas en idle/estable).</em><br><br>

**Observaciones:**
- No se observaron overruns (`ov=0`) en ninguna ventana.
- `qmax=10` se mantuvo estable en todas las ventanas registradas.
- Uso de CPU: `CPU avg` entre `13,6%` y `14,0%`; `CPU peak` entre `35,6%` y `38,0%`.


## 4.7 Cálculo del factor de uso de CPU (U)

Para evaluar la carga temporal del sistema se calculó el factor de utilización de CPU utilizando la expresión clásica de sistemas en tiempo real:

$$U = \sum_{i=1}^{n} \frac{C_i}{T_i}$$

donde (C_i) representa el WCET de la tarea (i), medido a partir de ventanas de ejecución en régimen estacionario, y (T_i) su período de activación.

La Tabla 4.5 resume los valores utilizados para el cálculo:

| Tarea                                   | (C_i) (WCET) [µs] | (T_i) [µs] | (C_i/T_i) |
| --------------------------------------- | ----------------: | ---------: | --------: |
| `task_adc_update`                       |               268 |       1000 |     0.268 |
| `task_system_update`                    |               125 |       1000 |     0.125 |
| `task_pwm_update`                       |               292 |       1000 |     0.292 |
| **Total (U) (WCET-based, conservador)** |                 – |          – | **0,685** |

<em>Tabla 4.5 — Parámetros utilizados para el cálculo de U (cota conservadora).</em><br><br>

El valor total obtenido, ($U = 0.685$), corresponde a una cota conservadora, ya que se construyó combinando los máximos tiempos de ejecución observados para cada tarea en ventanas temporales distintas y no a partir de una ocurrencia simultánea real de dichos máximos.

En contraste, las mediciones experimentales mostraron valores de utilización sensiblemente menores: la utilización basada en ventanas ($U_{wcet}$) se mantuvo entre $46,5$ % y $66,1$ %, mientras que la utilización promedio ($U_{avg}$) se ubicó en torno al 14 % en régimen permanente. En el caso particular del STM32F103RB, estos resultados indican un comportamiento temporal estable, con un margen de CPU suficiente para absorber variaciones transitorias de ejecución sin comprometer el cumplimiento de los períodos de las tareas, validando así la factibilidad temporal del diseño.


## 4.8 Gestión de bajo consumo y justificación

En esta iteración del TP no se implementó una estrategia dedicada de bajo consumo a nivel firmware (por ejemplo, entrada explícita a modos `Sleep/Stop` ni escalado dinámico de frecuencia), ya que el objetivo principal fue priorizar robustez funcional, seguridad eléctrica y cierre de integración.

No obstante, se evaluó el impacto energético real del sistema y los resultados muestran que el consumo del conjunto está dominado principalmente por el hardware periférico y la plataforma de prototipado:
- El salto de consumo al conectar el módulo Bluetooth es significativo (`64 mA` -> `104 mA`), aun sin transmitir.
- La diferencia entre Bluetooth desactivado y transmitiendo es menor (`104 mA` -> `107 mA`).
- En falla, el mayor consumo se explica por actuadores/indicadores (`buzzer + LED`), no por carga computacional del CPU.

Esto es consistente con el factor de uso medido (`Uavg` alrededor de `14%` y cota conservadora `Uwcet = 0,685`): la carga temporal del microcontrolador no aparece como cuello de botella energético principal en el prototipo actual.

En una versión orientada a producto (placa dedicada, sin sobrecarga de NUCLEO y periféricos de laboratorio), sí corresponde aplicar optimización sistemática de consumo:

- Reducir frecuencia de reloj del MCU al mínimo compatible con temporización y control de TRIAC.
- Incorporar política de idle de bajo consumo (entrada a `Sleep` entre eventos periódicos/interrupts).
- Migrar de HC-06 (Bluetooth clásico) a BLE para telemetría de bajo consumo.
- Revisar arquitectura de hardware auxiliar (drivers, conversores, etapas de acondicionamiento y protecciones) para eliminar consumo no esencial.

Conclusión: para el alcance académico de esta entrega, el consumo observado está mayormente determinado por decisiones de hardware e instrumentación de prototipo. La optimización fina de bajo consumo queda planificada como mejora de próxima revisión de diseño.

## 4.9 Cumplimiento de requisitos

La Tabla 4.6 resume el cumplimiento final de los requisitos (versión ajustada del informe de avances), discriminando el aporte de hardware y firmware.

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

<em>Tabla 4.6 — Cumplimiento final de requisitos (versión final).</em><br><br>

Leyenda:
- 🟢 implementado
- 🟡 parcialmente cumplido / con alcance acotado en prototipo
- 🔴 no implementado / descartado
- ✅ cumplido

Los requisitos cumplidos no requieren de mayor explicación, por lo que nos centramos en los que no se llegaron a cumplir:
- Requisito 2.2 (canales/configuración Bluetooth): En la implementación final no se desarrolló la selección de canales/configuraciones por DIP para Bluetooth. Se descartó por no ser necesario para el funcionamiento objetivo del sistema (telemetría de estado).

- Requisito 5.1 (220 VAC): La validación final sobre red de 220 VAC queda planificada para la etapa posterior a la aprobación académica del trabajo. Esta decisión se toma para reducir el riesgo de daño de la placa durante la instancia de entrega y evaluación.


## 4.10 Comparación con sistemas similares

La Tabla 4.7 presenta una comparación sintética de esta solución frente a alternativas típicas (control básico IR/RF y soluciones comerciales Wi‑Fi).

| Característica | Control IR/RF básico | Solución Wi-Fi comercial | Este proyecto |
| --- | :---: | :---: | :---: |
| Interfaz local de pared | No | Generalmente no | Sí |
| App móvil | No | Sí | Sí (telemetría) |
| Personalización firmware | No | No | Sí |
| Persistencia local | Variable | Sí | Sí |
| Costo de prototipo académico | N/A | Alto | Medio |

<em>Tabla 4.7 — Comparación con sistemas similares.</em><br><br>

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
- La compensación temporal del cruce por cero (aprox. 500 $\mu s$) es crítica para estabilidad del dimming.
- La fabricación de PCB artesanal aceleró iteraciones, pero exige mayor cuidado mecánico en footprints de componentes de potencia.
- La telemetría binaria de 2 bytes simplificó integración y depuración con app móvil.

## 5.3 Próximos pasos

Los próximos pasos podrían ser:
- Evaluar una revisión de hardware con ZCD simplificado.
- Mejor mecánica de placa para componentes de potencia (los agujeros).
- Posible control de dimming en microcontrolador dedicado (tal vez un ESP32 o ESP8266) y fuente en la misma placa. 

---

# Capítulo 6: Uso de herramientas de IA

Se documenta el uso de IA según requerimiento docente y archivo `listado de cosas hechas con IA.txt`.

## 6.1 Uso individual y conjunto

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

# Capítulo 7: Bibliografía y referencias

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
Fecha de edición: 19 de febrero de 2026
