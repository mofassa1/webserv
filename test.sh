#!/bin/bash

# Check if correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <num_connections> <ip_address> <port>"
    exit 1
fi

# Assign arguments to variables
NUM_CONNECTIONS=$1
IP_ADDRESS=$2
PORT=$3
URL="http://$IP_ADDRESS:$PORT"

# Function to send a single request
send_request() {
    curl -s -o /dev/null "$URL" &
}

# Loop to create multiple connections
for ((i=0; i<NUM_CONNECTIONS; i++)); do
    send_request
done

# Wait for all background processes to finish
wait

echo "Sent $NUM_CONNECTIONS connections to $IP_ADDRESS on port $PORT."
