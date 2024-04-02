import requests

class HttpRequestHandler:
    def __init__(self, headers=None):
        self.headers = headers or {}

    def send_request(self, method, url, path='', params=None):
        full_url = f"{url.rstrip('/')}/{path.lstrip('/')}"
        try:
            response = requests.request(method, full_url, headers=self.headers, params=params)
            response.raise_for_status()  # Raise an exception for 4xx and 5xx status codes
            return response
        except requests.RequestException as e:
            print(f"Error sending request: {e}")
            return None

    def save_response(self, response, filename='http_response.txt'):
        if not response:
            print("No response to save.")
            return
        
        http_version = response.raw.version if hasattr(response.raw, 'version') else "HTTP/1.1"
        headers_str = '\n'.join([f"{key}: {value}" for key, value in response.headers.items()])
        raw_response_str = f"{http_version} {response.status_code} {response.reason}\n{headers_str}\n\n{response.text}"

        try:
            with open(filename, 'w', encoding='utf-8') as file:
                file.write(raw_response_str)
            print(f"HTTP response saved to {filename}")
        except IOError as e:
            print(f"Error writing to {filename}: {e}")

def get_user_input(prompt):
    while True:
        user_input = input(prompt)
        if user_input.strip():
            return user_input.strip()

def main():
    http_method = get_user_input("Enter HTTP method (e.g., GET, POST, etc.): ").upper()
    host_url = get_user_input("Enter Host URL (e.g., https://www.example.com): ")
    resource_path = input("Enter path to resource (optional): ")

    headers = {
        'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36',
        'Accept-Language': 'en-US,en;q=0.8,es;q=0.6,fr;q=0.4',
    }

    http_handler = HttpRequestHandler(headers=headers)
    response = http_handler.send_request(http_method, host_url, path=resource_path)
    http_handler.save_response(response)

if __name__ == "__main__":
    main()
