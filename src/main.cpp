/**
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <ESP8266WebServer.h>

// WIFI related variables
char ssid[] = "iot_service_network";     //  your network SSID (name)
char password[] = "iotnetwork";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

// SERVO related variables
int pin_number = D5;
Servo myservo;

int servo_is_open;
int SERVO_DELAY = 5;

// LED variables
int ON = LOW;
int OFF = HIGH;

// WEBSERVER related variables
ESP8266WebServer server ( 80 );

// FUNCTIONs
void blink(int duration) {
  digitalWrite(LED_BUILTIN, ON); // turn the LED on (HIGH is the voltage level)
  delay(duration); // wait for the given time
  digitalWrite(LED_BUILTIN, OFF); // turn the LED off by making the voltage LOW
}

void rotate(int value) {
  myservo.attach(pin_number);
  int pos = myservo.read();
  int delta;
  if (pos <= value) {
    delta = 1;
  } else {
    delta = -1;
  }

  while(pos != value) {
    pos += delta;
    myservo.write(pos);
    delay(SERVO_DELAY);
  }
  myservo.detach();
}

void update_led_servo_status() {
  if (servo_is_open) {
    digitalWrite(D1, ON);
    digitalWrite(D2, OFF);
  } else {
    digitalWrite(D1, OFF);
    digitalWrite(D2, ON);
  }
}

void open() {
  Serial.println("opening");
  rotate(0);
  servo_is_open = 1;
  update_led_servo_status();
}

void close() {
  Serial.println("closing");
  rotate(45);
  servo_is_open = 0;
  update_led_servo_status();
}

void handleStatus() {
	digitalWrite (LED_BUILTIN, ON);
	char temp[400];
	int uptime_ms = millis();
  IPAddress localAddr = WiFi.localIP();
  byte oct1 = localAddr[0];
  byte oct2 = localAddr[1];
  byte oct3 = localAddr[2];
  byte oct4 = localAddr[3];
  char address[16];
  sprintf(address, "%d.%d.%d.%d", oct1, oct2, oct3, oct4);

	snprintf(temp, 400, "{\"uptime\":  %d, \"type\": \"iot_device\", \"status\": \"%s\", \"address\": \"%s\"}",
           uptime_ms,
           servo_is_open ? "open" : "closed",
           address);
	server.send ( 200, "text/html", temp );
	digitalWrite (LED_BUILTIN, OFF);
}

void handleOpen() {
	digitalWrite (LED_BUILTIN, ON);
  open();
	char temp[400];
	snprintf(temp, 400, "{\"action\":  \"open\", \"status\": \"OK\"}");
	server.send ( 200, "text/html", temp );
  Serial.println(HIGH);
	digitalWrite (LED_BUILTIN, OFF);
}

void handleClose() {
	digitalWrite (LED_BUILTIN, ON);
  close();
	char temp[400];
	snprintf(temp, 400, "{\"action\":  \"close\", \"status\": \"OK\"}");
	server.send ( 200, "text/html", temp );
	digitalWrite (LED_BUILTIN, OFF);
}

void handleNotFound() {
	digitalWrite (LED_BUILTIN, ON);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
	digitalWrite (LED_BUILTIN, OFF);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, OFF);

  // // initialize two outputs for "open" and "close" status
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D1, OFF);
  digitalWrite(D2, OFF);

  // Configure input
  pinMode(D3, INPUT);

  // Configure web server
  server.on ( "/", handleStatus);
  server.on ( "/status", handleStatus);
  server.on ( "/open", handleOpen);
  server.on ( "/close", handleClose);
  server.onNotFound ( handleNotFound );
  server.begin();

  // initialize door
  close();
}

void display_info() {
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Should display some information every 6 seconds
int info_show_period = 30000;
int last_info_display = -1;

void loop() {
  // put your main code here, to run repeatedly:
  int current_time = millis();
  if (last_info_display == -1 || (millis() - last_info_display) > info_show_period) {
    last_info_display = current_time;
    display_info();
  }
  server.handleClient();
  int d3_value = digitalRead(D3);
  if (d3_value == 0) {
    if (servo_is_open) {
      close();
    } else {
      open();
    }
  }
}
