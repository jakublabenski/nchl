
console.log('WebSocket ' + location.hostname);

var connection = null;

function connect() {
  console.log('Connecting');
  connection = new WebSocket('ws://' + location.hostname + ':81/');

  connection.onopen = function () {
    connection.send('Connect ' + new Date());
  };
  connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
  };
  connection.onmessage = function (e) {
    console.log('Server: ', e.data);
    var data = JSON.parse(e.data);

    if (data.server_ip != null) {
      set_server_ip(data.server_ip);
    }
    if (data.brightness != null) {
      document.getElementById('brightness').value = data.brightness;
    }
    if (data.timer != null) {
      document.getElementById('timer').checked = data.timer;
      update_timer();
    }
    if (data.start_time != null) {
      document.getElementById('start_time').value = data.start_time;  
    }
    if (data.stop_time != null) {
      document.getElementById('stop_time').value = data.stop_time;  
    }
  };
  connection.onclose = function () {
    console.log('WebSocket connection closed');
    set_server_ip("niepołączony");
    connect();
  };
}

function set_server_ip(ip) {
  var element = document.getElementById('server_ip');

  while (element.firstChild) {
    element.removeChild(element.firstChild);
  }
  element.appendChild(document.createTextNode(ip));
}

connect();



var Type = {
  RAINBOW: "R",
  RGB: "B",
  GREEN: "G",
  YELLOW: "Y",
  FLICKER_YELLOW: "F",
};

var type = Type.RAINBOW;

function update() {
  var payload = JSON.stringify({
    timer: document.getElementById('timer').checked,
    start_time: document.getElementById('start_time').value,
    stop_time: document.getElementById('stop_time').value,
    type: type,
    brightness: parseInt(document.getElementById('brightness').value, 10)
  });
  console.log('WebSocket sending ' + payload);
  connection.send(payload);
}

function update_timer()
{
  document.getElementById('start_time').disabled = !document.getElementById('timer').checked;
  document.getElementById('stop_time').disabled = !document.getElementById('timer').checked;
}

function timerChanged() {
  update_timer();
  update();
}

function setBrightness() {
  update();
}

function rainbowEffect() {
  type = Type.RAINBOW;
  update();
}

function greenEffect() {
  type = Type.GREEN;
  update();
}

function yellowEffect() {
  type = Type.YELLOW;
  update();
}

function flickerYellowEffect() {
  type = Type.FLICKER_YELLOW;
  update();
}

function rgbEffect() {
  type = Type.RGB;
  update();
}