# Statechart System Explanation

## Overview

This statechart implements an embedded systems controller using YAKINDU Statechart Tools. It's an event-driven system designed to manage hardware initialization, fault handling, and normal operation with interactive button and potentiometer controls. The system employs hierarchical state decomposition to handle multiple operational modes.

## Execution Model

- **Event-driven architecture**: The system responds to asynchronous events rather than polling
- **Super steps disabled**: Standard state execution semantics (no zero-time transitions)
- **Interfaces**: Comprehensive event and variable definitions for hardware abstraction

## Top-Level Architecture

The system is structured around a single parent state called **Blink** containing three main substates:

### 1. **Init_ST** (Initialization State)

A composite state responsible for system startup verification. It validates hardware components sequentially before transitioning to normal operation or fault handling.

#### Initialization Sequence:
```
Entry → ST_READ_FLASH → ST_READ_DIP → ST_CHECK_SENSORS → ST_RESTORE_PWM → ST_CONFG_BT → Exit
```

**States and Transitions:**

| State | Purpose | Events |
|-------|---------|--------|
| **ST_READ_FLASH** | Validate flash memory | `EV_INIT_FLASH_OK` → ST_READ_DIP<br/>`EV_INIT_FLASH_NOT_OK` → FAULT_EXIT |
| **ST_READ_DIP** | Read DIP switches (configuration) | `EV_INIT_DIP_OK` → ST_CHECK_SENSORS<br/>`EV_INIT_DIP_NOT_OK` → FAULT_EXIT |
| **ST_CHECK_SENSORS** | Verify sensor hardware | `EV_INIT_SENSORS_OK` → ST_RESTORE_PWM<br/>`EV_INIT_SENSORS_NOT_OK` → FAULT_EXIT |
| **ST_RESTORE_PWM** | Restore PWM configuration from storage | `EV_INIT_PWM_OK` → ST_CONFG_BT<br/>`EV_INIT_PWM_NOT_OK` → FAULT_EXIT |
| **ST_CONFG_BT** | Configure Bluetooth interface | `EV_INIT_BT_OK` → NORMAL_EXIT<br/>`EV_INIT_BT_NOT_OK` → FAULT_EXIT |

**Exit Conditions:**
- **NORMAL_EXIT**: Successfully triggered by `EV_INIT_BT_OK` → transitions to `Normal_ST`
- **FAULT_EXIT**: Triggered by any initialization failure → transitions to `Fault_ST`

---

### 2. **Fault_ST** (Fault/Error Handling State)

Handles system errors with an alert mechanism combining visual and audio feedback.

**Entry Action:**
```
entry / cut_off_voltage = true
```
Sets the cut-off voltage flag when entering, preventing normal operation.

#### Alert Sub-state Machine:

Two alternating states create a blinking/buzzing effect:

| State | Entry Action | Triggered By | Next State |
|-------|--------------|--------------|-----------|
| **ST_ALERT_ON** | `led_on = true; buzzer_on = true` | System entry / after 1s timeout from OFF | ST_ALERT_OFF |
| **ST_ALERT_OFF** | `led_on = false; buzzer_on = false` | after 1s timeout from ON | ST_ALERT_ON |

**Recovery:**
- After 10 seconds in `Fault_ST`: transitions back to `Init_ST` with `cut_off_voltage = false`
- This allows the system to attempt re-initialization

---

### 3. **Normal_ST** (Normal Operation State)

The operational state where the system responds to user input and manages normal functionality. It uses **parallel regions** to handle multiple concurrent concerns:

#### Region 1: Manual Light Control (ST_IDLE_MANUAL)

**Self-loop transitions (state remains active):**

1. **Button Press Event:**
   - Event: `EV_SYS_PRESSED`
   - Action: `toggle_light = true; raise EV_SEND_BT_UPDATE_LIGHT`
   - Effect: Toggles light and sends Bluetooth update notification

2. **Potentiometer Change Event:**
   - Event: `EV_POTE_CHANGED`
   - Action: `pwm_val++; raise EV_SEND_BT_UPDATE_POTE`
   - Effect: Increments PWM value and sends Bluetooth update

#### Region 2: Button Debouncing (ST_BTN)

Implements a state machine to debounce the physical button input, filtering noise and providing clean button press/release events.

**Debouncer States:**

```
       [ST_BTN_UNPRESSED] ←→ [ST_BTN_FALLING] ←→ [ST_BTN_PRESSED] ←→ [ST_BTN_RISING]
```

**State Descriptions:**

| State | Purpose | Transitions |
|-------|---------|-----------|
| **ST_BTN_UNPRESSED** | Button released state | `EV_BTN_PRESSED [tick=50]` → ST_BTN_FALLING |
| **ST_BTN_FALLING** | Falling edge (press) debounce | Counts down via `tick--` until stable<br/>`tick==0` raises `EV_SYS_PRESSED` → ST_BTN_PRESSED |
| **ST_BTN_PRESSED** | Button depressed state | `EV_BTN_UNPRESSED [tick=50]` → ST_BTN_RISING |
| **ST_BTN_RISING** | Rising edge (release) debounce | Counts down via `tick--` until stable<br/>`tick==0` raises `EV_SYS_UNPRESSED` → ST_BTN_UNPRESSED |

**Debouncing Logic:**
- Counter `tick` initialized to 50 on each edge detection
- Each event during edge decrements counter (if not at edge): `EV_BTN_PRESSED [tick>0] / tick--`
- When counter reaches 0, the edge is confirmed and appropriate event (`EV_SYS_PRESSED`/`EV_SYS_UNPRESSED`) is raised
- This filters out electrical noise on the button line

---

## Interface Definition

### Input Events

**Initialization Events:**
- `EV_INIT_FLASH_OK`, `EV_INIT_FLASH_NOT_OK`
- `EV_INIT_DIP_OK`, `EV_INIT_DIP_NOT_OK`
- `EV_INIT_SENSORS_OK`, `EV_INIT_SENSORS_NOT_OK`
- `EV_INIT_PWM_OK`, `EV_INIT_PWM_NOT_OK`
- `EV_INIT_BT_OK`, `EV_INIT_BT_NOT_OK`

**Bluetooth Events:**
- `EV_BT_CONNECTED`, `EV_BT_DISCONNECTED`

**Button Input:**
- `EV_BTN_PRESSED`, `EV_BTN_UNPRESSED` (raw input signals)

**Control Input:**
- `EV_POTE_CHANGED` (potentiometer change)

### Output Events

- `EV_SYS_PRESSED` (debounced button press)
- `EV_SYS_UNPRESSED` (debounced button release)
- `EV_SEND_BT_UPDATE_LIGHT` (light status update via Bluetooth)
- `EV_SEND_BT_UPDATE_POTE` (potentiometer value update via Bluetooth)

### State Variables

| Variable | Type | Initial Value | Purpose |
|----------|------|---------------|---------|
| `cut_off_voltage` | boolean | false | Fault state indicator; cuts power when true |
| `led_on` | boolean | false | LED feedback in fault mode |
| `buzzer_on` | boolean | false | Buzzer feedback in fault mode |
| `toggle_light` | boolean | false | Light state toggle flag |
| `tick` | integer | - | Debounce counter (0-50) |
| `pwm_val` | integer | 0 | PWM duty cycle value |

---

## State Flow Summary

```
┌─────────────┐
│   Entry     │
└──────┬──────┘
       │
       ▼
┌──────────────────────────────────────┐
│         Init_ST (Composite)          │
│  ┌────────────────────────────────┐  │
│  │ Flash → DIP → Sensors →        │  │
│  │ PWM → BT (sequential checks)   │  │
│  └────────────────────────────────┘  │
└──────┬───────────────┬────────────────┘
       │ All OK        │ Any Failure
       │               │
       ▼               ▼
┌─────────────┐  ┌─────────────────────┐
│ Normal_ST   │  │  Fault_ST           │
│ ┌─────────┐ │  │  ┌───────────────┐  │
│ │ Control ├─┼──┼──┤ Alert Loop    │  │
│ │ & Button│ │  │  │ (1s on/off)   │  │
│ └─────────┘ │  │  └───────────────┘  │
└─────────────┘  │ (after 10s → Init) │
                 └─────────────────────┘
```

---

## Key Design Patterns

### 1. **Hierarchical Decomposition**
- Top-level `Blink` state contains all functionality
- Multiple substates handle different operational phases
- Parallel regions in `Normal_ST` allow concurrent operations

### 2. **Debouncing State Machine**
- Dedicated state machine for button input filtering
- Adaptive counter mechanism prevents false triggers
- Clean separation of raw input from debounced output

### 3. **Sequential Validation**
- Initialization follows strict ordering
- Single point of failure transitions to Fault_ST
- No skipping stages prevents cascading failures

### 4. **Timeout-Based Recovery**
- 10-second timeout in Fault_ST attempts recovery
- No manual intervention required for transient faults
- Self-healing capability built into design

### 5. **Event-Driven Communication**
- Raised internal events (`EV_SYS_PRESSED`, `EV_SEND_BT_UPDATE_*`) decouple handlers
- Clean abstraction layer between hardware and application logic
- Supports asynchronous Bluetooth updates without blocking

---

## Practical Operation Example

**Scenario: User presses button in normal mode**

1. Raw `EV_BTN_PRESSED` signal arrives (noisy, bouncing)
2. **ST_BTN_UNPRESSED** → receives event, initializes counter `tick=50` → **ST_BTN_FALLING**
3. While bouncing, additional `EV_BTN_PRESSED` events decrement `tick` (50→49→48→...)
4. When electrical noise settles, `tick` reaches 0
5. Debouncer raises `EV_SYS_PRESSED` → **ST_BTN_PRESSED**
6. In parallel region, `EV_SYS_PRESSED` triggers: `toggle_light = true; raise EV_SEND_BT_UPDATE_LIGHT`
7. Button release starts rising edge debouncing similarly
8. After 50 events or 50ms settling time, `EV_SYS_UNPRESSED` is raised

**Safety:** Even with contact bounce lasting milliseconds, the system recognizes only one clean press/release event pair.

---

## Hardware Dependencies

The statechart assumes these hardware components:

- **Flash Memory**: Persistent configuration storage
- **DIP Switches**: Hardware configuration selectors
- **Sensors**: Input devices (validated during init)
- **PWM Controller**: Duty cycle output (restored from flash)
- **Bluetooth Module**: Wireless communication interface
- **LED**: Visual feedback for alerts
- **Buzzer**: Audible feedback for alerts
- **Button**: User input with debouncing via this state machine

