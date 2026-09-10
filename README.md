# AVR Embedded Games Project

## 📌 Project Overview

This project is an **embedded systems game project** developed using an AVR microcontroller.

The system provides a small interactive gaming platform with:

- 👥 Two-player login system
- 🔐 Player ID and password authentication
- 🎮 Two different game modes
- 🏓 Two-player Paddle Game
- 💡 LED Guessing Game
- 🖥️ 16x2 LCD display
- 🔢 7-segment score display
- ⌨️ 4x4 keypad input
- 🔊 Buzzer feedback
- 🟢🟥 LED status indicators
- 💾 EEPROM support for player high scores
- 🎲 Randomized game behavior

The project is written in **C for AVR microcontrollers**.

---

## 👨‍💻 Team Members

- **Mahmoud Essam**
- **Youssef Hegazy**
- **Youssef Mohamed**
- **Karim Arafat**
- **Youssef Farag**

---

## 🎮 Game Modes

### 1. Paddle Game

A two-player game similar to a simple Pong game.

Each player controls a paddle using the keypad:

| Player | Move Up | Move Down |
|--------|---------|-----------|
| Player 1 | `7` | `1` |
| Player 2 | `9` | `3` |

The ball moves automatically across the LCD.

- The ball bounces between the players.
- If a player misses the ball, the opponent gets a point.
- The score is displayed using the 7-segment display.
- The first player to reach **10 points** wins.
- Press `5` during the game to exit.

---

### 2. LED Guessing Game

The system randomly turns on one of three LEDs.

The player must press the corresponding keypad button:

| LED | Key |
|-----|-----|
| LED 1 | `1` |
| LED 2 | `2` |
| LED 3 | `3` |

The game provides:

- **10 attempts**
- Score tracking
- Random LED selection
- Buzzer feedback for incorrect answers
- Final score displayed on the LCD

---

## 🔐 Login System

Before accessing the games, two players must log in.

The system uses predefined player accounts:

| Player | ID | Password |
|--------|----|----------|
| P1 | `1` | `1111` |
| P2 | `2` | `2222` |
| P3 | `3` | `3333` |
| P4 | `4` | `4444` |

The password is entered through the keypad and displayed as `****` on the LCD.

A successful login turns on the green LED, while an incorrect login activates the red LED and buzzer.

---

## 🧩 Hardware Components

The project uses the following hardware components:

- AVR Microcontroller
- 16x2 LCD
- 4x4 Matrix Keypad
- Two 7-Segment Displays
- 3 LEDs
- Buzzer
- EEPROM
- Supporting resistors and wiring

---

## 🔌 Pin Configuration

### LEDs & Buzzer

| Component | Pin |
|-----------|-----|
| Red LED | PC0 |
| Green LED | PC1 |
| Buzzer | PC5 |
| Left 7-Segment | PC6 |
| Right 7-Segment | PC7 |

### LCD

| LCD Signal | AVR Pin |
|------------|---------|
| RS | PA1 |
| EN | PA2 |

### Keypad

The 4x4 keypad uses:

- Rows: `PB4 - PB7`
- Columns: `PD2 - PD5`

Keypad layout:

```text
+---+---+---+---+
| 7 | 8 | 9 | / |
+---+---+---+---+
| 4 | 5 | 6 | * |
+---+---+---+---+
| 1 | 2 | 3 | - |
+---+---+---+---+
| C | 0 | = | + |
+---+---+---+---+
```

---

## ⚙️ Main System Flow

```text
                    ┌─────────────────┐
                    │   System Start  │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Hardware Init   │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Player 1 Login  │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Player 2 Login  │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │ Select Game     │
                    └───────┬─────────┘
                            │
               ┌────────────┼────────────┐
               ▼            ▼            ▼
          ┌─────────┐  ┌─────────┐  ┌──────────┐
          │ Paddle  │  │  Guess  │  │  Logout  │
          │  Game   │  │  Game   │  │          │
          └────┬────┘  └────┬────┘  └────┬─────┘
               │            │             │
               └────────────┴─────────────┘
                            │
                            ▼
                       Select Game
```

---

## 🛠️ Software Concepts Used

The project demonstrates several embedded C concepts:

- AVR GPIO configuration
- Bit manipulation
- LCD communication
- Keypad scanning
- 7-segment multiplexing
- EEPROM read/write
- Random number generation
- Structs
- Arrays
- Functions
- String handling
- Input validation
- Timing and delays
- Game state management

---

## 📂 Project Structure

A recommended repository structure is:

```text
avr-embedded-games/
│
├── src/
│   └── main.c
│
├── README.md
│
└── LICENSE
```

---

## 🚀 How It Works

1. The AVR initializes the LCD, GPIO pins, keypad, LEDs, buzzer, and display.
2. Player 1 enters their ID and password.
3. Player 2 enters their ID and password.
4. After successful authentication, the players select a game mode.
5. The selected game starts.
6. The LCD provides game information and player interaction.
7. The 7-segment displays show the Paddle Game score.
8. The buzzer and LEDs provide feedback.
9. After the game ends, players can select another mode or log in again.

---

## 💾 EEPROM

The project defines EEPROM storage for player high scores.

Each player has a corresponding EEPROM location:

```c
uint8_t EEMEM nvHighScore[NUM_USERS];
```

The project also contains a function for updating a player's high score when a new score is higher than the stored value.

---

## 🎯 Project Goals

The main goal of this project is to combine multiple embedded-system peripherals into one interactive application.

It demonstrates how an AVR microcontroller can coordinate:

**Input → Processing → Display → Feedback → Storage**

using real hardware components.

---

## 📜 License

This project was developed as an educational embedded systems project by the team members listed above.
