from flask import Flask, render_template, jsonify
from flask_socketio import SocketIO, emit
import json
from datetime import datetime
import logging
import os
import requests

app = Flask(__name__)
app.config['SECRET_KEY'] = 'glucose_monitor_secret!'
socketio = SocketIO(app, cors_allowed_origins="*", logger=True, engineio_logger=True)

# Store glucose readings in memory (you might want to use a database in production)
glucose_readings = []

def get_local_ip():
    try:
        # Get all IP addresses
        import socket
        hostname = socket.gethostname()
        local_ip = socket.gethostbyname(hostname)
        return local_ip
    except:
        return "127.0.0.1"

def get_public_ip():
    try:
        response = requests.get('https://api.ipify.org?format=json')
        return response.json()['ip']
    except:
        return None

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/glucose_data')
def get_glucose_data():
    return jsonify(glucose_readings)

@socketio.on('connect')
def handle_connect():
    print('Client connected')
    emit('connection_response', {'data': 'Connected'})

@socketio.on('disconnect')
def handle_disconnect():
    print('Client disconnected')

@socketio.on('esp32_data')
def handle_esp32_data(data):
    try:
        glucose_value = float(data['glucose'])
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        reading = {
            'timestamp': timestamp,
            'value': glucose_value
        }
        glucose_readings.append(reading)
        
        # Keep only last 100 readings
        if len(glucose_readings) > 100:
            glucose_readings.pop(0)
            
        emit('new_reading', reading, broadcast=True)
        print(f"Received glucose value: {glucose_value} mg/dL")
    except Exception as e:
        logging.error(f"Error processing ESP32 data: {str(e)}")

if __name__ == '__main__':
    print("\nStarting Glucose Monitor Server...")
    
    # Get IP addresses
    local_ip = get_local_ip()
    public_ip = get_public_ip()
    
    print("\n=== Server Details ===")
    print(f"Local IP: http://{local_ip}:5000")
    if public_ip:
        print(f"Public IP: http://{public_ip}:5000")
        print("Note: To use public IP, port 5000 must be forwarded on your router")
    print("=====================\n")
    
    print("Server is running!")
    print("1. Copy the Local IP shown above into your ESP32 code")
    print("2. Update the serverHost variable in esp32_code/glucose_sender.ino")
    print("3. Upload the code to your ESP32")
    print("\nPress Ctrl+C to stop the server\n")
    
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
