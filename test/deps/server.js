const http = require('http');
const fs = require('fs');
const WebSocket = require('ws');

// Create HTTP server
const server = http.createServer();

// Create WebSocket server on top of HTTP
const wss = new WebSocket.Server({ server });

wss.on('connection', ws => {
  ws.on('message', message => {
    fs.appendFile('received.txt', message + '\n', err => {
      if (err) console.error('Error writing to file:', err);
    });
  });
});

server.listen(8080, () => {
  console.log('WebSocket server running at http://localhost:8080');
});
