# Perfilado de WCET y uso de CPU (copia de pruebas)

Esta carpeta es una copia de `Software STM32/main` creada para ensayos de timing.
No modifica el proyecto original.

## Qué se instrumentó

- Archivo: `app/src/app.c`
- Log por USART2 (ST-Link VCP) cada 1 segundo con:
  - cantidad de ciclos de scheduler en ventana (`n`)
  - overruns (`ov`) cuando runtime de ciclo > 1000 us
  - backlog máximo de ticks (`qmax`)
  - uso promedio y pico de CPU
  - WCET por tarea en ventana (`WCETw`)
  - WCET acumulado desde boot (`WCETb`)
  - C promedio por tarea (`Cavg`)
  - factor de uso por WCET (`Uwcet`) y promedio (`Uavg`)

## Formato de logs

Ejemplo:

```text
[PROF] n=1000 ov=0 qmax=1
[PROF] CPU avg=8.4% peak=14.2%
[PROF] WCETw us={12,35,48}
[PROF] WCETb us={15,40,52}
[PROF] Cavg  us={6,9,11}
[PROF] Uwcet=9.5% Uavg=2.6%
```

## Configuración relevante

En `app/inc/app.h`:

- `APP_PROFILE_ENABLE = 1`
- `APP_PROFILE_LOG_PERIOD_MS = 1000`
- `APP_PROFILE_TASK_PERIOD_US = 1000`
- `APP_TEST_MODE = 0` (se deja en 0 para evitar ruido de otros logs)

## Cómo correr la prueba

1. Abrir y compilar el proyecto de esta carpeta (`main_wcet_cpu_profile`).
2. Flashear en NUCLEO-F103RB.
3. Abrir consola serial del ST-Link VCP (USART2, 115200 8N1).
4. Dejar correr el sistema en los escenarios que quieras medir:
   - reposo,
   - pulsaciones ON/OFF,
   - barrido de potenciómetro,
   - modo falla.
5. Registrar las líneas `[PROF]` para completar la memoria técnica.
