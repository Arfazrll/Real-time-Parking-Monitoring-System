<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Log all requests for debugging
error_log("Received request to get_dashboard_data.php. Method: " . $_SERVER['REQUEST_METHOD']);

// Handle OPTIONS request for CORS preflight
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Include database connection
require_once '../config/db_connect.php';

// Allow both GET and POST for flexibility during development
if ($_SERVER['REQUEST_METHOD'] !== 'POST' && $_SERVER['REQUEST_METHOD'] !== 'GET') {
    http_response_code(405); // Method Not Allowed
    echo json_encode(['error' => 'Only GET or POST methods are allowed']);
    exit;
}

// Function to get slot status
function getSlotStatus($conn) {
    try {
        $stmt = $conn->prepare("SELECT is_occupied FROM parking_slot WHERE id = 1");
        $stmt->execute();
        $result = $stmt->fetch();
        
        $totalSlots = 1; // Tetap 1 slot
        
        if ($result && isset($result['is_occupied'])) {
            $isOccupied = (bool)$result['is_occupied'];
            // Jika slot terisi, maka tersedia = 0, terisi = 1
            // Jika slot tidak terisi, maka tersedia = 1, terisi = 0
            return [
                'available' => $isOccupied ? 0 : 1, 
                'occupied' => $isOccupied ? 1 : 0
            ];
        }
        
        // Default jika tidak ada hasil: tersedia = 1, terisi = 0
        return ['available' => 1, 'occupied' => 0];
    } catch (Exception $e) {
        error_log("Error in getSlotStatus: " . $e->getMessage());
        // Default jika ada error
        return ['available' => 1, 'occupied' => 0];
    }
}

// Function to get recent activity
function getRecentActivity($conn, $limit = 5) {
    try {
        $stmt = $conn->prepare(
            "SELECT pa.rfid_number, pa.status, pa.timestamp, 
                    u.name, u.plate_number
             FROM parking_activity pa
             LEFT JOIN users u ON pa.rfid_number = u.rfid_number
             ORDER BY pa.timestamp DESC
             LIMIT :limit"
        );
        $stmt->bindParam(':limit', $limit, PDO::PARAM_INT);
        $stmt->execute();
        
        return $stmt->fetchAll();
    } catch (Exception $e) {
        error_log("Error in getRecentActivity: " . $e->getMessage());
        return []; // Return empty array in case of error
    }
}

try {
    // Get slot status
    $slotStatus = getSlotStatus($conn);
    $recentActivity = getRecentActivity($conn);
    
    // Format the data for the frontend
    $data = [
        'totalSlots' => 1, // Fixed to 1 for single parking slot
        'availableSlots' => $slotStatus['available'],
        'occupiedSlots' => $slotStatus['occupied'],
        'recentActivity' => $recentActivity,
        'lastUpdated' => date('Y-m-d H:i:s')
    ];
    
    // Return the JSON response
    echo json_encode($data);
    
} catch(PDOException $e) {
    // Log error and return an error response
    error_log("API Error in get_dashboard_data.php: " . $e->getMessage(), 0);
    http_response_code(500);
    echo json_encode(['error' => 'Database error occurred']);
}
?>