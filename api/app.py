from flask import Flask, request, jsonify, render_template
import requests
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from datetime import datetime

app = Flask(__name__)

# Initialize app
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///sensor.db'
# Initialize database
db = SQLAlchemy(app)

# Create DB model
class sensor_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp =db.Column(db.DateTime, default=datetime.utcnow)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    co2 = db.Column(db.Float)

@app.route('/home')
def home():
    return "It's Home"

@app.route('/database')
def view_dat():
    data = sensor_table.query.all()
    return render_template('view_data.html', data=data)

@app.route('/receive_data', methods=['POST'])
def receive_data():
    data = request.get_json()  # Assuming data is sent as JSON
    if 'temperature' in data and 'humidity' in data and 'co2' in data:
        temperature = data['temperature']
        humidity = data['humidity']
        co2 = data['co2']
        timestamp = datetime.now()

        # Create a new sensor_table instance and add it to the database
        new_data = sensor_table(timestamp=timestamp, temperature=temperature, humidity=humidity, co2=co2)
        db.session.add(new_data)
        db.session.commit()
        db.session.close()
        return jsonify({"message":"Data received and stored"})
    else:
        return jsonify({"error":"invalid data"})        

@app.route('/send_command', methods=['POST'])
def send_commmand():
    return "Inside Send Command Route"
    
if __name__ == "__main__":
    with app.app_context():
        db.create_all()  # Create the database table
    app.run(host='0.0.0.0', debug=True)

