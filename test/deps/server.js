const http = require('http');
const fs = require('fs');
const WebSocket = require('ws');
const path = require('path');

const server = http.createServer();
const wss = new WebSocket.Server({
    server,
    perMessageDeflate: false,
    maxPayload: 1024 * 1024 * 1024
});

let clientCounter = 0;

wss.on('connection', (ws) => {
    clientCounter++;
    const clientId = clientCounter;
    console.log(`[SERVER]: Client connected`);

    const filePath = path.join(__dirname, `received_data.bin`);
    const writeStream = fs.createWriteStream(filePath, {
        flags: 'a',
        highWaterMark: 64 * 1024 * 1024
    });

    let totalBytes = 0;

    ws.on('message', (message, isBinary) => {
        if (isBinary) {
            totalBytes += message.length;
            writeStream.write(message);
        }
    });

    ws.on('close', () => {
        console.log(`[SERVER]: Client disconnected. Total: ${totalBytes} bytes`);
        writeStream.end(() => {
        });
    });

    ws.on('error', (err) => {
        console.error(`[SERVER]: Error with client:`, err);
        writeStream.end(() => {
            console.log(`[SERVER]: File closed after error for client: ${filePath}`);
        });
    });
});

server.listen(8080, () => {
    console.log('[SERVER]: Listening on port 8080');
});
