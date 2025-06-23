#!/usr/bin/env python3

import cgi
import cgitb

print("Content-Type: text/html\r\n\r\n")

# Enable debugging (useful during development)
cgitb.enable()

print("Content-Type: text/html")  # HTTP header
print()  # Blank line to end headers

form = cgi.FieldStorage()

name = form.getvalue("name", "World")

print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI Hello</title>
</head>
<body>
    <h1>Hello, {name}!</h1>
    <form method="GET" action="/hello.py">
        <label for="name">Enter your name:</label>
        <input type="text" id="name" name="name" />
        <input type="submit" value="Greet me" />
    </form>
</body>
</html>
""")
