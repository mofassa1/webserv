#!/usr/bin/env python3

print("Content-Type: text/html\r\n\r\n")

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Hello Server</title>
    <style>
        body {
            background: linear-gradient(to right, #4facfe, #00f2fe);
            color: white;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
        }
        h1 {
            font-size: 3em;
            margin-bottom: 0.2em;
        }
        p {
            font-size: 1.5em;
            background: rgba(255, 255, 255, 0.1);
            padding: 0.5em 1em;
            border-radius: 10px;
        }
        footer {
            position: absolute;
            bottom: 10px;
            font-size: 0.9em;
            opacity: 0.6;
        }
    </style>
</head>
<body>
    <h1>👋 Hello from my server!</h1>
    <p>You're in the <code>/scripts/</code> directory.</p>
    <footer>&copy; 2025 My CGI Server</footer>
</body>
</html>
""")
