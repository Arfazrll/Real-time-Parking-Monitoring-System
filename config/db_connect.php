<?php
// Database connection parameters for XAMPP
$db_host = 'localhost';      // Default XAMPP host
$db_name = 'sistem_parkir';  // Database name
$db_user = 'root';           // Default XAMPP username
$db_pass = '';               // Default XAMPP password (kosong)

// Create database connection
try {
    $conn = new PDO("mysql:host=$db_host;dbname=$db_name;charset=utf8", $db_user, $db_pass);
    
    // Set the PDO error mode to exception
    $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    
    // Set default fetch mode to associative array
    $conn->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
    
    // Enable UTF-8 support
    $conn->exec("SET NAMES utf8");
    
} catch(PDOException $e) {
    // Log the error to a file instead of displaying it
    error_log("Database connection failed: " . $e->getMessage(), 0);
    
    // Return JSON error if this is an API request
    if (strpos($_SERVER['REQUEST_URI'], '/api/') !== false) {
        header('Content-Type: application/json');
        http_response_code(500);
        echo json_encode(['error' => 'Database connection failed']);
        exit;
    }
    
    // Otherwise show a generic error
    die("Database connection failed. Please check your server logs or contact the administrator.");
}
?>