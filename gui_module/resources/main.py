# main.py
import json
import requests
import time
import threading

server_alive = True

def status_check():
    global server_alive
    url = "http://localhost:50001/status"
    while server_alive:
        try:
            response = requests.post(url)

            if response.status_code == 200:
                received_json = response.json()
                print("Client: Received JSON from server:")
                print(json.dumps(received_json, indent=4))

                if received_json["status"] == "dying":
                    print("Server reporting dying...")
                    server_alive = False

            else:
                print(f"Client: Server responded with status code {response.status_code}")
                print(f"Client: Response: {response.text}")

        except requests.exceptions.RequestException as e:
            print(f"Client: Request status failed: {e}")
            server_alive = False

        # Wait before the next attempt
        time.sleep(1)

def run_client():
    """
    Function to act as an HTTP client, sending JSON to the C++ server.
    """
    url = "http://localhost:50001/portal"
    headers = {'Content-Type': 'application/json'}

    # Create JSON data to send
    json_to_send = {
        "request": "dummy",
        "parameters": [ "param1", "param2" ]
    }

    # Serialize JSON to string
    json_input = json.dumps(json_to_send)

    i = 0
    global server_alive
    # Attempt to send the request multiple times with delays
    while server_alive:
        try:
            print(f"Client: Sending JSON to server (Attempt {i+1})...")
            response = requests.post(url, headers=headers, data=json_input)

            if response.status_code == 200:
                received_json = response.json()
                print("Client: Received JSON from server:")
                print(json.dumps(received_json, indent=4))
            else:
                print(f"Client: Server responded with status code {response.status_code}")
                print(f"Client: Response: {response.text}")

        except requests.exceptions.RequestException as e:
            print(f"Client: Request failed: {e}")

        # Wait before the next attempt
        time.sleep(1)

if __name__ == "__main__":
    thread = threading.Thread(target=status_check, daemon=True)
    thread.start()
    run_client()
