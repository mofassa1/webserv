<?php
header("Content-Type: text/html");

$name = isset($_POST['name']) ? htmlspecialchars($_POST['name']) : 'Unknown';
$age = isset($_POST['age']) ? htmlspecialchars($_POST['age']) : 'Unknown';

echo "<!DOCTYPE html>";
echo "<html>";
echo "<head><title>Form Submission</title></head>";
echo "<body>";
echo "<h1>Hello, $name!</h1>";
echo "<p>You are $age years old.</p>";
echo "</body>";
echo "</html>";
?>
