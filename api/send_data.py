import requests
import json
import time

# Define the Raspberry Pi server URL
raspberry_pi_url = "http://192.168.1.173:5000/receive_data"  # Replace with your Raspberry Pi's IP address

# Sample data to send (in JSON format)
data_to_send = {
    "temperature": 25.5,
    "humidity": 60.0,
    "co2":345
}

# Send the data as a POST request
def send_data():
    response = requests.post(raspberry_pi_url, json=data_to_send)
    if response.status_code == 200:
        print("Data sent successfully")
    else:
        print("Failed to send data")
        print(response.text)

if __name__ == "__main__":
    while(True):
        send_data()
        time.sleep(3)
