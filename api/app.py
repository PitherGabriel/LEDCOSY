from flask import Flask, request, jsonify, render_template, Response
from flask_sqlalchemy import SQLAlchemy
import json
from datetime import datetime
import time

app = Flask(__name__)

# Initialize app
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///COSYDatabase.db'

# Initialize database
db = SQLAlchemy(app)

#Table to store sensor data
class sensor_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp =db.Column(db.DateTime, default=datetime.utcnow)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    co2 = db.Column(db.Float)
#Table to store forescated data
class forecasting_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp =db.Column(db.DateTime, default=datetime.utcnow)
    co2 = db.Column(db.Float)
#Table to store command
class command_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    action =db.Column(db.String(255))
    gain = db.Column(db.Float)

def predict_data():
    return 'Predict function called'   


@app.route('/')
def home():
    return render_template('index.html')

@app.route('/Forecasting')
def predict_data():
    return 'Forecasting data'

@app.route('/Database')
def view_data():                                                       
    data = sensor_table.query.all()
    return render_template('view_data.html', data=data)

@app.route('/SensorData', methods=['POST'])
def handle_sensor_data():
    try:
        #Get data from JSON data
        data = request.get_json()
        if data:
            temperature = data.get('temperature',0.0)
            humidity = data.get('humidity',0.0)
            co2 = data.get('co2',0.0)
            timestamp = datetime.now()
            # Create a new sensor_table instance and add it to the database
            new_data = sensor_table(timestamp=timestamp, temperature=temperature, humidity=humidity, co2=co2)
            db.session.add(new_data)
            db.session.commit()
            db.session.close()
            return jsonify({"message":"Data received and stored"})
        else:
            return jsonify({"error":"invalid data"})
    except Exception as e:
        print(f"Error handling sensor data: {e}")
        return jsonify({"error": "Internal Server Error"})

@app.route('/SendCommand', methods=['GET'])
def send_commmand():
    return "Inside Send Command Route"

if __name__ == "__main__":
    with app.app_context():
        db.create_all()  # Create the database table
    app.run(host='0.0.0.0', debug=True)

