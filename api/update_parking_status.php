<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Log all requests for debugging
error_log("Received request to update_parking_status.php. Method: " . $_SERVER['REQUEST_METHOD']);

// Handle OPTIONS request for CORS preflight
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Include database connection
require_once '../config/db_connect.php';

// Check if it's a POST request
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405); // Method Not Allowed
    echo json_encode(['error' => 'Only POST method is allowed']);
    exit;
}

// Get JSON data
$json_data = file_get_contents('php://input');
error_log("Received data: " . $json_data);  // Log the raw input for debugging

$data = json_decode($json_data, true);

// Check if JSON decoding was successful
if ($data === null && json_last_error() !== JSON_ERROR_NONE) {
    error_log("JSON decode error: " . json_last_error_msg());
    http_response_code(400);
    echo json_encode(['error' => 'Invalid JSON format: ' . json_last_error_msg()]);
    exit;
}

// Validate required fields
if (!isset($data['rfid_number']) || !isset($data['status'])) {
    http_response_code(400); // Bad Request
    echo json_encode(['error' => 'Missing required fields']);
    exit;
}

// Extract data
$rfid_number = $data['rfid_number'];
$status = $data['status']; // 'Masuk' or 'Keluar'

// Validasi nilai status
if ($status !== 'Masuk' && $status !== 'Keluar') {
    http_response_code(400);
    echo json_encode(['error' => 'Status harus berupa "Masuk" atau "Keluar"']);
    exit;
}

// Begin transaction
try {
    $conn->beginTransaction();
    
    // Check if the RFID number exists in the users table, if not, add it
    $stmt = $conn->prepare("SELECT COUNT(*) FROM users WHERE rfid_number = :rfid");
    $stmt->bindParam(':rfid', $rfid_number);
    $stmt->execute();
    
    if ($stmt->fetchColumn() == 0) {
        // Add new RFID card to users table
        $stmt = $conn->prepare("INSERT INTO users (rfid_number) VALUES (:rfid)");
        $stmt->bindParam(':rfid', $rfid_number);
        $stmt->execute();
    }
    
    // Record the activity
    $stmt = $conn->prepare("INSERT INTO parking_activity (rfid_number, status) VALUES (:rfid, :status)");
    $stmt->bindParam(':rfid', $rfid_number);
    $stmt->bindParam(':status', $status);
    $stmt->execute();
    
    // Update slot status based on activity type
    // Logika: 'Masuk' berarti slot terisi (is_occupied = true)
    //         'Keluar' berarti slot kosong (is_occupied = false)
    $is_occupied = ($status == 'Masuk') ? true : false;
    $stmt = $conn->prepare("UPDATE parking_slot SET is_occupied = :occupied WHERE id = 1");
    $stmt->bindParam(':occupied', $is_occupied, PDO::PARAM_BOOL);
    $stmt->execute();
    
    // Commit transaction
    $conn->commit();
    
    // Return success response
    echo json_encode([
        'success' => true,
        'message' => "Parking status updated: RFID $rfid_number, Status $status",
        'is_occupied' => $is_occupied
    ]);
    
} catch(PDOException $e) {
    // Rollback transaction on error
    $conn->rollBack();
    
    // Log error and return an error response
    error_log("API Error in update_parking_status.php: " . $e->getMessage(), 0);
    http_response_code(500);
    echo json_encode(['error' => 'Database error occurred: ' . $e->getMessage()]);
}
?>