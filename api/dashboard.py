import streamlit as st
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime, timedelta

st.set_page_config(page_title="Temperature Forecast Dashboard", page_icon="🌡️")
st.sidebar.header("Settings")

# Simulated real-time data generation (replace with your data source)
@st.cache_data
def generate_realtime_data():
    current_time = datetime.now()
    time_intervals = [current_time - timedelta(minutes=i) for i in range(30)][::-1]
    temperature_values = [np.random.uniform(20, 30) for _ in time_intervals]
    data = pd.DataFrame({"Time": time_intervals, "Temperature (°C)": temperature_values})
    return data

data = generate_realtime_data()

# Display real-time temperature data table
st.subheader("Real-Time Temperature Data")
st.write(data)

# Plot real-time temperature data
st.subheader("Real-Time Temperature Plot")
fig, ax = plt.subplots()
ax.plot(data["Time"], data["Temperature (°C)"])
ax.set_xlabel("Time")
ax.set_ylabel("Temperature (°C)")
plt.xticks(rotation=45)
st.pyplot(fig)

# Temperature forecasting input
st.subheader("Temperature Forecast")
hours_to_forecast = st.slider("Hours to forecast", min_value=1, max_value=24, value=6)
forecast_button = st.button("Forecast Temperature")

