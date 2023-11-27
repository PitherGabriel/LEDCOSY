from flask import Blueprint, request, jsonify, render_template
from . import db
from app.models import sensor_table, forecasting_table, command_table
from app.models import lst_model
from datetime import datetime
import pandas as pd
import numpy as np
import json


def calculate_command(initial_value, predicted_value):
    #Calculate variation
    variation_percentage = (abs(initial_value-predicted_value)/initial_value)*100
    #Determinate direction of change
    if predicted_value > initial_value:
        direction_change = 1
        print("There will be ",variation_percentage,"%. increment in CO2")
    elif predicted_value < initial_value:
        direction_change = -1
        print("There will be ",variation_percentage,"%. decrement in CO2")
    else:
        direction_change = 0
        print("No changes on CO2 levels")

    return direction_change, variation_percentage
    

api = Blueprint('api', __name__)
@api.route('/')
def home():
    return render_template("index.html")

@api.route('/viewdata')
def viewdata():
    sensor_data = sensor_table.query.all()
    forecasting_data = forecasting_table.query.all()
    command_data = command_table.query.all()
    return render_template('view_data.html', sensor_data =sensor_data, forecasting_data =forecasting_data, command_data=command_data)

@api.route('/updatedb', methods=["POST"])
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

@api.route('/getcommand', methods=["GET"])
def handle_command_request():
    try:
        command = request.get_json()
        if command:
            action = command.get('action')
            gain = command.get('gain')
            # Get last 24 from sensor table, process data and predict
            data = db.session.query(sensor_table).order_by(sensor_table.id).limit(24).all()
            # Store actual last value for calculation
            #initial_value = db.session.query(sensor_table).order_by(sensor_table.id).limit(1).all()
            #initial_value = float(initial_value[0].value)
            # Store data into list 
            temp, hum, co2 = [], [], []
            for record in data:
                temp.append(record.temperature)
                hum.append(record.humidity)
                co2.append(record.co2)
            # Convert lists into one array
            input = np.array([temp,hum,co2])
            prediction = lst_model.predict(input)
            action, gain = calculate_command(230,340)
            return jsonify ({'action':action,
                             'gain': gain})
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error":"Error handling command request"})