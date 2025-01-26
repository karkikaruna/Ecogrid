from flask import Flask, request, jsonify

app = Flask(__name__)

# In-memory storage for sensor data and alerts
sensor_data = {}
alerts = []

@app.route('/sensor_data', methods=['POST'])
def receive_sensor_data():
    global sensor_data
    temperature = request.form.get('temperature')
    humidity = request.form.get('humidity')
    ldr = request.form.get('ldr')  # Optional if you plan to add LDR later
    timestamp = request.form.get('timestamp')
    
    if temperature and humidity:
        sensor_data = {
            "temperature": float(temperature),
            "humidity": float(humidity),
            "ldr": ldr or "--",
            "timestamp": timestamp or "N/A"
        }
        return jsonify({"message": "Data received successfully", "error": None}), 200
    else:
        return jsonify({"message": "Invalid data", "error": "Missing parameters"}), 400

@app.route('/get_sensor_data', methods=['GET'])
def send_sensor_data():
    return jsonify(sensor_data)

@app.route('/alert', methods=['POST'])
def receive_alert():
    global alerts
    alert_message = request.form.get('alert')
    if alert_message:
        alerts.append((alert_message, "Timestamp"))
        return jsonify({"message": "Alert received successfully"}), 200
    else:
        return jsonify({"message": "Invalid alert", "error": "Missing parameters"}), 400

@app.route('/get_alerts', methods=['GET'])
def send_alerts():
    return jsonify({"alerts": alerts})

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
