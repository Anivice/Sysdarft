#include "fallback.h"

const std::string fallback_page = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Sysdarft WebSocket Embedded UI (Fatal Fallback)</title>
  <style>
    html, body {
      margin: 0; padding: 0; height: 100%;
      background: black; color: white; font-family: monospace;
      display: flex; justify-content: center; align-items: center;
    }
    #screen {
      white-space: pre;
      background: black; color: white;
      margin: 0; padding: 8px;
    }
  </style>
</head>
<body>
  <pre id="screen"></pre>
  <script>
    const screenEl = document.getElementById('screen');
    // Connect to WebSocket at /ws
    const ws = new WebSocket("ws://" + location.host + "/ws");

    ws.onopen = () => {
      console.log("WebSocket open");
    };

    ws.onmessage = (evt) => {
      // The server sends the entire video buffer as text
      screenEl.textContent = evt.data;
    };

    ws.onclose = () => {
      console.log("WebSocket closed");
    };

    // Send keyboard events to server
    window.addEventListener('keydown', (e) => {
      const msg = {
        type: "keydown",
        key: e.key,
        code: e.code
      };
      ws.send(JSON.stringify(msg));
    });
  </script>
</body>
</html>
)HTML";
