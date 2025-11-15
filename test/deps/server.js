const http = require('http');
const fs = require('fs');
const WebSocket = require('ws');
const path = require('path');

const server = http.createServer();
const wss = new WebSocket.Server({ server, perMessageDeflate: false });

const filePath = path.join(__dirname, 'received_data.bin');

fs.closeSync(fs.openSync(filePath, 'w'));

let totalBytes = 0;

wss.on('connection', (ws) => {
    console.log('[SERVER]: Client connected');
    
    ws.on('message', (message, isBinary) => {
        if (isBinary) {
            totalBytes += message.length;

            const fd = fs.openSync(filePath, 'a');
            fs.writeSync(fd, message);
            fs.fsyncSync(fd);
            fs.closeSync(fd);

            console.log(`[RCV]: ${message.length}`);
        }
    });
    
    ws.on('close', () => {
        console.log(`[SERVER]: Client disconnected. Total bytes: ${totalBytes}`);
    });
});

server.listen(8080, () => {
    console.log('[SERVER]: Listening on port 8080');
});
