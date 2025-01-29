To use the **ESP32 DevKitM-1** in the Arduino IDE, follow these steps:

---

### Step 1: Install the Arduino IDE
1. If you haven’t already, download and install the **Arduino IDE** from the [Arduino website](https://www.arduino.cc/en/software).

---

### Step 2: Install ESP32 Board Support
1. Open the Arduino IDE.
2. Go to **File** → **Preferences**.
3. In the **Additional Boards Manager URLs** field, add the following URL:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   - If there are already other URLs, separate them with a comma.

4. Click **OK** to save the preferences.

5. Go to **Tools** → **Board** → **Boards Manager**.

6. In the Boards Manager window, search for **ESP32**.

7. Install the latest version of **esp32 by Espressif Systems**.

---

### Step 3: Select the ESP32 DevKitM-1 Board
1. After installation, go to **Tools** → **Board** → **esp32**.

2. Select **ESP32 Dev Module** (this works for the ESP32 DevKitM-1).

---

### Step 4: Select the Port
1. Connect your ESP32 DevKitM-1 to your computer via USB.

2. In the Arduino IDE, go to **Tools** → **Port** and select the port that corresponds to your ESP32. (If unsure, check the **Device Manager** on Windows or `ls /dev/tty.*` on Linux/macOS.)

---

### Step 5: Test with an Example Sketch
1. Go to **File** → **Examples** → **WiFi** → **WiFiScan**.

2. Click **Upload** (the arrow icon). The sketch will compile and upload to your ESP32.

3. Open the **Serial Monitor** (**Tools** → **Serial Monitor**) and set the baud rate to **115200** to see the output.

---

### Step 6: Troubleshooting
- If the upload fails, hold down the **BOOT** button on the ESP32 DevKitM-1 while uploading and release it once the upload starts.
- Ensure you have the correct **Port** and **Board** selected in the **Tools** menu.

---
