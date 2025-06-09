#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <num_connections> <ip_address> <port>"
    exit 1
fi

NUM_CONNECTIONS=$1
IP_ADDRESS=$2
PORT=$3
URL="http://$IP_ADDRESS:$PORT"

# The body content you want to send (will be chunked)
CHUNK_BODY="username=testuser&password=testpassword&action=login"

# Function to convert body to hex size and format it as chunked
make_chunked_body() {
    local data="$1"
    local length=$(printf "%x" ${#data})  # Convert to hex
    echo -ne "$length\r\n$data\r\n0\r\n\r\n"
}

# Build the chunked body (data + terminating chunk)
CHUNKED_BODY=$(make_chunked_body "$CHUNK_BODY")

# Function to send a chunked POST request
send_chunked_post_request() {
    echo "Sending chunked POST request to $URL"

    (
        echo -ne "POST / HTTP/1.1\r\n"
        echo -ne "Host: $IP_ADDRESS:$PORT\r\n"
        echo -ne "Transfer-Encoding: chunked\r\n"
        echo -ne "Content-Type: application/x-www-form-urlencoded\r\n"
        echo -ne "\r\n"
        echo -ne "$CHUNKED_BODY"
    ) | nc -w 3 $IP_ADDRESS $PORT &
}

# Launch the requests in parallel
for ((i=0; i<NUM_CONNECTIONS; i++)); do
    send_chunked_post_request
done

# Wait for all background processes to complete
wait

echo "Sent $NUM_CONNECTIONS chunked POST requests to $IP_ADDRESS on port $PORT."
