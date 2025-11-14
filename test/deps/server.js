const http = require('http');
const fs = require('fs');
const WebSocket = require('ws');

const server = http.createServer();
const wss = new WebSocket.Server({ server, perMessageDeflate: false });
const writeStream = fs.createWriteStream('received_file.bin', { flags: 'a' });
let totalBytes = 0;

wss.on('connection', (ws) => {
    ws.on('message', (message, isBinary) => {
        if (isBinary) {
            totalBytes += message.length;
            writeStream.write(message);
        }
    });
});

server.listen(8080, () => {
    console.log('[SERVER]: Listening on port 8080');
});