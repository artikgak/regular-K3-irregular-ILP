
# example.py
import json
import requests

def run_example():
    """Calls the OR API to solve a shift scheduling problem."""
    
    # Endpoint for the workforce scheduling solver in the OR API.
    end_point = "https://optimization.googleapis.com/v1/scheduling:solveShiftGeneration"
    
    # Read the API Key from a JSON file with the format:
    # {"key": "your_api_key"}
    with open("credentials.json") as f:
        credentials = json.load(f)
        api_key = credentials["key"]

    # Load the JSON file with the request.
    with open("example_request.json", "r") as f:
        json_request = json.load(f)

    # Call the API post method.
    response = requests.post(f"{end_point}?key={api_key}", json=json_request)

    # Process the response.
    if response.ok:
        solution = json.loads(response.content)
        with open("example_response.json", "w") as f:
            json.dump(solution, f, indent=2)
        print(solution)
    else:
        error = json.loads(response.content)["error"]
        print(f'Status code {error["code"]}: {error["message"]}')

if __name__ == "__main__":
    run_example()