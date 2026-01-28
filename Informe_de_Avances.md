# Dimmer + Switch (Ventilador & Luces)  
Control de ventilador y luces de línea (220 V) desde pared y vía Bluetooth

<div align="center">

<img width="535" height="179" alt="image" src="https://github.com/Embebidos-Fran-Marcos-Nacho/tdse-tf_1-2/blob/08290a7a62c8a7d3fcd22fc57871dafbbf35ab15/logo-fiuba.png" />

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

---

## 1. Introducción

El presente documento corresponde al **informe de avance** del proyecto *Dimmer + Switch (Ventilador & Luces)*, desarrollado en el marco de la materia **TALLER DE SISTEMAS EMBEBIDOS (TA134)**.

El objetivo de este informe es detallar el **estado actual de implementación** del sistema respecto a los **requisitos definidos en el README del proyecto**, identificando qué funcionalidades se encuentran implementadas, cuáles están en desarrollo y cuáles no de podrán abordar o deberán ser cambiadas por restricciones de tiempo.

---

## 2. Convenciones de estado

A continuación se detallan las convenciones utilizadas para indicar el estado de cada requisito:

| Estado | Descripción |
|------|-------------|
| 🟢 | Implementado |
| 🟡 | En proceso de implementación |
| 🔴 | No implementado / descartado |

Comentarios:
Se consideran "En proceso de implementación" aquellas _features_ que estén en el Diagrama de Harel. 

---

## 3. Avance por grupo de requisitos

### 3.1 Control

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 1.1 | El sistema permitirá encender y apagar las luces mediante un botón físico. | 🟢 | 🟡 |
| 1.2 | El sistema permitirá ajustar la velocidad del ventilador mediante un potenciómetro. | 🟢 | 🟡 |
| 1.3 | El sistema permitirá controlar el ventilador y las luces vía Bluetooth. | 🟢 | 🔴 |

#### Comentarios:
Por un lado, los ítems 1.1 y 1.2 se encuentran en proceso de desarrollo, y ya están implementados en el Hardware del proyecto. Por otro lado, por limitaciones de tiempo, el ítem 1.3 se deberá cambiar a lo siguiente: 
- 1.3: "El sistema permitirá ver el estado del ventilador y las luces vía Bluetooth."

Se espera que este cambio reduzca la dificultad/tiempo de la implementación de una aplicación en MIT App Inventor. 

---

### 3.2 Bluetooth

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. | 🟢 | 🟡 |
| 2.2 | El DIP switch permitirá seleccionar configuraciones o canales del módulo Bluetooth. | 🟢 | 🟡 |

#### Comentarios:
Se prevé el uso del dip switch como fue planteado inicialmente. 

---

### 3.3 Indicadores

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 3.1 | El sistema contará con LEDs que indiquen el estado del Bluetooth. | 🟢 | 🟡 |
| 3.2 | El sistema contará con un buzzer para señalizar eventos del sistema. | 🟢 |🟡 |

#### Comentarios:
No se prevén cambios en esta sección. 

---

### 3.4 Memoria

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 4.1 | El sistema deberá guardar en memoria flash el último valor de PWM utilizado. | 🟢 | 🟡 |
| 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. | 🟢 | 🟡 |

#### Comentarios:
El microprocesador ya dispone de su propia memoria flash, por lo que solo resta completar la implementación en software. 

---

### 3.5 Seguridad eléctrica

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 5.1 | El sistema deberá operar de forma segura sobre cargas de 220 VAC. | 🟡 | N/A |

#### Comentarios:
Este ítem está en proceso de evaluación. 

---

### 3.6 Aplicación móvil

| ID | Descripción | Estado de implementación - Hardware | Estado de implementación - Software |
|------|-------------|--------|--------|
| 6.1 | La aplicación permitirá realizar todas las acciones disponibles desde los controles físicos.| N/A | 🔴 |
| 6.2 | El sistema deberá evitar conflictos entre control físico y control Bluetooth.| N/A | 🔴 |

#### Comentarios:

Dados los cambios realizados sobre el ítem 1.3, se proponen las siguientes modificaciones:
- 6.1: "La aplicación dará información sobre los estados disponibles, que incluyen la velocidad del ventilador, estado de luces, etc.".
- 6.2: "El sistema deberá evitar conflictos entre el control físico y la comunicación Bluetooth.". Esto abarca conflictos de _timings_. 

---

## 4. Resumen

A la fecha de este informe, el proyecto se encuentra en una **etapa intermedia de desarrollo**, con los siguientes avances generales:

- Definición completa del alcance y requisitos del sistema.  
- Selección de arquitectura general y módulos principales.  
- Hardware en proceso de evaluación. Esto incluye 2 placas, una de controles y otra de triacs (220 VAC).  

Actualmente se están terminando de diagramar los Diagramas de Harel. Una vez terminados y aprobados por el cuerpo docente, se procedería a desarrollar el Software propiamente dicho. 

---

## 5. Observaciones importantes

Ambos estudiantes, Francisco e Ignacio, nos encontramos realizando pasantías desde el cuatrimestre pasado, las cuales insumen una porción significativa del tiempo semanal y requieren un margen razonable de descanso, especialmente considerando que el año previo fue dedicado íntegramente al estudio y trabajo.

A ello se suma el inicio del período de evaluaciones de febrero/marzo, durante el cual debemos rendir finales de otras asignaturas y cumplir con la entrega de trabajos pendientes. En particular, Francisco debe rendir dos exámenes finales, mientras que Ignacio debe presentar una monografía y desarrollar el trabajo práctico profesional, aprovechando el receso del período lectivo.

Nos encontramos avanzando de manera sostenida con el trabajo; sin embargo, dadas las circunstancias mencionadas, no es posible sostener una dedicación intensiva de forma continua, considerando que ya venimos trabajando a un ritmo elevado desde el cuatrimestre anterior. En este contexto, resulta además irregular que la materia no cuente con el período de 15 fechas de finales que sí poseen el resto de las asignaturas (y que han tenido alumnos de esta misma materia cuatrimestres pasados), lo cual comprime aún más los tiempos disponibles y limita la posibilidad de una planificación equilibrada del esfuerzo.

Queremos aclarar que nos resulta incómodo tener que plantear esta situación, y que lo hacemos con el mayor respeto hacia el cuerpo docente y hacia el trabajo que realizan. Sin embargo, consideramos importante acercar esta observación de manera constructiva, ya que entendemos que la organización actual impacta de forma significativa en nuestros tiempos y podría resultar también injusta para futuros alumnos de la materia. Nuestro objetivo es únicamente poner en conocimiento esta dificultad, con la intención de que pueda ser tenida en cuenta en instancias posteriores.

---
