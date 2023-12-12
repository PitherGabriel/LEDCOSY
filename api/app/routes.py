from flask import Blueprint, request, jsonify, render_template, make_response
from . import db
from app.models import sensor_table, forecasting_table, command_table
from app.models import lst_model
import datetime
import pandas as pd
import numpy as np
import json
import logging
import math


def calculate_command(last_prediction):
    """

    """
    # Get last value from database
    last_value = float(db.session.query(sensor_table.co2).order_by(sensor_table.timestamp.desc()).limit(1).scalar())
    #Calculate variation
    if last_value == 0:
        gain=0
    else:
        gain = round((abs(last_value-last_prediction)/last_value)*100)
    
    #Determinate direction of change
    if last_prediction > last_value:
        action = 1
        print("There will be ",gain,"%. increment in CO2")
    elif last_prediction < last_value:
        action = -1
        print("There will be ",gain,"%. decrement in CO2")
    else:
        action = 0
        print("No changes on CO2 levels")
    update_command_table(action,gain)

    return action, gain

def predict():
    """
    """
    try:
        record_count = db.session.query(sensor_table).count()
        if record_count >= 24:
            # Get last 24 data from database 
            data = db.session.query(sensor_table).order_by(sensor_table.timestamp.desc()).limit(24).all()
            # Check for when there is no data in the table
            # Store data into list 
            temp, hum, co2 = [], [], []
            for record in data:
                temp.append(record.temperature)
                hum.append(record.humidity)
                co2.append(record.co2)
            # Convert lists into one array
            input_data = np.array([temp,hum,co2]).transpose()
            prediction = lst_model.predict(input_data)
            output = prediction
            update_forecasting_table(output)
            return output
        else:
            return 0
    except Exception as e:
        logging.error(f"Error prediccting CO2:{e}")

def update_forecasting_table(prediction):
    # Get last date from sensor table
    start_date_timestamp = db.session.query(sensor_table.timestamp).order_by(sensor_table.timestamp.desc()).limit(1).scalar()
    timestamp = [start_date_timestamp+datetime.timedelta(minutes=i*30) for i in range (len(prediction))] #list comprehension
    for i in range(len(prediction)):
        new_data = forecasting_table(timestamp=timestamp[i],co2=prediction[i])
        db.session.add(new_data)
    db.session.commit()
    db.session.close()

def update_command_table(action, gain):
    new_data = command_table(action=action, gain=gain)
    db.session.add(new_data)
    db.session.commit()
    db.session.close()

api = Blueprint('api', __name__)
@api.route('/')
def home():
    return render_template("index.html")

@api.route('/viewdata')
def viewdata():
    sensor_data = sensor_table.query.order_by(sensor_table.timestamp.desc()).limit(12).all()
    forecasting_data = forecasting_table.query.order_by(forecasting_table.timestamp.desc()).limit(12).all()
    command_data = command_table.query.order_by(command_table.id.desc()).limit(12).all()
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
            timestamp = datetime.datetime.now()
            if any(math.isnan(value) or value == 0 for value in [temperature, humidity,co2]):
                logging.error(f"NaN or O values error")
                return jsonify({"error":"Invalid data -NaN/0 values are not allowed"})
            # Create a new sensor_table instance and add it to the database
            new_data = sensor_table(timestamp=timestamp,temperature=temperature, humidity=humidity, co2=co2)
            db.session.add(new_data)
            db.session.commit()
            db.session.close()
            return jsonify({"message":"Data received and stored"})
        else:
            return jsonify({"error":"invalid data"})
    except Exception as e:
        logging.error(f"Error handling sensor data: {e}")
        return jsonify({"error": "Internal Server Error"})

@api.route('/getcommand', methods=["GET"])
def handle_command_request():
    try:
        command = request.get_json()
        if command:
            prediction= predict()
            #print(prediction)
            if prediction==0:
                return jsonify ({'action':0, 
                                 'gain': 0})
            
            action, gain = calculate_command(prediction[-1])
            return jsonify ({'action':action,
                             'gain': gain})
    except Exception as e:
        logging.error(f"Error: {e}")
        return jsonify({"error":"Error handling command request"})

@api.route('/testing', methods=["GET"])
def handle_test():
    try:
        prediction= predict()
        print(prediction)
        action, gain = calculate_command(prediction[-1])
        data = {'action':action,
                 'gain': gain}
        response = make_response(json.dumps(data))
        response.status_code = 200
        response.headers['Content-Type'] = 'text/plain'  # Set the content type as needed

        return response
    except Exception as e:
        logging.error(f"Error: {e}")
        return jsonify({"error":"Error handling command request"})
