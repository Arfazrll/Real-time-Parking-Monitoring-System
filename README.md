# 🅿️Real-time Parking Monitoring System


<img src="https://img.shields.io/badge/Web-Dashboard-orange" alt="Web Dashboard"/> <img src="https://img.shields.io/badge/Status-Active-brightgreen" alt="Status Active"/> <img src="https://img.shields.io/badge/ESP32-IoT-blue" alt="ESP32 IoT"/>

A comprehensive solution for real-time monitoring of parking spaces that integrates ESP32 microcontroller with RFID and IR sensors, providing a web-based dashboard for managing parking activity.

## 📋 Project Overview

This project implements an automated parking system with:

- **RFID Authentication**: Secure access control for vehicles
- **Real-time Monitoring**: Track parking availability instantly
- **Web Dashboard**: User-friendly interface for monitoring
- **Automatic Barrier Control**: Servo-operated parking barrier

## 🛠️ Technology Stack

### Hardware
- ESP32 microcontroller
- RFID-RC522 module
- IR sensors for detection
- Servo motor for barrier control

### Software
- **Backend**: PHP with MySQL database
- **Frontend**: HTML, CSS, JavaScript
- **Microcontroller**: C++ with Arduino framework
- **Communication**: RESTful API

## 📁 Project Structure

```
REAL-TIME-PARKING-MONITORING-SYSTEM/
├── .vscode/                  # VSCode configuration
│   ├── c_cpp_properties.json
│   ├── launch.json
│   └── settings.json
├── api/                      # Backend API endpoints
│   ├── get_dashboard_data.php
│   └── update_parking_status.php
├── config/                   # Configuration files
│   └── db_connect.php
├── database/                 # Database scripts
├── module/                   # ESP32 Code
│   └── main.cpp
├── index.html                # Main web interface
├── LICENSE                   # License file
├── main.js                   # Frontend JavaScript
├── README.md                 # Documentation
└── web.css                   # CSS styling
```

## ⚙️ Setup and Installation

### Web Server Setup
1. Configure your web server (Apache/Nginx)
2. Import database structure from the SQL file
3. Update database connection parameters in `config/db_connect.php`

### ESP32 Setup
1. Connect hardware components according to pin definitions in `module/main.cpp`
2. Update WiFi credentials and server URL
3. Flash the code to ESP32 using Arduino IDE or VSCode with PlatformIO

## 📊 Features

- Tracks parking occupancy in real-time
- Automated entry/exit with RFID authentication
- History of parking activity
- Responsive web dashboard with light/dark mode
- Visual indicators for available/occupied spaces

## 🔧 Usage

1. Access the dashboard through a web browser
2. Monitor parking status and historical data
3. Use authorized RFID cards to enter/exit the parking area
4. The system automatically updates the database when vehicles enter or exit


## 📜 License

This project is available under the MIT License. See the LICENSE file for more information.

## 👤 Author

CPS4R

---

© 2023 Real-time Parking Monitoring System
