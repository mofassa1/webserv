#!/usr/bin/env python3

import cgi

print("Content-Type: text/html\n")

form = cgi.FieldStorage()
num = form.getvalue("number")

print("<html><body>")

try:
    # Bug: no validation if num is None or non-numeric string
    squared = int(num) ** 2
    print(f"<h1>Square of {num} is {squared}</h1>")
except Exception as e:
    print(f"<h1>Error: Invalid input '{num}'</h1>")
    print(f"<p>{e}</p>")

print("</body></html>")
