
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
    if (data.number_of_leds != null) {
      document.getElementById('number_of_leds').value = data.number_of_leds;  
    }
    if (data.type != null) {
      type = data.type;
    }
    if (data.server_start_time != null) {
      set_text(document.getElementById('server_start_time'), data.server_start_time);  
    }
  };
  connection.onclose = function () {
    console.log('WebSocket connection closed');
    set_server_ip("niepołączony");
    connect();
  };
}

function set_server_ip(ip)
{
  set_text(document.getElementById('server_ip'), ip);
}

function set_text(element, text) {
  while (element.firstChild) {
    element.removeChild(element.firstChild);
  }
  element.appendChild(document.createTextNode(text));
}

connect();

var Type = {
  RAINBOW: "R",
  RGBY: "B",
  RGOBY: "O",
  GREEN: "G",
  YELLOW: "Y",
  FLICKER_YELLOW: "F",
  WHITE: "W",
  RED: "D",
  BLUE: "L",
};

var type = Type.RAINBOW;

function update() {
  var payload = JSON.stringify({
    timer: document.getElementById('timer').checked,
    start_time: document.getElementById('start_time').value,
    stop_time: document.getElementById('stop_time').value,
    type: type,
    brightness: parseInt(document.getElementById('brightness').value, 10),
    number_of_leds: parseInt(document.getElementById('number_of_leds').value, 10),
  });
  console.log('WebSocket sending ' + payload);
  connection.send(payload);
}

function update_timer()
{
  document.getElementById('start_time').disabled = !document.getElementById('timer').checked;
  document.getElementById('stop_time').disabled = !document.getElementById('timer').checked;
}

function timer_changed() {
  update_timer();
  update();
}

function switch_rainbow() {
  type = Type.RAINBOW;
  update();
}

function switch_green() {
  type = Type.GREEN;
  update();
}

function switch_yellow() {
  type = Type.YELLOW;
  update();
}

function switch_flicker_yellow() {
  type = Type.FLICKER_YELLOW;
  update();
}

function switch_rgby() {
  type = Type.RGBY;
  update();
}

function switch_rgoby() {
  type = Type.RGOBY;
  update();
}

function switch_white() {
  type = Type.WHITE;
  update();
}

function switch_red() {
  type = Type.RED;
  update();
}

function switch_blue() {
  type = Type.BLUE;
  update();
}