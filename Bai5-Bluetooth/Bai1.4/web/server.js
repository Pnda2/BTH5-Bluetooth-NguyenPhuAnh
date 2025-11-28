const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// 1. Cấu hình Web Server
app.use(express.static('public')); // Thư mục chứa file HTML
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/public/index.html');
});

// 2. Cấu hình Serial Port (Thay đổi 'COM2' cho đúng với máy của bạn)
const port = new SerialPort({
    path: 'COM2', // Cổng COM nhận dữ liệu (Cặp của cổng Arduino gửi)
    baudRate: 9600,
    autoOpen: false
});

port.open((err) => {
    if (err) {
        return console.log('Lỗi mở cổng COM: ', err.message);
    }
    console.log('Đã mở cổng Serial thành công.');
});

// Parser để đọc từng dòng (theo ký tự xuống dòng \n từ Arduino gửi)
const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

// 3. Xử lý dữ liệu từ Serial -> Gửi xuống Web
parser.on('data', (line) => {
    try {
        console.log("Nhận từ Arduino:", line);
        const jsonData = JSON.parse(line); // Parse chuỗi JSON
        
        // Gửi dữ liệu tới tất cả client đang kết nối qua Socket.io
        io.emit('sensor-data', jsonData); 
    } catch (e) {
        console.error("Lỗi parse JSON hoặc nhiễu tín hiệu:", e);
    }
});

// 4. Khởi chạy Server
server.listen(3000, () => {
    console.log('Server đang chạy tại http://localhost:3000');
});