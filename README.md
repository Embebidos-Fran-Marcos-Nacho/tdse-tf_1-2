# Dimmer + Switch (Ventilador & Luces)  
Control de ventilador y luces de línea (220 V) desde pared y vía Bluetooth

<div align="center">

<img width="535" height="179" alt="image" src="https://github.com/user-attachments/assets/c504f104-30a7-43e1-9181-e95dde039c2c" />

**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**TA134 – Sistemas Embebidos**  
Curso 1 – Grupo 2

</div>

## Autores
- Ignacio Ezequiel Cavicchioli — Legajo 109428  
- Francisco Javier Moya — Legajo 109899  

---

# 1. Selección del proyecto a implementar

## 1.1 Objetivo del proyecto y resultados esperados

Este proyecto implementa un módulo de control de **ventilador** y **luces de línea (220 V)** que permite operar:

- Desde la pared, por medio de un **botón físico** y un **potenciómetro**  
- Desde un dispositivo móvil mediante **Bluetooth (MIT App Inventor)**

Resultados esperados:

- Control de velocidad del ventilador.  
- Encendido/apagado de luces y ventilador.  
- Feedback mediante **LEDs** y **buzzer** para estados y alertas.  
- Permitir **deshabilitar o configurar el BT** mediante un DIP switch.  
- Guardado en memoria del último estado usado.  
- Funcionamiento seguro sobre cargas de 220 V.

---

## 1.2 Proyectos similares

Para establecer un punto de referencia, se identificaron dos soluciones comerciales que resuelven parcialmente el mismo problema que aborda este proyecto. Ambas están disponibles en el mercado, pero presentan diferencias significativas respecto al sistema que se busca implementar.

1. **Ventilador con control remoto IR/RF**  
   Existen en el mercado local ventiladores controlados por control remoto dedicado. 
   Este tipo de control funciona correctamente, pero presenta limitaciones importantes:
   - Solo tiene control remoto, no tiene control fijo. 
   - No ofrece conectividad con el celular.  
   - No guarda configuraciones ni estados previos del ventilador.  
   - Solo tiene 3 velocidades de ventilador. 


2. **Controladores disponibles internacionalmente (Amazon)**  
   En el mercado internacional existen productos más avanzados, capaces de integrar control de luces y ventilador, conectividad Wi-Fi, y aplicaciones móviles.  
   Sin embargo:
   - Tienen costos significativamente más altos o no cuentan con disponibilidad local inmediata. 
   - En general los que usan wi-fi no tienen tecla y representan una amenaza a la seguridad de la red doméstica del usuario.  

   Estos dispositivos representan lo más cercano al objetivo del proyecto, pero no resultan accesibles para implementarlos o utilizarlos en el contexto local.

En resumen, el proyecto actual se inspira en estas dos líneas de productos, combinando:
- la disponibilidad y simpleza de los controles remotos comerciales locales  
- con la funcionalidad avanzada de los módulos inteligentes que existen internacionalmente  

pero proponiendo una solución **propia, accesible y adaptada al entorno de trabajo de la materia.**

---

## 1.3 Selección de proyecto

La elección del proyecto se fundamenta en una combinación de requerimientos de la consigna, viabilidad técnica y practicidad.

En primer lugar, la cátedra establece como objetivo diseñar un sistema embebido con interacción física y comunicación inalámbrica. Dentro de las alternativas disponibles, el uso de **Bluetooth** se alineó naturalmente con el entorno de desarrollo provisto (MIT App Inventor Companion), permitiendo crear una aplicación funcional sin complejidades de red o protocolos avanzados.

### Razones para no usar Wi-Fi
- Presenta mayor complejidad en configuración, seguridad y mantenimiento de la conexión.  
- Requiere infraestructura adicional (router, red doméstica).  
- Demanda más recursos del microcontrolador.  
- Implica un ciclo de desarrollo más largo, poco compatible con los tiempos del proyecto académico.
- Un dispositivo mal diseñado podría volverse una vulnerabilidad en la red. 

Por ello, Bluetooth resultó ser la opción más equilibrada entre simplicidad, robustez y disponibilidad de herramientas.

### Comparación frente a otras alternativas
- **Control manual únicamente:** No cumple con los requisitos de conectividad inalámbrica definidos por la materia.  
- **Controles RF dedicados:** Los módulos de control remoto comercialmente disponibles para plataformas como Arduino son convenientes para el usuario final, pero no ofrecen un entorno de desarrollo ni la flexibilidad pedagógica necesaria.  
- **Soluciones comerciales completas:** No permiten aprendizaje, programación ni adaptación; además, suelen tener costos elevados o disponibilidad limitada.

### Consideración personal / diseño ideal
En un escenario de producto comercial real, probablemente sería más cómodo utilizar un control **RF dedicado**, especialmente para el uso nocturno (por ejemplo, tener un control en la mesa de luz).  
Sin embargo, dadas las restricciones del proyecto académico y los objetivos de aprendizaje, la elección de Bluetooth y una aplicación móvil resulta la opción más adecuada y formativa.

En conclusión, este proyecto se seleccionó porque:
- cumple con los requerimientos de la materia  
- permite un diseño flexible y completamente programable  
- evita la complejidad innecesaria del Wi-Fi  
- se inspira en productos reales tanto locales como internacionales  
- y aporta una solución integrada para luz + ventilador, que no se encuentra fácilmente en el mercado argentino

---

### 1.3.1 Diagrama en bloques

<div align="center">
<img src="diagrama en bloques.jpg" />
</div>

---

# 2. Elicitación de requisitos y casos de uso

## 2.1 Requisitos del proyecto

A continuación se presenta una tabla de requisitos:

### Tabla de requisitos

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
---
<!-- |  | 5.2 | El sistema deberá contar con mecanismos de protección ante **sobretemperatura**. | -->

## 2.2 Casos de uso

A continuación se presentan los casos de uso siguiendo el formato del archivo ejemplo.


### Caso de uso 1: Control manual desde la pared

| Elemento | Definición |
|----------|-----------|
| **Disparador** | El usuario presiona el botón físico o ajusta el potenciómetro. |
| **Precondiciones** | El sistema está correctamente alimentado y los módulos están inicializados. |
| **Flujo principal** | El usuario presiona el botón para encender/apagar las luces o gira el potenciómetro para controlar el ventilador. |
---

### Caso de uso 2: Control remoto desde el celular vía Bluetooth

| Elemento | Definición |
|----------|-----------|
| **Disparador** | El usuario abre la aplicación móvil y se conecta al dispositivo mediante Bluetooth. |
| **Precondiciones** | El DIP switch debe habilitar el Bluetooth. El sistema debe estar energizado y en estado operativo. |
| **Flujo principal** | El usuario enciende o apaga las luces y ajusta la velocidad del ventilador desde la aplicación. El sistema recibe los comandos, actualiza el estado interno y modifica las salidas correspondientes. Los indicadores visuales y acústicos reflejan los cambios realizados. |
| **Flujos alternativos** | a) Si el sistema recibe un comando Bluetooth con un **formato inválido**, incompleto o con una estructura distinta a la esperada, el comando se **descarta** y el sistema mantiene el **último estado validado recibido por Bluetooth**.  <br> b) Si durante el funcionamiento se **pierde la conexión Bluetooth**, el sistema conserva el **último estado válido recibido** antes de la desconexión, sin restaurar el modo manual ni cambiar de fuente de control. <br> c) Si el Bluetooth está apagado por DIP switch al intentar conectar, la aplicación no podrá interactuar con el dispositivo. |




### Caso de uso 3: Restauración del último estado guardado

| Elemento | Definición |
|----------|-----------|
| **Disparador** | El sistema se energiza o se reinicia. |
| **Precondiciones** | Debe existir al menos un valor previo guardado en memoria. |
| **Flujo principal** | El sistema lee la memoria flash interna y restaura el último valor de PWM utilizado antes del apagado. El ventilador y/o las luces vuelven al estado correspondiente a ese valor. La aplicación móvil, si se conecta, puede consultar o sincronizar este estado restaurado. |
| **Flujos alternativos** | a) Si la memoria está **vacía**, contiene un valor **inválido**, o los datos resultan **corruptos**, el sistema descarta la lectura y utiliza un **preset de seguridad** en el que tanto las luces como el ventilador permanecen **apagados**. <br> b) Si la memoria resulta **inaccesible** por falla interna, el sistema también entra en el estado seguro con todo apagado, operando únicamente bajo comandos manuales o Bluetooth posteriores. |


---

# 3. Descripción general del sistema

## 3.1 Funcionalidades principales

- Control manual mediante botón y potenciómetro.  
- Control remoto vía Bluetooth.  
- Indicadores LED y buzzer para estados del sistema y del Bluetooth.  
- Memoria interna para restaurar el estado previo.  
- DIP Switch para configurar o deshabilitar el Bluetooth.  
- Operación segura sobre línea de 220 V.

---

# 4. Uso del sistema

## 4.1 Uso manual desde la pared

- Botón físico → encendido/apagado de luces.  
- Potenciómetro → ajuste de velocidad del ventilador.  
- LEDs indican estado del Bluetooth.  

## 4.2 Control desde la app

- Conectarse vía Bluetooth.  
- Encender/apagar luces y ventilador.  
- Ajustar nivel PWM.  
- La app puede mostrar el último valor de PWM restaurado automáticamente.

---

# 5. Configuración (DIP switch)

- **DIP1:** Habilitar o deshabilitar Bluetooth.  
- **DIP2–DIP4:** Configuración del canal o modo del módulo BT.

---

# 6. Cronograma estimado

No se prevé la terminación del TP para diciembre.  
El primer avance se presentará entre enero y febrero de 2026.

---

