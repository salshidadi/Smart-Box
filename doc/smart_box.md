# Smart Trash Box (ESP32)

A trash box that opens its lid by itself when you wave your hand over it, measures how full it is, and sends that information to a server over WiFi.

---

## What it does

1. **Opens the lid automatically.** When your hand comes close to the lid sensor, a servo motor opens the lid.
2. **Closes the lid when you're done.** When your hand moves away, the lid closes.
3. **Measures how full the box is.** A second sensor looks down into the box and calculates a fill percentage.
4. **Shows the fill level on a screen.** A small LCD displays the percentage.
5. **Reports to a server.** Every 5 seconds (and whenever the lid opens or closes), the box sends its status to a server.

---

## Hardware you need

| Part | Purpose |
|---|---|
| ESP32 board | The brain of the project |
| Servo motor | Opens and closes the lid |
| 2x Ultrasonic sensors (HC-SR04) | One detects your hand, one measures the trash level |
| 16x2 LCD with I2C | Shows the fill percentage |

### Pin connections

| Part | ESP32 Pin |
|---|---|
| Servo | 12 |
| Lid sensor (Trig / Echo) | 26 / 25 |
| Level sensor (Trig / Echo) | 5 / 18 |
| LCD (I2C address `0x27`) | Default I2C pins (SDA / SCL) |

---

## Libraries needed

Install these from the Arduino IDE Library Manager:

- `ESP32Servo`
- `LiquidCrystal_I2C`

(`Wire`, `WiFi`, and `HTTPClient` come built in with the ESP32 board package.)

---

## Settings you can change

At the top of the code:

| Setting | What it does | Current value |
|---|---|---|
| `WIFI_SSID` / `WIFI_PASS` | Your WiFi name and password | *(your own)* |
| `SERVER_URL` | Where the data is sent | *(your own)* |
| `SERVO_CLOSED` | Servo angle when the lid is closed | `90` |
| `SERVO_OPEN` | Servo angle when the lid is open | `10` |
| `EMPTY_CM` | Sensor distance when the box is empty | `30` cm |
| `FULL_CM` | Sensor distance when the box is full | `5` cm |
| `SEND_INTERVAL` | Time between regular updates | `5000` ms (5 seconds) |

> **Tip:** Measure your own box and set `EMPTY_CM` and `FULL_CM` to match it, otherwise the percentage won't be accurate.

---

## How it works

### When the ESP32 starts (`setup`)
- Sets up the servo, sensors, and LCD.
- Closes the lid.
- Connects to WiFi (and waits until it's connected).
- Shows "WiFi Connected!" on the screen.

### Then it repeats forever (`loop`)

**Part 1: Check how full the box is**
- The level sensor measures the distance down to the trash.
- The distance is converted to a percentage (close to the sensor = full, far = empty).
- The percentage shows on the LCD.
- Every 5 seconds, it's sent to the server with the status `CLOSED`.

**Part 2: Check for a hand**
- If something is closer than **10 cm** to the lid sensor, it counts as a hand.
- The lid opens, and the server is told `OPEN`.
- The lid stays open for at least 3 seconds.
- It then keeps checking every 0.15 seconds. Once the hand has been missing 3 times in a row, the lid closes.
- The server is told `CLOSED`.

---

## Data sent to the server

The box sends a **POST** request with JSON like this:

```json
{
  "device_id": "smart_box_01",
  "cover_status": "OPEN",
  "fill_percentage": 73.4
}
```

| Field | Meaning |
|---|---|
| `device_id` | Name of this box |
| `cover_status` | `OPEN` or `CLOSED` |
| `fill_percentage` | How full the box is (0 to 100) |

---

## Good to know

- **"Out of range"** on the LCD means the level sensor couldn't get a reading.
- **Status code `-1`** in the Serial Monitor means the server didn't answer in time (the data may still have arrived).
- While the lid is open, the fill level is **not updated** until the lid closes.
- The ESP32 and the server must be on the **same network**.

---
