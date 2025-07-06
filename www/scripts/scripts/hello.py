#!/usr/bin/env python3

import os
import urllib.parse
from datetime import datetime

# Output HTTP header
print("Content-Type: text/html\r\n\r\n")

# Manually parse query string
query = os.environ.get("QUERY_STRING", "")
params = urllib.parse.parse_qs(query)
name = params.get("name", ["Guest"])[0]

# Sanitize input (very basic)
name = name.replace("<", "&lt;").replace(">", "&gt;")

# Dynamic content: server time
current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Output HTML content
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Pure Python CGI Page</title>
    <style>
        body {{
            font-family: sans-serif;
            background-color: #f7f7f7;
            color: #333;
            padding: 30px;
        }}
        .container {{
            max-width: 600px;
            margin: 0 auto;
            background-color: #fff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }}
        h1 {{
            color: #007acc;
        }}
        form input[type="text"] {{
            padding: 8px;
            width: 70%;
        }}
        form input[type="submit"] {{
            padding: 8px 16px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Hello, {name}!</h1>
        <p>The current server time is: <strong>{current_time}</strong></p>
        <form method="get">
            <input type="text" name="name" placeholder="Enter your name">
            <input type="submit" value="Submit">
        </form>
    </div>
</body>
</html>
""")