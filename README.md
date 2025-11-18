
<div align="center">
  
<img width="535" height="179" alt="image" src="https://github.com/user-attachments/assets/c504f104-30a7-43e1-9181-e95dde039c2c" />


**UNIVERSIDAD DE BUENOS AIRES**  

**Facultad de Ingeniería**  

**TA134 Sistemas Embebidos**  

Curso 1

</div>

# Dimmer + Switch (Ventilador & Luces)  
Control de ventilador y luces de línea (220 V) desde pared y vía Bluetooth

### Autores:
Ingancio Ezequiel Cavicchioli, legajo: 109428

Francisco Javier Moya, legajo: 109899
<A chequear Marcos>

## Descripción  
Este proyecto implementa un módulo de control para **luces** y **ventilador** (220 V línea) que permite:  
- control manual desde la pared (botones y potenciómetro)  
- control inalámbrico vía bluetooth (por app generada con MIT App Inventor 2 / Companion)  
- guardar/restaurar la velocidad del ventilador previa al apagado (memoria flash interna)  
- alertas (leds, buzzer) para conexión de bluetooth
- DIP Switch que permite configurar el Bluetooth (apagarlo o elegir el canal). 

### Objetivos principales  
- Permitir un control más fino del ventilador (dimmer) y de las luces en un mismo módulo.  
- Facilitar la operación tanto desde el interruptor de pared como desde el móvil.  
- Garantizar seguridad al operar cargas de línea (220 V) y protección ante sobretemperatura.  
- Ofrecer una experiencia de usuario versátil: manual o inalámbrica.

## Funcionalidades  
- DIP switch para habilitar o deshabilitar funciones configurables.  
- Botón físico: encendido/apagado de las luces.  
- Potenciómetro: en modo manual permite ajustar la velocidad del ventilador.  
- Módulo Bluetooth: comunicación con la app del móvil para control remoto.  
- Memoria flash interna: guarda el valor de PWM (brillo/velocidad) antes de apagado, y lo restaura al volver a encender.  
- Led indicadores: Estado de conexión bluetooth (ej: flashing en modo pairing, apagado en condición normal). 
- Buzzer: Estado de conexión bluetooth.  

## Diagrama en bloques

<div align="center">

<img src="diagrama en bloques.jpg" /> 

</div>

## Diagrama de Harel  


## Uso  
### Manual desde pared  
- Presionar el botón físico encenderá/apagará las luces.  
- Girar el potenciómetro ajusta la velocidad del ventilador (según configuración).  
- Los leds indicadores muestran el estado de bluetooth.  

### Control vía móvil  
- Abre la app en tu móvil (configurada en MIT AI2 Companion).  
- Conéctate al módulo Bluetooth.  
- En la app podrás:  
  - encender/apagar luces y ventilador  
  - ajustar velocidad (PWM)  
- La memoria interna restaurará el último valor de PWM al encendido. La idea es que el microprocesador le envie el último estado recordado al celular. 

## Configuración (DIP switch)  
- DIP1: Habilitar bluetooth. 
- DIP2 a DIP4: seteo de canal bluetooth. 

## Tabla de Requerimientos de Entrega
## Tabla de Requerimientos de Entrega

| Nº | Grupo      | Requerimiento                                                                 |
|----|------------|-------------------------------------------------------------------------------|
| 1  | Control    | Repositorio GitHub con proyecto completo                                     |
| 2  | Control    | README.md actualizado con descripción, uso y funcionalidades                  |
| 3  | Control    | Código fuente del firmware listo para compilar                               |
| 4  | Aplicación | Carpeta `/app` con proyecto de la app en MIT App Inventor 2                   |
| 5  | Actuador   | Carpeta `/hardware` con esquemas eléctricos, diagramas de bloques, diagrama de Harel |
| 6  | Sensor     | Carpeta `/docs` con datasheets de componentes y consideraciones de seguridad eléctrica |
| 7  | Control    | Documentación de la configuración de DIP switch y Bluetooth                   |
| 8  | Aplicación | Inserción de test de funcionamiento: archivo de resultados o vídeo demostrativo |
| 9  | Control    | Licencia del proyecto (`LICENSE`), preferiblemente MIT                        |



## Esquemático
