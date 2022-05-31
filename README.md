# Smart-Mouse-Trap
Hardware Network System, developed on ***ESP32, ESP32Cam, and Android***. Smart Mouse Trap is designed for automatic mouse catching which allow users to monitor and control trap door via an Android app.

## Project Design
![alt text](https://cdn.discordapp.com/attachments/935406204002369586/981036279221338112/unknown.png)

## Development Technique
  - Using 2 Controller Board; ESP32 (sensor and hardware parts), and ESP32Cam (image processing)
  - Network Communication using MQTT Protocol
  - ***Due to COVID-19 Situation:*** most of development process has been done by port forwarding instead of development within the same LAN

## Main Functionality
  - ESP32Cam Controller captures image from the camera, process the image using image processing model, and send a signal to ESP32 main controller board to close the door if there is a mouse.
  - ESP32 Main Controller receives sensor data,  and waiting for signal to close/open door from others.
  - Android Client receives image data from ESP32Cam Controller in Base64 format, then, decode the image and show in the moniter screen.

## Hardware Design
![image](https://user-images.githubusercontent.com/79203105/171089746-d55ef60b-faa0-4cf6-8781-1d1dd081929b.png)

## List of Hardware Processor
  - ESP32
  - ESP32Cam
  - Android
  - MQTT Broker Laptop

