#!/usr/bin/php
<?php
// CGI header to tell the browser what kind of content to expect
header("Content-Type: text/html");

// Parse GET parameters
$name = isset($_GET['name']) ? htmlspecialchars($_GET['name']) : 'Guest';

// Output a simple HTML response
echo "<!DOCTYPE html>";
echo "<html>";
echo "<head><title>PHP CGI GET Example</title></head>";
echo "<body>";
echo "<h1>Hello, $name!</h1>";
echo "<p>This is a response from a PHP CGI script using GET method.</p>";
echo "</body>";
echo "</html>";
?>
