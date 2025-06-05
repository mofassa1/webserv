#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <num_connections> <ip_address> <port>"
    exit 1
fi

# Assign arguments to variables
NUM_CONNECTIONS=$1
IP_ADDRESS=$2
PORT=$3
URL="http://$IP_ADDRESS:$PORT"

# Manually set the POST body content
POST_BODY="username=testuser&password=testpassword&action=login"

# Manually calculate the Content-Length (number of characters in the POST_BODY)
CONTENT_LENGTH=${#POST_BODY}

# Manually constructing the POST request headers and body
send_post_request() {
    echo "Sending POST request to $URL with Content-Length: $CONTENT_LENGTH"
    
    # Manually set the POST request with content length and headers
    (
        echo -ne "POST / HTTP/1.1\r\n"
        echo -ne "Host: $IP_ADDRESS:$PORT\r\n"
        echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
        echo -ne "Content-Length: $CONTENT_LENGTH\r\n"
        echo -ne "\r\n"
        echo -ne "$POST_BODY"
    ) | nc -w 3 $IP_ADDRESS $PORT &
}

# Loop to create multiple connections
for ((i=0; i<NUM_CONNECTIONS; i++)); do
    send_post_request
done

# Wait for all background processes to finish
wait

echo "Sent $NUM_CONNECTIONS POST requests to $IP_ADDRESS on port $PORT with Content-Length: $CONTENT_LENGTH."

