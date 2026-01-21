# **Dimmer + Switch (Ventilador & Luces)**  
## **Informe de Avance del Proyecto**

**Universidad de Buenos Aires**  
**Facultad de Ingeniería**  
**TA134 – Sistemas Embebidos**

**Autores**  
- Ignacio Ezequiel Cavicchioli — Legajo XXXXX  
- Francisco Javier Moya — Legajo XXXXX  

**Fecha:** DD/MM/AAAA  
**Cuatrimestre:** XX cuatrimestre AAAA  

---

## 1. Introducción

El presente documento corresponde al **informe de avance** del proyecto *Dimmer + Switch (Ventilador & Luces)*, desarrollado en el marco de la materia **Sistemas Embebidos**.

El objetivo de este informe es detallar el **estado actual de implementación** del sistema respecto a los **requisitos definidos en el README del proyecto**, identificando qué funcionalidades se encuentran implementadas, cuáles están en desarrollo y cuáles no serán abordadas en esta etapa.

---

## 2. Convenciones de estado

A continuación se detallan las convenciones utilizadas para indicar el estado de cada requisito:

| Estado | Descripción |
|------|-------------|
| 🟢 | Implementado |
| 🟡 | En proceso de implementación |
| 🔴 | No implementado / descartado |

---

## 3. Avance por grupo de requisitos

### 3.1 Control manual

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 1.1 | El sistema permitirá encender y apagar las luces mediante un botón físico. | 🟡 |
| 1.2 | El sistema permitirá ajustar la velocidad del ventilador mediante un potenciómetro. | 🟡 |
| 1.3 | El sistema permitirá controlar el ventilador y las luces vía Bluetooth. | 🟡 |

---

### 3.2 Bluetooth y configuración

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 2.1 | El sistema contará con un DIP switch para habilitar o deshabilitar el Bluetooth. | 🟡 |
| 2.2 | El DIP switch permitirá seleccionar configuraciones o canales del módulo Bluetooth. | 🔴 |

---

### 3.3 Indicadores y señalización

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 3.1 | El sistema contará con LEDs que indiquen el estado del Bluetooth. | 🟡 |
| 3.2 | El sistema contará con un buzzer para señalizar eventos del sistema. | 🔴 |

---

### 3.4 Memoria y restauración de estado

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 4.1 | El sistema deberá guardar en memoria flash el último valor de PWM utilizado. | 🔴 |
| 4.2 | El sistema deberá restaurar automáticamente el último valor guardado al encender. | 🔴 |

---

### 3.5 Seguridad eléctrica

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 5.1 | El sistema deberá operar de forma segura sobre cargas de 220 V. | 🟡 |

---

### 3.6 Aplicación móvil

| Req ID | Descripción | Estado |
|------|-------------|--------|
| 6.1 | La aplicación permitirá realizar todas las acciones disponibles desde los controles físicos. | 🟡 |
| 6.2 | El sistema deberá evitar conflictos entre control físico y control Bluetooth. | 🔴 |

---

## 4. Estado general del sistema

A la fecha de este informe, el proyecto se encuentra en una **etapa intermedia de desarrollo**, con los siguientes avances generales:

- Definición completa del alcance y requisitos del sistema.  
- Selección de arquitectura general y módulos principales.  
- Implementación parcial de las funcionalidades de control.  
- Integración inicial de la comunicación Bluetooth.  

Quedan pendientes las etapas de:
- consolidación de la lógica de control,
- implementación de memoria persistente,
- validación de la seguridad eléctrica,
- pruebas de integración completa.

---

## 5. Próximos pasos

Para el próximo período de trabajo se prevé:

- Completar la implementación del control manual.  
- Avanzar con la integración total del módulo Bluetooth.  
- Implementar el guardado y restauración de estado en memoria.  
- Realizar pruebas funcionales y de seguridad.  
- Ajustar detalles de la aplicación móvil.

---

## 6. Observaciones finales

El desarrollo del proyecto avanza de acuerdo a lo planificado, manteniendo coherencia con los objetivos definidos inicialmente.  
Este informe refleja el estado actual del sistema y servirá como base para los siguientes hitos de implementación y validación.

---
