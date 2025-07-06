#!/usr/bin/env python3

import sys

print("Content-Type: text/html\n")

# This will raise a ZeroDivisionError and crash the script
result = 1 / 0

print(f"<html><body><h1>Result is {result}</h1></body></html>")
