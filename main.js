// DOM Element references
const totalSlotsElement = document.getElementById('total-slots');
const availableSlotsElement = document.getElementById('available-slots');
const occupiedSlotsElement = document.getElementById('occupied-slots');
const themeToggle = document.getElementById('theme-toggle-input');
const activityTableBody = document.getElementById('activity-body');
const refreshButton = document.getElementById('refresh-btn');
const lastUpdateElement = document.getElementById('last-update');

// Global data object
let parkingData = {
    totalSlots: 1,
    availableSlots: 1,
    occupiedSlots: 0,
    recentActivity: []
};

// Format date string
function formatDate(dateString) {
    const date = new Date(dateString);
    return date.toLocaleString('id-ID');
}

// Update current date and time
function updateDateTime() {
    const now = new Date();
    lastUpdateElement.textContent = 'Terakhir diperbarui: ' + now.toLocaleString('id-ID');
}

// Populate dashboard data
function updateDashboard() {
    totalSlotsElement.textContent = parkingData.totalSlots;
    availableSlotsElement.textContent = parkingData.availableSlots;
    occupiedSlotsElement.textContent = parkingData.occupiedSlots;
    updateActivityTable();
    updateDateTime();
}

// Populate recent activity table
function updateActivityTable() {
    activityTableBody.innerHTML = '';
    
    if (!parkingData.recentActivity || parkingData.recentActivity.length === 0) {
        const row = document.createElement('tr');
        const cell = document.createElement('td');
        cell.colSpan = 5;
        cell.textContent = 'Belum ada aktivitas parkir';
        cell.style.textAlign = 'center';
        row.appendChild(cell);
        activityTableBody.appendChild(row);
        return;
    }
    
    parkingData.recentActivity.forEach(activity => {
        const row = document.createElement('tr');
        
        const timeCell = document.createElement('td');
        timeCell.textContent = formatDate(activity.timestamp);
        row.appendChild(timeCell);
        
        const rfidCell = document.createElement('td');
        rfidCell.textContent = activity.rfid_number;
        row.appendChild(rfidCell);
        
        const nameCell = document.createElement('td');
        nameCell.textContent = activity.name || '-';
        row.appendChild(nameCell);
        
        const plateCell = document.createElement('td');
        plateCell.textContent = activity.plate_number || '-';
        row.appendChild(plateCell);
        
        const statusCell = document.createElement('td');
        statusCell.textContent = activity.status;
        statusCell.classList.add(activity.status.toLowerCase() === 'masuk' ? 'status-masuk' : 'status-keluar');
        row.appendChild(statusCell);
        
        activityTableBody.appendChild(row);
    });
}

// Theme Toggle Functionality
function initThemeToggle() {
    // Check for saved theme preference or use preferred color scheme
    const savedTheme = localStorage.getItem('theme');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    
    if (savedTheme === 'dark' || (!savedTheme && prefersDark)) {
        document.body.classList.add('dark-mode');
        themeToggle.checked = true;
    }
    
    // Toggle theme when switch is clicked
    themeToggle.addEventListener('change', function() {
        if (this.checked) {
            document.body.classList.add('dark-mode');
            localStorage.setItem('theme', 'dark');
        } else {
            document.body.classList.remove('dark-mode');
            localStorage.setItem('theme', 'light');
        }
    });
}

// Fungsi untuk menampilkan pesan error
function showErrorMessage(message) {
    // Hapus pesan error sebelumnya jika ada
    const existingError = document.querySelector('.error-message');
    if (existingError) {
        existingError.remove();
    }
    
    const errorMsgElement = document.createElement('div');
    errorMsgElement.className = 'error-message';
    errorMsgElement.textContent = message;
    document.querySelector('.container').insertBefore(errorMsgElement, document.querySelector('.main-display'));
    
    // Remove error message after 5 seconds
    setTimeout(() => {
        const errorElement = document.querySelector('.error-message');
        if (errorElement) {
            errorElement.remove();
        }
    }, 5000);
}

// Fetch dashboard data from API with POST method
async function fetchDashboardData() {
    try {
        console.log('Fetching dashboard data...');
        const response = await fetch('http://172.20.10.3/sistem_parkir/api/get_dashboard_data.php', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                request_type: 'get_data'
            })
        });
        
        console.log('Response status:', response.status);
        
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }
        
        // Log raw response for debugging
        const rawText = await response.text();
        console.log('Raw API response:', rawText);
        
        // Parse JSON if valid
        let data;
        try {
            data = JSON.parse(rawText);
        } catch (e) {
            console.error('JSON parse error:', e);
            throw new Error('Invalid JSON response');
        }
        
        if (data.error) {
            throw new Error(`API Error: ${data.error}`);
        }
        
        console.log('Parsed data:', data);
        
        // Update global parking data
        parkingData = data;
        
        // Update UI
        updateDashboard();
        
        // Hide error message if exists
        const errorElement = document.querySelector('.error-message');
        if (errorElement) {
            errorElement.remove();
        }
        
    } catch (error) {
        console.error('Error fetching dashboard data:', error);
        showErrorMessage('Gagal memuat data. Periksa koneksi atau refresh.');
    }
}

// Initialize the application
function init() {
    console.log('Initializing application...');
    
    // Fetch initial data
    fetchDashboardData();
    
    // Init theme toggle
    initThemeToggle();
    
    // Set auto-refresh (every 5 seconds for single slot system)
    setInterval(fetchDashboardData, 5000);
    
    // Attach refresh button event
    refreshButton.addEventListener('click', function() {
        this.disabled = true;
        this.innerHTML = '<i class="fas fa-sync-alt fa-spin"></i> Memperbarui...';
        
        // Fetch new data
        fetchDashboardData().then(() => {
            this.disabled = false;
            this.innerHTML = '<i class="fas fa-sync-alt"></i> Refresh';
        }).catch(() => {
            this.disabled = false;
            this.innerHTML = '<i class="fas fa-sync-alt"></i> Refresh';
        });
    });
    
    console.log('Application initialized successfully');
}

// Run initialization when the page loads
window.addEventListener('load', init);

// Add some animations on load
document.addEventListener('DOMContentLoaded', function() {
    console.log('DOM Content Loaded');
    
    // Animate the dashboard cards
    const statusCards = document.querySelectorAll('.status-card');
    statusCards.forEach((card, index) => {
        card.style.opacity = '0';
        card.style.transform = 'translateY(20px)';
        
        setTimeout(() => {
            card.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
            card.style.opacity = '1';
            card.style.transform = 'translateY(0)';
        }, 100 * index);
    });
    
    // Add interactive hover effects
    const cards = document.querySelectorAll('.status-card');
    cards.forEach(card => {
        card.addEventListener('mouseenter', function() {
            this.style.transform = 'translateY(-8px) scale(1.02)';
            this.style.boxShadow = '0 12px 30px rgba(0, 0, 0, 0.15)';
        });
        
        card.addEventListener('mouseleave', function() {
            this.style.transform = '';
            this.style.boxShadow = '';
        });
    });
});