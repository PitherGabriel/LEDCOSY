from app import db
from keras.models import load_model
from datetime import datetime
from sklearn.preprocessing import MinMaxScaler
import numpy as np

class sensor_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp =db.Column(db.DateTime, default=datetime.utcnow)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    co2 = db.Column(db.Float)


class forecasting_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp =db.Column(db.DateTime, default=datetime.utcnow)
    co2 = db.Column(db.Float)


class command_table(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    action =db.Column(db.String(255))
    gain = db.Column(db.Float)

class LSTMModel:
    # Class variable
    Scaler = MinMaxScaler(feature_range=(0,1))

    def __init__(self):
        self.model = None 
    def load_model(self, model_path):
        self.model = load_model(model_path)

    def predict(self, input_data):
        # Scale data
        scaled_data = self.Scaler.fit_transform(input_data)
        scaled_data = scaled_data.reshape(1,scaled_data.shape[0], scaled_data.shape[1])
        # Prediction and reshape
        prediction = self.model.predict(scaled_data) # (1,12)
        reshaped = prediction.transpose() # (12,1)
        reshaped = np.repeat(reshaped, 3, axis=1) # (12,3)
        # Re-scale data
        real = self.Scaler.inverse_transform(reshaped)[:,-1] # (12, )
        return real[-1] # Return last prediction

lst_model = LSTMModel()