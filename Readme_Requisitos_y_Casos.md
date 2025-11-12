# Dimmer + Switch (Ventilador & Luces)  
Control de ventilador y luces de línea (220 V) desde pared y vía Bluetooth

## Descripción  
Este proyecto implementa un módulo de control para **luces** y **ventilador** (220 V línea) que permite:  
- control manual desde la pared (potenciómetro, botón)  
- control inalámbrico vía bluetooth (por app generada con MIT App Inventor 2 / Companion)  
- guardar/restaurar el brillo previo al apagado (memoria flash interna)  
- monitorear temperatura para protección de sobretemperatura  
- alertas (leds, buzzer) en caso de condición anómala

### Objetivos principales  
- Permitir un control más fino del ventilador (dimmer) y de las luces en un mismo módulo.  
- Facilitar la operación tanto desde el interruptor de pared como desde el móvil.  
- Garantizar seguridad al operar cargas de línea (220 V) y protección ante sobretemperatura.  
- Ofrecer una experiencia de usuario versátil: manual o inalámbrica.

## Funcionalidades  
- DIP switch para habilitar o deshabilitar funciones configurables (por ejemplo, modo bluetooth, recuperación automática de brillo, modos predefinidos).  
- Botón físico: encendido/apagado de las luces.  
- Potenciómetro: en modo manual permite ajustar el PWM para la luz (brillo) o velocidad del ventilador.  
- Módulo Bluetooth: comunicación con la app del móvil para control remoto.  
- Memoria flash interna: guarda el valor de PWM (brillo/velocidad) antes de apagado, y lo restaura al volver a encender.  
- Sensor de temperatura (analógico): mide la temperatura ambiente o la de los triacs para activar buzzer y led en caso de sobretemperatura.  
- Led indicadores: alimentación de baja tensión / alta tensión, estado de bluetooth, alarma de temperatura.  
- Buzzer: señal sonora al interactuar (por ejemplo con botón o potenciómetro) y para alertas de sobretemperatura.

## Diagrama de hardware (resumen)  
- Entrada de línea 220 V para ventilador + luces → módulo de control (triacs, circuito de potencia, aislamiento)  
- Fuente auxiliar (por ejemplo 5 V o 3.3 V) para microcontrolador, módulo bluetooth, sensores, potencia de control  
- Microcontrolador (por ejemplo un ESP8266/ESP32, u otro MCU adecuado) que genera señal PWM, lee sensores, gestiona bluetooth, memoria flash, botones/DIP switch  
- Potenciómetro conectado al ADC para lectura manual  
- Sensor de temperatura conectado al ADC  
- DIP switch entradas digitales para configuración  
- Botón de encendido/apagado de las luces  
- LEDs y Buzzer conectados al MCU para estado/alerta  
- Módulo Bluetooth para comunicarse con la app móvil  
- Interfaz de potencia (triac o similar) para controlar la carga de 220 V  

*(Aquí se recomienda incluir un esquema o imagen del circuito si se tiene)*

## Uso  
### Manual desde pared  
- Presionar el botón físico encenderá/apagará las luces.  
- Girar el potenciómetro ajusta el brillo de la luz o la velocidad del ventilador (según configuración).  
- Los leds indicadores muestran el estado de alimentación, bluetooth, temperatura.  
- Si el sensor de temperatura detecta exceso, el buzzer se activa y el sistema reduce o apaga automáticamente la carga.

### Control vía móvil  
- Abre la app en tu móvil (configurada en MIT AI2 Companion).  
- Conéctate al módulo Bluetooth.  
- En la app podrás:  
  - encender/apagar luces y ventilador  
  - ajustar brillo / velocidad (PWM)  
  - ver estado de temperatura / alimentación  
  - configurar modo de funcionamiento (a través de DIP switch o menú)  
- La memoria interna restaurará el último valor de PWM al encendido si esa función está habilitada.

## Configuración (DIP switch)  
Por ejemplo:  
- DIP1: Habilitar recuperación automática del PWM guardado  
- DIP2: Habilitar control Bluetooth  
- DIP3: Habilitar modo manual sólo potenciómetro  
- DIP4: Habilitar alarma de sobretemperatura (buzzer + led)  
*(Ajustar según la implementación real)*

## Seguridad y consideraciones eléctricas  
- El proyecto trabaja con **cargas de 220 V**. Asegúrate de que las conexiones eléctricas estén realizadas con las normas de seguridad correspondientes.  
- Utilizar aislamiento adecuado, fusibles, protección contra cortocircuitos.  
- El sensor de temperatura debe ubicarse cerca de los componentes térmicamente críticos (por ejemplo los triacs) para detectar correctamente la sobretemperatura.  
- Se recomienda probar primero con cargas livianas y luego con las cargas finales.

