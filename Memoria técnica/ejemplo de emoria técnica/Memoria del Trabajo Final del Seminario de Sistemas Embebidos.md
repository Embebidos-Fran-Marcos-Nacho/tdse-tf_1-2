<img src="https://github.com/user-attachments/assets/15600b18-f73b-4ba3-a959-47f0048a1ab6" alt="image2" width="30%">

**UNIVERSIDAD DE BUENOS AIRES**  
**Facultad de Ingeniería**  
**86.65 Sistemas Embebidos**

Memoria del Trabajo Final:

***Smartlock* - Cerradura Inteligente con Wi-Fi**

**Autor:**

**Sr. Marcos Gómez Villafañe**

Legajo: 105.055 

*Este trabajo fue realizado en las Ciudad Autónoma de Buenos Aires,*   
*entre Agosto y Diciembre de 2024\.*

  
**RESUMEN**

En este trabajo se realiza el diseño e implementación de una cerradura inteligente con Wi-Fi para uso cotidiano en hogares, corporaciones o industrias. Este sistema tiene por objetivo registrar y administrar el acceso de los individuos a las distintas instalaciones, facilitando al dueño de la instalación un manejo dinámico y en tiempo real de los permisos de acceso. Para la comodidad de los usuarios, la cerradura inteligente cuenta con dos métodos de acceso: mediante tarjetas RFID o mediante un pin. Adicionalmente, el sistema cuenta con sensores magnéticos que permiten registrar los estados de las puertas en todo momento. El trabajo presenta una solución práctica y escalable para la administración de accesos a una instalación, previniendo filtraciones de seguridad y promoviendo un monitoreo transparente a través de la aplicación móvil.

La implementación del mismo se realizó utilizando los lenguajes de código C++ y Python mediante el uso del entorno de desarrollo de Mbed en una placa Núcleo-F429ZI proporcionada por la cátedra de Sistemas Embebidos de la Facultad de Ingeniería de la Universidad de Buenos Aires, y se utilizaron además diferentes módulos para el desarrollo del proyecto.

En esta memoria se presenta la motivación del proyecto, los diseños de las distintas partes, y se proponen futuras mejoras y ampliaciones.

**ABSTRACT**

In this work, the design and implementation of a smart lock with Wi-Fi for everyday use in homes, corporations or industries is carried out. This system aims to register and manage the access of individuals to different facilities, providing the owner of the facility with a dynamic and real-time management of access permissions. For the comfortability of users, the smart lock has two access methods: RFID card or pin access. In addition, the system has magnetic sensors that allow the door statuses to be recorded at all times. The work presents a practical and scalable solution for managing access to a facility, preventing security leaks and promoting transparent monitoring through the mobile application.

The implementation was carried out using C++ and Python code languages through the use of the Mbed development environment on a Núcleo-F429ZI board provided by the Embedded Systems Department of the School of Engineering of the University of Buenos Aires, and different modules were also used for the development of the project.

This report presents the motivation of the project, the designs of the different parts, and proposes future improvements and extensions.



# Índice General

- [**Registro de versiones**](#registro-de-versiones)
- [**Introducción general**](#introducción-general)
  - [1.1 Análisis de necesidad y objetivos](#11-análisis-de-necesidad-y-objetivos)
  - [1.2 Módulos e Interfaces de Smartlock](#12-módulos-e-interfaces-de-smartlock)
- [**Introducción específica**](#introducción-específica)
  - [2.1 Requisitos](#21-requisitos)
  - [2.2 Casos de uso](#22-casos-de-uso)
  - [2.3 Descripción de los Módulos del sistema](#23-descripción-de-los-módulos-del-sistema)
    - [2.3.1 Alimentación](#231-alimentación)
    - [2.3.2 Microcontrolador](#232-microcontrolador)
    - [2.3.3 Motor](#233-motor)
    - [2.3.4 Lector RFID](#234-lector-rfid)
    - [2.3.5 Teclado Matricial](#235-teclado-matricial)
    - [2.3.6 Sensor Magnético](#236-sensor-magnético)
    - [2.3.7 Parlante](#237-parlante)
    - [2.3.8 Comunicación Wi-Fi](#238-comunicación-wi-fi)
- [**Diseño e Implementación**](#diseño-e-implementación)
  - [3.1 Diseño del Hardware](#31-diseño-del-hardware)
    - [3.1.1 Diseño de la alimentación](#311-diseño-de-la-alimentación)
    - [3.1.2 Diseño de los indicadores e interruptores](#312-diseño-de-los-indicadores-e-interruptores)
    - [3.1.3 Diseño del lector RFID](#313-diseño-del-lector-rfid)
    - [3.1.5 Diseño del motor](#315-diseño-del-motor)
    - [3.1.6 Diseño del teclado matricial](#316-diseño-del-teclado-matricial)
    - [3.1.7 Diseño del Módulo Wi-Fi](#317-diseño-del-módulo-wi-fi)
    - [3.1.8 Diseño del hardware con la placa NUCLEO-F429ZI](#318-diseño-del-hardware-con-la-placa-nucleo-f429zi)
  - [3.2 Firmware del Smartlock](#32-firmware-del-smartlock)
    - [3.2.1 Módulo Access Keys](#321-módulo-access-keys)
    - [3.2.2 Módulo Keypad](#322-módulo-keypad)
    - [3.2.3 Módulo Motors](#323-módulo-motors)
    - [3.2.4 Módulo MQTT](#324-módulo-mqtt)
    - [3.2.5 Módulo RFID](#325-módulo-rfid)
    - [3.2.6 Módulo Speaker](#326-módulo-speaker)
    - [3.2.7 Módulo System](#327-módulo-system)
    - [3.2.8 Módulo UART Communications](#328-módulo-uart-communications)
  - [3.3 Firmware del ESP32 DEVKIT V1](#33-firmware-del-esp32-devkit-v1)
  - [3.4 MOSQUITTO broker](#34-mosquitto-broker)
  - [3.5 Diseño de la aplicación y manejo de paquetes](#35-diseño-de-la-aplicación-y-manejo-de-paquetes)
- [**Ensayos y Resultados**](#ensayos-y-resultados)
  - [4.1 Pruebas funcionales de funcionamiento](#41-pruebas-funcionales-de-funcionamiento)
  - [4.2 Cumplimiento de requisitos](#42-cumplimiento-de-requisitos)
  - [4.3 Comparación con otros sistemas similares](#43-comparación-con-otros-sistemas-similares)
  - [4.4 Documentación del desarrollo realizado](#44-documentación-del-desarrollo-realizado)
- [**Conclusiones**](#conclusiones)
  - [5.1 Resultados obtenidos](#51-resultados-obtenidos)
  - [5.2 Próximos pasos](#52-próximos-pasos)
- [**Bibliografía**](#bibliografía)





# Registro de versiones 

| Revisión | Cambios realizados | Fecha |
| :---: | ----- | ----- |
| 1.0 | Creación del documento | 9/12/2024 |
| 1.1 | Redacción del primer capítulo | 9/12/2024 |
| 1.2 | Redacción del segundo capítulo | 10/12/2024 |
| 1.3 | Redacción del tercer capítulo | 10/12/2024 |
| 1.4 | Redacción final | 13/12/2024 |

### 

# **CAPÍTULO 1** 

# **Introducción general** 

## **1.1 Análisis de necesidad y objetivos** 

La creciente urbanización, las permanentes tasas de criminalidad y el acceso no autorizado a instalaciones han generado una necesidad de reinventar el tradicional mecanismo de acceso con llaves. Las cerraduras tradicionales, aunque ampliamente utilizadas, presentan limitaciones críticas, como la vulnerabilidad al robo de llaves, su duplicación no autorizada y la falta de registros de acceso. El auge de las tecnologías IoT promueven la implementación de sistemas de acceso más seguros, transparentes e ilimitadamente escalables y adaptables. A través de protocolos de Wi-Fi, una cerradura convencional puede controlarse de forma remota, otorgando flexibilidad en las entregas a domicilio y el permiso de acceso para visitas. En la Figura 1.1 se puede ver un modelo de cerradura inteligente con Wi-Fi y una apliación de celular para controlarla.

La implementación de las tecnologías IoT en el ámbito de las cerraduras y la seguridad presenta los siguientes beneficios:
- Seguridad Avanzada: El método de acceso convencional (llave) es susceptible a robo y plagios, pero con las cerraduras inteligentes se puede utilizar pines, RFID o huellas dactilares o reconocimiento facial, eliminando los riesgos de seguridad de las llaves físicas.
- Acceso remoto: A través de una conexión Wi-Fi, se puede acceder a la instalación desde cualquier lugar.
- Flexibilidad y comodidad de acceso: Se pueden crear claves de acceso de forma dinámica, tanto permanentes como temporales, lo cual es cómodo tanto para el dueño como para el visitante.
- Monitoreo: Las llaves convencionales no permiten identificar quien accede al edificio ni cuando accede. Estos dos problemas se solucionan con los sistemas inteligentes, que notifican y registran los accesos.


 <img src="https://github.com/user-attachments/assets/9d2beedb-58af-48eb-bad7-87d907b22b32" alt="image2" width="50%">
 
**Figura 1.1**: Modelo de cerradura inteligente con Wi-Fi .


En Argentina existen varios productos de características similares a las del proyecto, cada producto diferenciándose de otro con detalles, como por ejemplo huellas digitales, llaves, etc. Sin embargo, parecería que no hay un producto en Argentina que cumpla exactamente la misma función que el proyecto desarrollado.

Como primer competidor del mercado se puede mencionar a Yale, la cual ofrece una amplia gama de cerraduras digitales inteligentes para seguridad del hogar. Los precios de sus productos orbitan desde los 45.000 hasta los 600.000 pesos y sus productos incluyen desde cerraduras con teclado matricial hasta cerraduras con huellas digitales, todas con una aplicación para el celular. El proyecto tiene similitudes en el teclado matricial y en la aplicación, pero se diferencia en la oferta de los sensores magnéticos, del lector RFID y de la comunicación con parlante y micrófono.

Un segundo competidor es Philips, la cual tiene productos con reconocimiento facial y video portero. Esta tecnología es considerablemente superior al igual que sus precios, los cuales rondan el 1.000.000 de pesos. Sin embargo, la empresa no cuenta con lectores RFID ni sensores magnéticos, lo cual diferencia a nuestro proyecto.

En resumen, el mercado es sumamente competitivo y las características y funcionalidades de la cerradura pueden ser modificadas y ampliadas fácilmente, permitiendo desarrollar productos únicos y distintos de la competencia que encuentren un lugar en el mercado.

El presente proyecto se destaca especialmente por incorporar un módulo Wi-Fi que permite configurar los permisos de acceso de los individuos y un parlante y micrófono. Esto lo diferencia de otros sistemas similares en que permite seleccionar de forma remota los permisos de acceso y visualizar los intentos de acceso de forma remota. Adicionalmente, permite monitorear y regsitrar el ingreso y egreso de las visitas a la instalacion a traves de la aplicación. Una última característica especial es la presencia de un sensor magnético, el cual indica cuando se deja abierta la puerta accidentalmente, lo cual previene filtraciones de seguridad.

### **1.2 Módulos e Interfaces de *Smartlock*** 
El sistema *Smartlock* consta dos subsistemas principales: el sistema montado en la cerradura y el sistema de interfaz virtual para el ususario. En la Figura 1.2.1 se muestra el diagrama en bloques del proyecto.

 <img src="https://github.com/user-attachments/assets/54470de0-882c-4329-aed6-d6e38e0a0185" alt="image2" width="80%">

**Figura 1.2.1**: Diagrama en bloques del proyecto *Smartlock*.

En la Fiugra 1.2.1 se observa por un lado el subsistema de la cerradura con los periféricos que se encapsulan juntos como son el motor, el lector RFID y otros, y por el otro lado, el segundo susbsistema se encarga de mostrar los datos de la cerradura y enviar los comandos desde la aplicacion, interconectando los subsistemas por Wi-Fi.

La cerradura cuenta con dos métodos de acceso, mediante tarjetas RFID o mediante un pin de identificación. La tarjeta RFID es un identificador pasivo que al acercarse a un lector RFID, éste induce una corriente sobre ella para retransmitir una respuesta al lector. Estas tarjetas tienen un código de identificación unico que puede ocultarse para prevenir filtraciones de seguridad. La Figura 1.2.2 muestra la tarjeta y un lector RFID.

 <img src="https://github.com/user-attachments/assets/46be1b4e-dd71-4adf-a30a-cbd49e53a733" alt="image2" width="40%">
 
**Figura 1.2.2**: Tarjeta RFID y lector RFID.

Por otro lado, el pin se introduce en un teclado matricial de 4x4 caracteres. El largo del pin esta fijo en 4 dígitos pero es modificable. La Figura 1.2.3 muestra el teclado matricial en el que se introducen los códigos de acceso.

<img src=https://github.com/user-attachments/assets/67fca740-727b-4869-a861-af216d65048b alt="image2" width="40%">
 
**Figura 1.2.3**: Teclado Matricial.

La interfaz virtual se muestra en al Figura 1.2.4. Esta permitido el acceso a la interfáz únicamente con usuario y contraseña por razones de seguridad.

<img src=https://github.com/user-attachments/assets/ec4eb17f-931d-4d5d-add4-48fec88283c6 alt="image2" width="40%">

**Figura 1.2.4**: Interfaz virtual para el usuario.

En las próximas secciones se describirá con mayor detalle los módulos utilizados y sus características.

# **CAPÍTULO 2** 

# **Introducción específica** 

## **2.1 Requisitos**

Habiendo analizado las características principales de la cerradura, se definieron los principales requisitos para que el sistema cumpla con su función de forma correcta y resulte útil para su propósito. Para esto, se realizó una tabla definiendo los principales requisitos a implementar, mostrado en la Tabla 1.2.

| Grupo | ID | Descripción |
| :---- | :---- | :---- |
| Acceso | 1.1 | El sistema permitirá el acceso a la instalación mediante RFID |
|  | 1.2 | El sistema permitirá el acceso a la instalación mediante un teclado matricial |
|  | 1.3 | En caso de acceso permitido, el  sistema abrirá la puerta durante un periodo de tiempo fijado |
| Indicadores | 2.1 | El sistema contará con un indicador luminoso para indicar que la puerta está cerrada |
|  | 2.2 | El sistema contará con un indicador luminoso para indicar que la puerta está abierta |
|  | 2.3 | El sistema contará con un parlante que indique mediante sonido cuando se abre la puerta |
|  | 2.4 | El sistema contará con un parlante que indique mediante sonido cuando se introduce un código incorrecto |
|  | 2.5 | El sistema contará con un parlante que indique mediante sonido cuando se presiona una tecla |
|  | 2.6 | El sistema notificará con el parlante cuando se deja la puerta abierta |
|  | 2.7 | El sistema notificará mediante el titileo de leds cuando la puerta se deja abierta o cuando se introduce un código/RFID erróneo |
|  | 2.8 | El sistema notifica con audio y luz cuando se deja un código sin introducir completamente |
| Motor | 3.1  | El sistema contará con un motor para abrir y cerrar la cerradura |
|  | 3.2 | El sistema solo podrá cerrar la puerta cuando esta se halla contra el marco |
| Comunicación Audio  | 4.1 | El sistema deberá contar con un parlante para transmitir señales de audio |
| Comunicación Wi-Fi | 5.1 | El sistema se comunicará mediante Wi-Fi con la aplicación de celular |
|  | 5.2 | El sistema deberá enviar a la aplicación la información de acceso a la instalación |
|  | 5.3 | El sistema deberá enviar a la aplicación la información de acceso denegado a la instalación |
|  | 5.4 | En caso de lectura RFID, el sistema deberá comunicar la lectura de RFIDs desconocidos, junto con su ID (para poder guardarlo posteriormente) |
|  | 5.4 | El sistema deberá comunicarle a la aplicación el estado de la puerta y el numero de puerta |
| Aplicación | 6.1 | La aplicación tendrá la base de datos de los individuos habilitados con sus IDs |
|  | 6.2 | La aplicación deberá permitirle al usuario configurar los permisos de acceso de los individuos |
|  | 6.3 | La aplicación deberá poder mostrar la información de los accesos e intentos de acceso |
|  | 6.4 | La aplicación deberá poder controlar manualmente la apertura o cerradura de la puerta |
|  | 6.5 | La aplicación deberá permitirle al usuario configurar añadir nuevos individuos a la base de datos mediante codigo o RFID |
|  | 6.6 | La aplicación deberá mostrarle al usuario el estado de la puerta |
| Interruptores/ Botones | 7.1 | El sistema contará con botones para abrir o cerrar la puerta de forma manual |
|  | 7.2 | El sistema contará con un botón de timbre |
| Sensores | 8.1 | El sistema contará con un sensor magnético que detectará cuando la puerta se halla contra el marco |

<p align="center"><em>Tabla 2.1: Requisitos del proyecto.</em></p>


## **2.2 Casos de uso**
En las tablas 2.2 a 2.4 se presentan 3 casos de uso para ejemplificar el uso del sistema.

| Elemento | Definición |
| :---- | :---- |
| Disparador | Se quiere acceder a la instalación con permiso de acceso y con código en el teclado matricial. |
| Precondiciones | El sistema está encendido La puerta está cerrada, con la  cerradura cerrada. La aplicación está conectada al sistema. El indicador de puerta cerrada está encendido  |
| Flujo principal | El individuo introduce su código en el teclado matricial. Por cada tecla que presione recibe una melodía indicando la recepción del carácter introducido. Una vez introducido todo el código, el motor abre la cerradura y se notifica con una melodía el acceso habilitado. Además, con un led se notifica la habilitación del acceso. El módulo Wi-Fi comunica a la aplicación el acceso a la instalación, mostrando el estado de la puerta, la identificación del ingresante, la hora y el número de puerta. El individuo cierra la puerta una vez adentro. |
| Flujos alternativos | a. El individuo introduce erróneamente el código, con lo cual se notifica auditiva y visualmente al usuario y se notifica mediante Wi-Fi que se intentó acceder sin permiso a la instalación.  b. El individuo deja de introducir el código a mitad de camino. Luego de un tiempo preestablecido, el código a medias se descarta y se notifica que el intento fue incorrecto tanto al individuo como al propietario mediante Wi-Fi.. c. El individuo olvida la puerta abierta, entonces se suena una “alarma” y se notifica al propietario mediante Wi-Fi d. Un usuario dentro de la instalación presiona el botón de cerrar la puerta antes de que se introduzca el código. La puerta permanece cerrada. e. El usuario acerca su tarjeta en vez de terminar el código. La puerta se abre y se indica que la puerta está abierta. |

<p align="center"><em>Tabla 2.2: Caso de uso 1: El usuario quiere acceder con código.</em></p>


| Elemento | Definición |
| :---- | :---- |
| Disparador | Se quiere acceder a la instalación con permiso de acceso y con RFID. |
| Precondiciones | El sistema está encendido La puerta está cerrada, con la  cerradura cerrada. La aplicación está conectada al sistema. El indicador de puerta cerrada está encendido  |
| Flujo principal | El individuo acerca su tarjeta al lector. El ID de la tarjeta es correcto, el motor abre la cerradura y se notifica con una melodía el acceso habilitado. Además, con un led se notifica la habilitación del acceso. El módulo Wi-Fi comunica a la aplicación el acceso a la instalación, mostrando el estado de la puerta, la identificación del ingresante, la hora y el número de puerta. El individuo cierra la puerta una vez adentro. |
| Flujos alternativos | a. El individuo usa una tarjeta no habilitada, con lo cual se notifica auditiva y visualmente al usuario y se notifica mediante Wi-Fi que se intentó acceder sin permiso a la instalación.  b. El individuo olvida la puerta abierta, entonces se suena una “alarma” y se notifica al propietario mediante Wi-Fi |

<p align="center"><em>Tabla 2.3: Caso de uso 2: El usuario quiere acceder con RFID.</em></p>


| Elemento | Definición |
| :---- | :---- |
| Disparador | Se quiere añadir una nueva tarjeta RFID |
| Precondiciones | El sistema está encendido La puerta está cerrada, con la  cerradura cerrada. La aplicación está conectada al sistema. El indicador de puerta cerrada está encendido |
| Flujo principal | Se acerca una tarjeta desconocida al lector RFID. El sistema indica que el intento de acceso es incorrecto. El sistema muestra el ID de la tarjeta e indica si se quiere guardar la tarjeta mediante comunicación Wi-Fi con la aplicación. |
| Flujos alternativos | a. Se pierde la conexión Wi-Fi. El sistema no puede continuar con el guardado de la tarjeta. Se indica que se perdió la conexión b. Se apaga el sistema. El sistema no puede continuar con el guardado de la tarjeta.  |

<p align="center"><em>Tabla 2.4: Caso de uso 3: El usuario quiere guardar una tarjeta RFID.</em></p>

## **2.3 Descripción de los Módulos del sistema**
## **2.3.1 Alimentación**
El sistema se alimenta con 3 sistemas de alimentación que pueden reemplazarse posteriormente por un solo sistema de alimentación. Se menciona que se tiene muchos sistemas de alimentación debido a la etapa de desarrollo inicial. La primer alimentación se encarga de alimentar la placa Nucleo-F429ZI y es la alimentación proveniente de la computadora portatil mediante USB, la cual permite comunicación UART. 

El segundo sistema de alimentación es una fuente transformadora marca HAMA modelo 00201644 con 5V de salida y hasta 1.2 A de corriente. Esta alimentación es para el esp32, encargado de la comunicación Wi-Fi. Una tercera fuente transformadora 220 VAC a 12 VDC con 2000 mA modelo FJ-SW102 se utiliza para alimentar una fuente modelo Mb-102 capaz de entregar a la salida 5 V o 3,3 V seleccionables, y hasta 700 mA. A través del Mb-102 se alimenta a todos los otros periféricos como el lector RFID, el motor y otros. Se muestra la fuente Mb-102 en la Figura

<img src=https://github.com/user-attachments/assets/036b1989-56ed-49ab-b803-4aeecc3f1c3f alt="image2" width="30%">

**Figura 2.3.1**: Fuente de alimentación Mb-102.


## **2.3.2 Microcontrolador** 
Como controlador principal del sistema se utiliza la placa NUCLEO-F429ZI \[4\]. La elección de esta placa recayó exclusivamente en la disponibilidad, teniendo además como requerimiento la cantidad de memoria, pines y periféricos de la placa. La placa se programó en C++ a través de la plataforma *Keil Studio Cloud* y se muestra en la Figura 2.3.2 .

<img src=https://github.com/user-attachments/assets/14a92d96-6aab-463f-b4e8-22d7dc2e253b alt="image2" width="40%">

**Figura 2.3.2**: Placa NUCLEO-F429ZI.

## **2.3.3 Motor** 
El acceso a la instalación se habilita con el accionamiento de un servomotor. El servomotor utilizado en este proyecto es el microservo de 9g de TOWERPRO modelo SG90, mostrado en la Figura 2.3.3. Para el control de este servomotor se usa PWM, identificando mediante experimentos las frecuencias correspondientes a los ángulos máximos de operación del servomotor.

<img src=https://github.com/user-attachments/assets/ee3fd32b-8d09-45cc-870d-dcbe58f827f3 alt="image2" width="30%">

**Figura 2.3.3**: Servomotor SG90.

Este servomotor se reemplazaría para puertas grandes por otro motor con mas potencia para poder mover la perilla más pesada.

## **2.3.4 Lector RFID** 
Para este proyecto se buscó un lector RFID de bajo costo y compactabilidad, eligiendo el modelo RC522 mostrado en la Figura 2.3.4.Este módulo cuenta con el circuito integrado MFRC522, que es un lector inalámbrico que trabaja a 13,56 MHz con la cual se leen las tarjetas de acceso.

<img src=https://github.com/user-attachments/assets/83078155-b70e-44d6-9eb2-d634a3e11e06  alt="image2" width="30%">

**Figura 2.3.4**: Lector RFID RC522.

Para comunicarse con este módulo se utiliza el protocolo SPI, y para la programación de la placa NUCLEO se utilizó una librería de software escrita por Martin Olejar y publicado en la página oficial de MBED \[2\].

## **2.3.5 Teclado Matricial** 
El teclado matricial elegido es un Teclado Membrana Matricial 4x4 como se muestra en la Figura 2.3.5. La selección de este teclado se hizo en base a su disponibilidad en el mercado local y su bajo costo.

<img src=https://github.com/user-attachments/assets/6c647715-ff55-45e0-8941-e491e28d6c11  alt="image2" width="30%">

**Figura 2.3.5**: Teclado matricial.

Para comunicarse con este módulo se utiliza GPIO, el código del teclado fue obtenido y adaptado del libro *A Beginner’s Guide to Designing Embedded System Applications on Arm Cortex-M Microcontrollers* por Ariel Lutenberg, Pablo Gomez y Eric Pernia \[1\].

## **2.3.6 Sensor Magnético** 
El sensor magnetico utilizado consta de dos componentes: un imán y una llave magnetica (se muestra ambos en la Figura 2.3.6). La llave magnética son hilos finos que al acercar un imán hace un contacto.

<img src=https://github.com/user-attachments/assets/41ace232-ff76-4d27-9861-f1221cc78199  alt="image2" width="30%">

**Figura 2.3.6**: Sensor magnético.

## **2.3.7 Parlante** 
Para la comunicación por audio se usa un parlante de 8 ohms y 2 Watts como se muestra en la Figura 2.3.7 .

<img src= https://github.com/user-attachments/assets/59824229-6f30-46c4-aadc-45b2e2f2f51d alt="image2" width="30%">

**Figura 2.3.7**: Parlante 8 Ohms 2 Watts.

## **2.3.8 Comunicación Wi-Fi** 
Para la comunicación por Wi-Fi se usa el ESP32-DEVKIT por su módulo Wi-Fi incorporado de 2.4 GHz. El módulo se muestra en la Figura 2.3.8.
<img src= https://github.com/user-attachments/assets/ac1e1a22-ae50-4a56-801a-45ebbec1cbdc alt="image2" width="30%">


**Figura 2.3.8**: ESP32-DEVKIT.

El código utilizado en el ESP32 para la comunicación Wi-Fi se basó en la implementación hecha por Augusto Villacampa \[3\].

# **CAPÍTULO 3** 

# **Diseño e implementación** 

## **3.1 Diseño del Hardware** 

Para el diseño del prototipo se construyó una maqueta de madera haciendo los agujeros correspondientes para pasar los cables y conectándolos al protoboard de forma prolija y ordenada. Las Figuras 3.1a y 3.1b muestran el montaje final del prototipo para una maqueta de una puerta.

<img src=https://github.com/user-attachments/assets/6c80e61d-8d8d-4d6c-a643-738eeee609c2 alt="image2" width="60%">

**Figura 3.1a**: Montaje final del prototipo, parte delantera.

<img src=https://github.com/user-attachments/assets/4eee60df-88e1-4a73-8e30-01146b2f9be2 alt="image2" width="60%">

**Figura 3.1b**: Montaje final del prototipo, parte trasera.

Los esquemáticos se muestran en las secciones posteriores.

## **3.1.1 Diseño de la alimentación** 
En la figura 3.1.1 se muestra el circuito esquemático del hardware de alimentación con sus debidos capacitores a la salida para estabilizar la salida ante picos de corriente.

<img src=https://github.com/user-attachments/assets/a7625bac-d907-4369-8133-8f78a38716eb alt="image2" width="40%">

**Figura 3.1.1**: Alimentación del sistema.

## **3.1.2 Diseño de los indicadores e interruptores** 
La Figura 3.1.2 muestra el esquemático de los indicadores e interruptores. Para todos los indicadores se utiliza un transistor que activa la corriente en el led. La corriente máxima estimada es 4 mA para cada led. EL transistor que se usa es siempre el mismo: transistor NPN BC545. La corriente de salida necesitada para encender el LED es muy baja. Los botones y sensores magnéticos se conectan a 3,3 V y se configuran las entradas de la placa NUCLEO-F429ZI en modo Pull Down. 

<img src=https://github.com/user-attachments/assets/37805a98-bcfc-4c11-983b-6d91c570b76e  alt="image2" width="40%">

**Figura 3.1.2**: Circuito esquemático de los indicadores e interruptores del sistema.

## **3.1.3 Diseño del lector RFID** 
En la Figura 3.1.2 se muestra el circuito esquemático del hardware del módulo RFID. Se observa la utilización de los pines de MISO, MOSI, SCK y CS para la comunicación SPI, además del conexionado del pin de Reset para la correcta configuración del módulo. 

<img src=https://github.com/user-attachments/assets/a2770d18-31b0-4e5b-81f5-ed92d74112ae  alt="image2" width="40%">

**Figura 3.1.3**:  Circuito esquemático del lector RFID del sistema.

## **3.1.4 Diseño del circuito para controlar el parlante**

Para el parlante se diseña un amplificador de audio clase A con los componentes que se tienen (Figura 3.1.4). El capacitor C3 y la resistencia R11 actuan como filtro pasabajos para hacer la señal PWM mas suave y más parecida a una senoidal. El capacitor C4 actua junto con el parlante como un filtro pasa-altos e interrupe el flujo de corriente continua, dando una mejor señal de salida en el parlante.

<img src=https://github.com/user-attachments/assets/cf5e304f-a75d-4c79-8a84-3961b2433360  alt="image2" width="40%">

**Figura 3.1.4**:  Circuito esquemático del parlante del sistema.

## **3.1.5 Diseño del circuito para controlar el motor** 
El esquemático del motor se muestra en la Figura 3.1.5. Se tiene una resistencia de 1kohms para restringir la corriente que puede llegar a entregar el servomotor al pin del PWM, cuando se selecciona un angulo no disponible o se traba el servomotor. El fin de la resistencia d 1kohm es proteger la placa F429ZI. La resistencia de 10 ohms en la alimentación cumple la función de limitar corrientes muy altas y picos de corriente.

<img src=https://github.com/user-attachments/assets/4ce16d1c-2db0-4873-8110-544dd7427657  alt="image2" width="40%">

**Figura 3.1.5**:  Circuito esquemático del motor del sistema.

## **3.1.6 Diseño del circuito para controlar el teclado matricial** 
La Figura 3.1.6 muestra la conexión del teclado matricial. Se observa la conexión directa de los 8 pines a la placa NUCLEO-F429ZI, de los cuales 4 son configurados en la placa NUCLEO-F429ZI como entrada en modo Pull Down (C1, C2, C3, C4) y los otros 4 como salida. La idea es barrer el teclado matricial con los outputs (filas), detectar activaciones en los inputs (columnas) y así leer el caracter introducido.

<img src=https://github.com/user-attachments/assets/ffb93399-ce71-42b2-83dc-5e8bec9a9624  alt="image2" width="40%">

**Figura 3.1.6**:  Circuito esquemático del teclado matricial del sistema.

## **3.1.7 Diseño del circuito para controlar el Módulo Wi-Fi** 
En la Figura 3.1.7 se muestra el circuito esquemático del hardware para el ESP32. Notar que únicamente se hace uso de los pines UART2 (GPIO16 y GPIO17) del módulo para comunicarse con la placa NUCLEO-F429ZI.

<img src=https://github.com/user-attachments/assets/66cd0279-c347-494b-bb1c-3e01c6b74f6f alt="image2" width="40%">

**Figura 3.1.7**:  Circuito esquemático del Módulo Wi-Fi.

## **3.1.8 Diseño del hardware con la placa NUCLEO-F429ZI** 
En la Figura 3.1.9 se muestra el circuito esquemático del conexionado de la placa NUCLEO-F429ZI. Para esta implementación se buscó utilizar únicamente pines a los que se pudiera acceder a través de alguno de los zócalos, de modo de no necesitar realizar soldaduras. Es importante mencionar la conexión a tierra de todos los módulos (tierra compartida) para que sea posible la comunicación entre módulos.

<img src= https://github.com/user-attachments/assets/2ddeee79-c87c-4892-9026-08ff3395c338 alt="image2" width="40%">

**Figura 3.1.9**:  Circuito esquemático del NUCLEO-F429ZI.

## **3.2 Firmware del *Smartlock*** 

La implementación del trabajo se realizó en lenguaje C++ utilizando Mbed y bajo el paradigma de la programación orientada a objetos. El firmware está conformado por un archivo main.cpp que inicializa al sistema con *system_init()* y luego convoca repetidamente a la función *system_update()*, las librerías arm_book_lib.h, global_defines.h, el archivo mbed_app.json y una carpeta con modulos que contienen un archivo .cpp y otro .h. Las librerias mencionadas y el archivo json contienen, respectivamente, la función main del programa, numerosas definiciones y algunas configuraciones. La Figura 3.2.1 muestra un diagrama de archivos del sistema.

<img src= https://github.com/user-attachments/assets/95144be5-f9d5-4684-866b-590c30889119 alt="image2" width="40%">

**Figura 3.2.1**: Diagrama de archivos del *Smartlock*.

Como se mencionaba, el archivo main.cpp solamente se encarga de inicializar el sistema y actualizar el estado, como se muestra en la Figura . En el archivo  *global_defines.h* se encuentran muchas definiciones constantes de pins y valores necesarios para el funcionamiento del sistema. Solo fueron definidos los pines para la placa que se usa en este trabajo, pero se tomó esta decisión para simplificar el trabajo si en un futuro se decide utilizar otra placa. La Figura 3.2.1 muestra el archivo main.cpp.

<img src=https://github.com/user-attachments/assets/96ef8f41-0467-491d-a9cd-02f5e88e5a65 alt="image2" width="30%">

**Figura 3.2.2**: Archivo Main.cpp.

## **3.2.1 Módulo Access Keys** 
Este módulo se encarga de comparar los intentos de acceso con las llaves de acceso, perimitiendo acceder cuando un intento de acceso coincide con una llave de acceso (RFID o Keypad) y denegando el acceso cuando el intento de acceso no coincide con ninguna llave. La función responsable de esto se llama *access_attempt_update()* y devuelve 3 estados: ACCESS_GRANTED cuando se concede el acceso, ACCESS_DENIED cuando no se lo concede y NO_ACCESS_ATTEMPT cuando no hay ningun intento de acceso. La Figura 3.2.3 muestra la función *access_attempt_update()*.

<img src=https://github.com/user-attachments/assets/93a97b01-2636-4409-b7d9-b468e6e90b7b alt="image2" width="60%">

**Figura 3.2.2**: Función *access_attempt_update()* del módulo Access Keys.

Otra funcionalidad de este módulo es la de gestionar la base de datos interna de la placa NUCLEO-F429ZI, permitiendo recibir códigos nuevos ingresados por el dueño y guardandolos para permitir el acceso o recibiendo ordenes de borrar códigos viejos. Esta funcionalidad la realiza con la función *ProcessSecurityMessage()*, haciendo uso de memoria dinámica a través de vectores para guardar y eliminar datos estructurados.

## **3.2.2 Módulo Keypad** 
Para este módulo el código y la FSM se incorpora del libro *A Beginner’s Guide to Designing Embedded System Applications on Arm Cortex-M Microcontrollers* escrito por Ariel Lutenberg, Pablo Gomez y Eric Pernia y se lo modifica para que sea un objeto. La máquina de estados finita del módulo se muestra en la Figura 3.2.4.

<img src=https://github.com/user-attachments/assets/23a69610-ce9a-432d-a751-463a9237f7d6 alt="image2" width="60%">

**Figura 3.2.4**: Máquina de estados del módulo Keypad [1].

Se tiene un tiempo de debounce ajustable para prevenir lecturas falsas.

## **3.2.3 Módulo Motors** 
Crea objetos llamados Motor que traducen el angulo requerido a una señal PWM que controla la posición del motor. Los valores máximos y mínimos de duty cycle se encontraron de forma experimental en la facultad de ingeniería. El código es relativamente simple y se muestra en la Figura 3.2.5.

<img src=https://github.com/user-attachments/assets/98299e92-b497-4de2-9530-17c7f3baa440 alt="image2" width="60%">

**Figura 3.2.5**: Archivo Motors.cpp.

## **3.2.4 Módulo MQTT** 
En este módulo se maneja toda la comunicación del protocolo MQTT, que en realidad es una comunicación por UART con el módulo de hardware ESP32. Esta clase cuenta con un objeto de la clase BufferedSerial para manejar la comunicación con la placa ESP32 y un objeto DigitalOut para indicar con un led cuando se recibe o envía un mensaje. Entre los métodos públicos con los que cuenta esta clase están: *keepAlive()* que manda un mensaje periodicamente para indicar que la placa NUCLEO-F429ZI está activa, *SendStatus()* que recibe el estado de la puerta y lo envía, *publish()* y *receive()* que se encargan de publicar y recibir mensajes MQTT respectivamente y *ShowRFID()* que muestra el RFID una vez se lee. Además, posee una estructura de datos *MQTTMessage* donde describe si se recibió un mensaje, el tópico y el mensaje. También cuanta con métodos para enviar mensajes de logger a la aplicación cuando sea necesario. Para más detalle sobre los métodos, se invia al lector a visitar el módulo MQTT en este repositorio. Los tópicos a suscribirse estan predefinidos en el ESP32.

## **3.2.5 Módulo RFID** 
El módulo contiene el código de la libreria creada por un tercero, como se mencionó previamente, que define la clase MFRC522. Adicionalmente, contiene las funciones para leer el RFID e inicializarlo. Las funciones son simples y hacen uso de las funciones de la librería que estan muy bien modularizadas. Se menciona que la librería de MFRC522 tuvo que incorporarse en los archivos, con lo cual se puede ver el archivo MFRC522.cpp y MFRC522.h correspondientes a la librería del autor indicado. Las funciones más importantes del Módulo RFID son *RFID_read()* que devuelve el ID leido (si se lee) o el puntero a null (si no se lee) y la función *RFID_reader_init()* que inicializa el módulo. La Figura 3.2.6 muestra parte de la función *RFID_read()*.

<img src=https://github.com/user-attachments/assets/c2434444-42d2-4b6f-ab14-b281a0d248ff alt="image2" width="60%">

**Figura 3.2.6**: Función *RFID_read()* del archivo RFID_reader.cpp.

No se introdujo una máquina de estados para el lector RFID porque no se consideró necesario. Se introduce además una funcionalidad para restringir el tiempo entre lecturas del lector RFID, con este tiempo entre lecturas definido en el archivo *global_defines.h*.

## **3.2.6 Módulo Speaker** 
En este módulo se define una clase *Speaker* que se encarga de tocar las notas musicales para los accesos correctos, accesos incorrectos, alarma de puerta olvidada, introduccion de botones y timbre. Para cada uno de esos tonos hay un método especial que dice que melodía se tocará (Figura 3.2.7). El método *play_melody()* se encarga de tomar una secuencia de periodos de notas musicales (C5, E6, etc), una secuencia de duraciones para cada nota y producir una señal PWM para cada nota con la duración indicada. A la salida, se obtiene sobre el parlante una señal que intenta parecerse a una señal con la frecuencia indicada (hardware).

<img src=https://github.com/user-attachments/assets/68ce7c65-a95a-4fa2-8776-9a0be4f04224 alt="image2" width="60%">

**Figura 3.2.7**: Métodos de la clase *Speaker* en el archivo *Speaker.h*.

Adicionalmente, se tienen los métodos update y alarm_update que actualizan el cambio de notas musicales. La variable ringbell es una comodidad para tocar una única nota musical cuando se toca el timbre.

## **3.2.7 Módulo System** 
El módulo System se encarga de orquestrar el funcionamiento de varios módulos. Se definen dos funciones públicas, system_init y system_update, que se llaman desde el archivo main.cpp, funciones para cada estado de la puerta con el sufijo _update() y otras funciones miscelaneas que no se contemplan en los módulos mencionados pero se requieren para controlar el funcionamiento del sistema. En la Figura 3.2.8 se peuden ver los objetos creados así como los parámetros necesarios en cada caso, y que fueron definidos en el archivo global_defines.h. 

<img src=https://github.com/user-attachments/assets/929cfece-d981-42e6-ad05-cb604bd3ce30 alt="image2" width="60%">

**Figura 3.2.8**: Objetos creados de otros módulos en el archivo *System.cpp*.

La función de actualización principal *system_update()* se muestra en la Figura 3.2.9.

<img src=https://github.com/user-attachments/assets/403e5de7-70c1-4707-b60e-a3309f802c04 alt="image2" width="60%">

**Figura 3.2.9**: Función *system_update()* del  archivo *System.cpp*.

Notar que cada estado de la puerta tiene su propia funcion de actualización. Los estados de la puerta on 4: DOOR_CLOSED, DOOR_OPENING, DOOR_OPEN y DOOR_CLOSING. La función *process_mqtt()* se encarga de procesar los mensajes mqtt y generar los cambios necesarios en las variables del sistema para actuar los controladores o para modificar la base de datos. El control de *force_block_door* es un comando que se recibe por MQTT para mantener la puerta cerrada.
## **3.2.8 Módulo UART Communications** 
Por último, el módulo de comunicaciones UART permite notificar mediante UART el estado de la puerta y los accesos. Este módulo es unicamente útil si se decide leer los estados del sistema de la placa NUCLEO-F429ZI a través de UART. Además, permite recibir comandos para guardar un único ID de RFID de forma manual y sin contar con la apliación. La Figura 3.2.10 meustra las funciones de este módulo

<img src=https://github.com/user-attachments/assets/48a3698f-224e-4990-a563-29d1defdeaee alt="image2" width="60%">

**Figura 3.2.10**: Funciones utilizadas para la comunicación UART en el archivo *UART_comm.h*.

## **3.3 Firmware del *ESP32 DEVKIT V1*** 
El desarrollo de este firmware se realizó con el entorno de desarrollo de Arduino, que permite programar la placa ESP32 directamente mediante la definición de la placa ESP32 como la placa objetivo. En este caso todo el firmware se implementó en un único archivo, cuyo desarrollo se basó e la implementación de MQTT hecha por Augusto Villacampa \[3\] . Al comienzo del programa, luego de incluir las librerías de código necesarias, lo primero que hace es definir objetos y variables necesarias. Esto incluye los objetos que manejan la conexión Wi-Fi y MQTT, y variables como la red Wi-Fi y la contraseña, la IP del servidor MQTT y el puerto usado, o los pines que se usarán. Esto se muestra en la figura 3.3.1. 

<img src=https://github.com/user-attachments/assets/8e5fd336-1061-415f-a30a-6638186b97df alt="image2" width="40%">

**Figura 3.3.1**: Objetos declarados en la placa *ESP32*.

De todas las funciones definidas en el archivo del ESP32, la más importante es la función *callback()* y la función *reconnect()*. La función callback se encarga de reenviar los mensajes recibidos por Wi-Fi con MQTT a la placa NUCLEO-F429ZI a través de UART. La Figura 3.3.2 muestra el código de la función callback. También se envían a la computadora, en caso de estar conectada por computadora. La función reconnect se encarga de conectar al ESP32 al broker de MQTT, el cual es un broker Mosquitto (broker sin necesidad de licencia), y posteriormente se suscribe a los tópicos necesarios para el funcionamiento del sistema.

<img src=https://github.com/user-attachments/assets/af775709-4cfe-42c9-854c-86305e8d7294 alt="image2" width="40%">

**Figura 3.3.2**: Función *callback()* en el firmware del ESP32.

Finalmente, todo mensaje MQTT que se reciba de la placa NUCLEO-F429ZI a través de UART se reenvía por Wi-Fi con empaquetamiento MQTT como se muestra en la Figura 3.3.3.

<img src=https://github.com/user-attachments/assets/f230aaeb-0cb2-47a8-bf4e-a1fd049410c5 alt="image2" width="40%">

**Figura 3.3.2**: Main Loop del firmware del ESP32.

## **3.4 MOSQUITTO broker** 

Mosquitto es un broker de mensajes que implementa el protocolo MQTT (Message Queuing Telemetry Transport), diseñado para facilitar la comunicación ligera y eficiente entre dispositivos en redes IoT (logo en la Figura 3.4.1). MQTT utiliza un modelo de publicación-suscripción, donde el broker actúa como intermediario para distribuir mensajes entre clientes.

<img src=https://github.com/user-attachments/assets/dcd16b06-05dd-4b17-9ab9-b6bb010c72a7 alt="image2" width="40%">

**Figura 3.4.1**: Logo Mosquitto.

En este proyecto, se configuró un broker Mosquitto en el puerto 1883, que es el puerto estándar para conexiones MQTT sin encriptación. En términos de seguridad, Mosquitto soporta encriptación mediante TLS (Transport Layer Security), lo que asegura que la transmisión de datos esté protegida contra accesos no autorizados o interceptaciones. Para este prototipo no se configuró pero se sentaron las bases para configurarlo.

## **3.5 Diseño de la aplicación y manejo de paquetes** 
Para el diseño de la interfaz virtual y el manejo de paquetes se utilizó Node-RED (logo en la Figura 3.5.1). Node-RED es una herramienta de desarrollo basada en flujo diseñada para conectar dispositivos, servicios y API de manera sencilla. Está construida sobre Node.js y proporciona una interfaz visual donde los usuarios pueden arrastrar y soltar nodos (representando funcionalidades o dispositivos) para crear flujos de trabajo que ejecuten tareas específicas. Cada nodo tiene un propósito particular, como procesar datos, interactuar con servicios externos, o controlar hardware. Los flujos se configuran gráficamente y pueden incluir lógica, transformaciones de datos o integraciones con protocolos como MQTT, HTTP, WebSocket y más.

El uso de Node-RED facilita el escalamiento de los proyectos y la cómoda adaptabilidad para incporporar nuevos módulos. Es por esto que Node-RED es ampliamente utilizado en proyectos de IoT (Internet de las Cosas) para integrar sensores, dispositivos inteligentes y sistemas en tiempo real, permitiendo una gestión eficiente y personalizable sin requerir habilidades avanzadas de programación. 

<img src=https://github.com/user-attachments/assets/d10d6ddb-b495-4138-8f4e-70ae2bf64477 alt="image2" width="20%">

**Figura 3.5.1**: Logo Node-RED

El diagrama de Node-RED se visualiza en la Figura 3.5.2. Los nodos violetas son conexiones al broker para publicar o para suscribirse a tópicos. Los mensajes se procesan en los nodos naranajas que son funciones programables para el manejo del paquete en lenguaje JavaScript. El nodo template permite crear la interfaz con HTML, permitiendo utilizar paquetes como entrada y generando paquetes de salida.

<img src=https://github.com/user-attachments/assets/c9f1ff49-6075-4d71-9456-07642252de78 alt="image2" width="60%">

**Figura 3.5.2**: Diagrama del proyecto en Node-RED.

Se tienen 5 tópicos MQTT principales:
- Alive: Cada un intervalo T de tiempo, la placa NUCLEO-F429ZI manda un mensaje *alive* al tópico. Si en el tiempo T+1 (*timestap*) no se detecta el mensaje *alive*, entonces el estado es DESCONECTADO. Caso contrario, es CONECTADO.
- Status: La placa NUCLEO-F429ZI envía a este tópico el estado de la puerta.
- Control: A través de la aplicación, el usuario puede comandar que se abra o cierre la puerta. Estos comandos se publican en el tópico control que luego la placa NUCLEO-F429ZI recibe.
- Logger: Unicamente para que publique la placa NUCLEO,  este tópico tiene todos los mensajes notificando cambios en el estado de la puerta con la hora y día.
- Security: Uno de los tópicos más complejos, se encarga de recibir los IDs de nuevas tarjetas RFID leidas por la placa y luego, si el usuario decide agregar un permiso de acceso con el ID de la tarjeta y un pin, envía la solicitud de agregar dicho usuario. También puede comandar que se borren usuarios.

Por medio de los nodos template, se creó la interfaz de usuario mostrada en la Figura 3.5.3.
<img src=https://github.com/user-attachments/assets/ec4eb17f-931d-4d5d-add4-48fec88283c6 alt="image2" width="60%">

**Figura 3.5.3**: Interfáz para el usuario.

La interfáz se accede mediante la introducción del url con el IP del servidor, el puerto dodne corre node red y /ui. Por ejemplo: 111.111.111.111:1880/ui . Se ve que el Smartlock 1 es el único dispositivo conectado y que se contempla la posibilidad de agregar otro Smartlock. Se tienen las acciones de control para forzar el bloqueo y para abrir la puerta manualmente. Además, se pueden agregar nuevos usuarios y eliminarlos con los botones de eliminar. Cuando se lee un RFID no conocido, un mensaje se muestra en la pantalla indicando la lectura y luego se agrega automáticamente en el campo de RFID. El logger muestra todos los datos de las dos cerraduras y guarda la información en un .txt como se muestra en la Figura 3.5.4.

<img src=https://github.com/user-attachments/assets/4ca7095a-f9cf-47e6-bd08-57f3b6e2879b alt="image2" width="40%">

**Figura 3.5.4**: Logger.txt.

Finalmente, se incorporaron mecanismos de seguridad para que solamente las personas con permiso de acceso a la aplicación puedan conectarse para visualizar la aplicación, como se muestra en la Figura 3.5.5. 

<img src=https://github.com/user-attachments/assets/67972201-607d-4955-a4c4-0dfdf35daf87 alt="image2" width="40%">

**Figura 3.5.5**: Acceso a la interfaz.

# **CAPÍTULO 4** 

# **Ensayos y resultados** 

## **4.1 Pruebas funcionales de funcionamiento**  

Se separó el desarrollo de este proyecto en cuatro etapas, para las cuales se realizaron pruebas de funcionamiento específicas de cada etapa. Los módulos incorporados en cada etapa y las pruebas de funcionamiento pueden leerse en los siguientes archivos:
- README_TP1.md : Lector RFID, indicadores de estado de la puerta y button de cerrado.
- README_TP2.md : Sensor Magnético, Teclado Matricial.
- README_TP3.md : Parlante, servomotor.
- README_TP_FINAL.md : Comunicación Wi-Fi, aplicación.

Cada etapa tiene un video con una prueba funcional de los módulos agregados. Adicionalmente, el TP_FINAL incluye un video con el funcionamiento integral de todos los módulos del proyecto \[5\] . Se observa en el video que la prueba resultó satisfactoria, por lo que se da por finalizado el desarrollo del trabajo. Los esquemáticos correspondientes para las pruebas funcionales se encuentran en los siguientes archivos:
- TP1_SE_esquematico.pdf
- TP2_SE_esquematico.pdf
- TP3_SE_esquematico.pdf
- TP_FINAL_SE_esqumatico.pdf

## **4.2 Cumplimiento de requisitos**  

Una vez finalizado el trabajo, se realizó una tabla con los requisitos iniciales, agregando el estado de los mismos. Esto se observa en la tabla 4.2.
| Grupo | ID | Descripción | Estado |
| :---- | :---- | :---- | :---- |
| Acceso | 1.1 | El sistema permitirá el acceso a la instalación mediante RFID | COMPLETADO |
|  | 1.2 | El sistema permitirá el acceso a la instalación mediante un teclado matricial | COMPLETADO |
|  | 1.3 | En caso de acceso permitido, el sistema abrirá la puerta durante un periodo de tiempo fijado | COMPLETADO |
| Indicadores | 2.1 | El sistema contará con un indicador luminoso para indicar que la puerta está cerrada | COMPLETADO |
|  | 2.2 | El sistema contará con un indicador luminoso para indicar que la puerta está abierta | COMPLETADO |
|  | 2.3 | El sistema contará con un parlante que indique mediante sonido cuando se abre la puerta | COMPLETADO |
|  | 2.4 | El sistema contará con un parlante que indique mediante sonido cuando se introduce un código incorrecto | COMPLETADO |
|  | 2.5 | El sistema contará con un parlante que indique mediante sonido cuando se presiona una tecla | COMPLETADO |
|  | 2.6 | El sistema notificará con el parlante cuando se deja la puerta abierta | COMPLETADO |
|  | 2.7 | El sistema notificará mediante el titileo de leds cuando la puerta se deja abierta o cuando se introduce un código/RFID erróneo | COMPLETADO |
|  | 2.8 | El sistema notifica con audio y luz cuando se deja un código sin introducir completamente | COMPLETADO |
| Motor | 3.1 | El sistema contará con un motor para abrir y cerrar la cerradura | COMPLETADO |
|  | 3.2 | El sistema solo podrá cerrar la puerta cuando esta se halla contra el marco | COMPLETADO |
| Comunicación Audio | 4.1 | El sistema deberá contar con un parlante para transmitir señales de audio | COMPLETADO |
|  | 4.2 | El sistema deberá contar con un micrófono para recibir señales de audio | INCOMPLETO |
| Comunicación Wi-Fi | 5.1 | El sistema se comunicará mediante Wi-Fi con la aplicación de celular | COMPLETADO |
|  | 5.2 | El sistema deberá enviar a la aplicación la información de acceso a la instalación | COMPLETADO |
|  | 5.3 | El sistema deberá enviar a la aplicación la información de acceso denegado a la instalación | COMPLETADO |
|  | 5.4 | En caso de lectura RFID, el sistema deberá comunicar la lectura de RFIDs desconocidos, junto con su ID (para poder guardarlo posteriormente) | COMPLETADO |
|  | 5.4 | El sistema deberá comunicarle a la aplicación el estado de la puerta y el número de puerta | COMPLETADO |
|  | 5.5 | El sistema deberá poder recibir señales de audio de la aplicación | INCOMPLETO |
|  | 5.6 | El sistema deberá poder transmitir señales de audio a la aplicación | INCOMPLETO |
| Aplicación | 6.1 | La aplicación tendrá la base de datos de los individuos habilitados con sus IDs | COMPLETADO |
|  | 6.2 | La aplicación deberá permitirle al usuario configurar los permisos de acceso de los individuos | COMPLETADO |
|  | 6.3 | La aplicación deberá poder mostrar la información de los accesos e intentos de acceso | COMPLETADO |
|  | 6.4 | La aplicación deberá poder controlar manualmente la apertura o cerradura de la puerta | COMPLETADO |
|  | 6.5 | La aplicación deberá permitirle al usuario configurar añadir nuevos individuos a la base de datos mediante código o RFID | COMPLETADO |
|  | 6.6 | La aplicación deberá mostrarle al usuario el estado de la puerta | COMPLETADO |
| Interruptores/ Botones | 7.1 | El sistema contará con botones para abrir o cerrar la puerta de forma manual | COMPLETADO |
|  | 7.2 | El sistema contará con un botón de timbre | COMPLETADO |
| Sensores | 8.1 | El sistema contará con un sensor magnético que detectará cuando la puerta se halla contra el marco | COMPLETADO |

**Tabla 4.2**: Requisitos del proyecto y estados de cumplimiento.

Se observa que la gran mayoría de los requisitos se cumplieron para este proyecto. Solamente resta agregar el micrófono y procesar el audio por la aplicación.

## **4.3 Comparación con otros sistemas similares**   

Como se mencionaba en el incio del documento, el mercado actual tiene productos desarrollados con características muy similares a las de este proyecto. En Argentina no esta muy normalizado el uso de estas cerraduras y es por esto que el uso de un lector RFID y un sensor magnético puede diferenciar a este proyecto del resto. Sin embargo, se reitera que el mercado es grande y hay muchas variantes de cerraduras inteligentes, imposibilitando una comparación justa entre nuestro producto y otros.

## **4.4 Documentación del desarrollo realizado**

La tabla 4.4 muestra la documentación del desarrollo de este proyecto.
| Nombre                 | Fecha de Finalización     | Referencia         |
|------------------------|---------------------------|--------------------|
| Trabajo Práctico 1    | 11 de Octubre del 2024    | README_TP1.md      |
| Trabajo Práctico 2    | 17 de Octubre del 2024    | README_TP2.md      |
| Trabajo Práctico 3    | 17 de Noviembre del 2024  | README_TP3.md      |
| Requisitos y casos   | 18 de Noviembre del 2024  | Readme_requisitos_y_casos.md      |
| Informe de Avance   | 29 de Noviembre del 2024  | Informe de Avance del Trabajo Final.md      |
| Trabajo Práctico Final | 9 de Diciembre del 2024   | README_TP_FINAL.md |
| Memoria del trabajo | 13 de Diciembre del 2024   | Memoria del trabajo final.md |

**Tabla 4.4**: Desarrollo del proyecto.

# **CAPÍTULO 5** 

# **Conclusiones** 

## **5.1 Resultados obtenidos**
El trabajo aquí presentado logró implementar casi todas las funcionalidades deseadas en tiempo y forma, incorporando diseños de robustez y futura adaptabilidad para el uso de microcontroladores mas chicos. La incorporación de protocolos de comuniación IoT como MQTT preparan al sistema para utilizarlo en conjunto con otros dispositivos, sin tener que rediseñar la estrategia de comunicación.

Los resultados clave de este proyecto pueden resumirse en los siguientes puntos:
- Uso de PWM para emitir señales de audio en el parlante y para controlar el motor.
- Implementación de máquinas de estado para el funcionamiento de los módulos y para la interacción entre módulos.
- Desarrollo de técnicas de IoT para la comunicación por Wi-Fi del proyecto.
- Manejo de flujos de paquetes y desarrollo de una página web para visualizar los parámetros críticos del proyecto e interactuar con el sistema.
- Desarrollo de un sistema escalable y adaptable para futuros proyectos.

 Se concluye que al terminar este trabajo se cuenta con un nuevo y amplio abanico de conocimientos para el desarrollo de futuros proyectos de electrónica. Desde lo práctico a la teórico, el proyecto da un panorama de todos los aspectos críticos de un proyecto y enseña a transformar una idea en un producto.

 
## **5.2 Próximos pasos** 

Se puede extender el trabajo aquí realizado con las siguientes implementaciones adicionales:
1) Implementación del micrófono y transmisión de señales de audio a la aplicación.
2) Implementación de alimentación con baterías.
3) Reducción del tamaño de la placa, reemplazando la placa actual por una más chica que contenga todas las funcionalidades necesarias para el funcionamiento actual.
4) Implementación de una base de datos que almacene los datos de acceso y los cargue automáticamente a la placa NUCLEO-F429ZI y a la página web.
5) Incorporación de una cámara para ver al invitado.
6) Encriptación de los datos de comunicación MQTT.
7) Realizar un dispositivo definitivo, mediante una placa PCB incorporando los módulos utilizados y componentes de montaje superficial para disminuir el tamaño del dispositivo.
8) Desarrollar la conexión a múltiples dispositivos.
   
Las mejoras son ilimitadas y dependen de la priorarización de dichas mejoras. Por suerte, el proyecto es muy flexible y abierto a la ampliación de las funcionalidades y a la escalabilidad.

   


# **Bibliografía** 

\[1\] A Beginner’s Guide to Designing Embedded System Applications on Arm Cortex-M Microcontrollers, por Ariel Lutenberg, Pablo Gomez y Eric Pernia. [Online]. Available: https://www.arm.com/resources/education/books/designing-embedded-systems

\[2\] Módulo de software para el uso del chip MFRC522, por Martin Olejar. [Online]. Available: https://os.mbed.com/users/AtomX/code/MFRC522/

\[3\] Módulo de software para el manejo de Wi-Fi en el ESP32 con MQTT, por Augusto Villacampa. [Online]. Available: https://github.com/avillacampafiuba/Proyecto_Tranquera_SE

\[4\] Manual de usuario NUCLEO-f429ZI [Online]. Available: https://www.st.com/en/evaluation-tools/nucleo-f429zi.html

\[5\] Video de presentación [Online]. Available:  https://www.youtube.com/watch?v=JHn5aLL8CIk&t=3s

