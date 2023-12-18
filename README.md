 # Smart Hose System with IoT Integration

This project integrates a Smart Hose System with IoT capabilities, encompassing two main components: an Arduino-based control system for accessing the house and an ESP32-based system for home automation and security. Additionally, a Flask web application serves as an API to facilitate remote control and monitoring.

## Used Components

- ESP32 
- Arduino UNO
- Servo Motor
- 4x4 KEYPAD 
- Light Sensor (LDR  4 pins)
- Temperature & Humidity Sensor (DHT11)
- Motion Sensor (PIR) 
- Relay Module 1 channel
- Adapter 12v output
- Fan
- LEDs
- Wires 
- Resistors

## Arduino Code (enteringHouseWithPasswd_Arduino.ino)

The Arduino code (`enteringHouseWithPasswd_Arduino.ino`) controls house access using a password-based mechanism. Key features include:

- **Keypad Input**: Utilizes a 4x4 keypad for password entry.
- **Servo Control**: Manages a servo motor to lock and unlock the door.
- **Password Protection**: Allows configuration of a password to unlock the door.
- **Intrusion Detection**: Monitors unsuccessful attempts and triggers an intrusion alert if the maximum limit is reached.

### Usage

1. Set the correct `servoPin`, `ROW_NUM`, `COLUMN_NUM`, and keypad layout.
2. Define the correct `correctPassword`.
3. Adjust `maxWrongAttempts` as needed.
4. Upload the code to your Arduino device.

## ESP32 Code (homeSmartSystem_esp32.ino)

The ESP32 code (`homeSmartSystem_esp32.ino`) focuses on home automation and security, incorporating the following features:

- **WiFi Connection**: Connects to a WiFi network for remote access.
- **Sensors Integration**: Utilizes a PIR motion sensor, DHT11 temperature and humidity sensor, and a light sensor.
- **Home Automation**: Controls a relay module to manage lights and a fan based on environmental conditions.
- **Email Notifications**: Sends email notifications in case of an intrusion attempt.

### Usage

1. Set the correct `ssid` and `password` for your WiFi network.
2. Configure the email settings in the code (SMTP server, sender credentials, etc.).
3. Customize the pins for sensors and actuators as needed.
4. Upload the code to your ESP32 device.

## Flask Server (web server directory)

The `web server` directory contains a Flask web application (`app.py`) acting as an API for remote control and monitoring of the smart home system. Features include:

- **User Authentication**: Supports user sign-up and login functionalities.
- **Warnings System**: Logs warnings for intrusion attempts.
- **Remote Control Endpoints**: Provides endpoints to control lights, fans, and reset the system remotely.

### Usage

1. Navigate to the `web server` directory.
2. Install the required dependencies using `pip install -r requirements.txt`.
3. Set environment variables (e.g., `SECRET_KEY`) in a `.env` file.
4. Run the Flask server with `python app.py`.

## Database Handling (database.py)

The `database.py` file manages SQLite3 database operations, including user data storage, warnings logging, and authentication.

### Usage

1. Update the `ESP32_IP_ADDRESS` in `app.py` with the correct IP address of your ESP32 device.
2. Ensure the `SECRET_KEY` environment variable is set for Flask.

## Contributing

Feel free to contribute to this project by opening issues or submitting pull requests. Your input is valuable!

## License

This project is licensed under the [MIT License](LICENSE).
