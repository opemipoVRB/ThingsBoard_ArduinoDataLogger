/*

$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$...........................................................................$$
$$..........................$$$$$$$$$$$$$....................................$$
$$.......................$$$$$$$$$$$$$$$$$$$.................................$$
$$.....................$$$$$$$$$$$$$$$$$$$$$$$...............................$$
$$....................$$$$$$$$$$$$$$$$$$$$$$$$$..............................$$
$$...................$$$$$$$$$$$$$$$$$$$$$$.$$...............................$$
$$...................$$$$$$$$$$$$$$$$$$$$$...$$..............................$$
$$...................$$$$$$$$$$$$$$$$$$.$$...$$$.............................$$
$$...................$$$$$$$$$$$$$$$$$$$$$$$$$$..............................$$
$$....................$$$$$$$$$$$$$.....$$$$$$$$$............................$$
$$......................$$$$$$$$$$$$$$$$..$$$$$$$............................$$
$$...................................$$$.....................................$$
$$.................$$................$$$$ $$$$$$$........$...................$$
$$...............$$$$$$..............$$$$$$$$$$$$$...$$$$$$..................$$
$$............$$$$..$$$$$.........................$$$$$$$$$..................$$
$$............$$$$...$$$$$$$....................$$$$$$.$$.$$.................$$
$$...............$$$$$$$$$$$$$$............$$$$$$$$..........................$$
$$.........................$$$$$$$$$...$$$$$$$...............................$$
$$..............................$$$$$$$$$$...................................$$
$$..........................$$$$$....$$$$$$$$$...............................$$
$$............$$.$$$$$$$$$$$$$............$$$$$$$$$$$$$$$$$..................$$
$$............$$.$$..$$$$.....................$$$$$$$$$$$$$$.................$$
$$..............$$$$$$............................$$.$$$$$$$.................$$
$$..................                                   ......................$$
$$.................. @@@  @@@  @@@@@@@        @@@@@@@ .......................$$
$$.................. @@@  @@@  @@@   @@@@     @@@   @@@@.....................$$
$$.................. @@!  @@@  @@!   !@@      @@!   !@@......................$$
$$.................. !@!  @!@  !@!   !@!      !@!   !@!......................$$
$$.................. @!@  !@!  !!@!@!!@@!     !!@!@!!@@!.....................$$
$$.................. !@!  !!!  !!!      !!!   !!!     !!!....................$$
$$.................. :!:  !!:  !!:      :!!   !!:     :::....................$$
$$................... ::!!:!   :!:      :!:   :!:     :::....................$$
$$.................... ::::    :::      :::   :::     :::....................$$
$$...................... :      :        :      :::::::  ....................$$
$$...........................................................................$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
$$***************************************************************************$$
$$ ThingsBoard_DHT_Sensor_GPRSVersion.ino Created by  Durodola Opemipo 2017  $$
$$  	  aka Emmanuel Caster @vrb_vectorian                                   $$
$$            Personal Email : <opemipodurodola@gmail.com>                   $$
$$                 Telephone Number: +2348182104309                          $$
$$***************************************************************************$$
$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

*/

#include <GSM.h>
#include "DHT.h"
#include <PubSubClient.h>
#define TOKEN "IOT_UNO"

// DHT
#define DHTPIN 4
#define DHTTYPE DHT22

// PIN Number
#define PINNUMBER "0000"

// APN data
#define GPRS_APN       "9mobile" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

char thingsboardServer[] = "https://demo.thingsboard.io";

// Initialize the Ethernet client object
GSMClient  gprsClient; // Create this Guy or find him
GPRS gprs;
GSM gsmAccess;
// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

PubSubClient client(gprsClient);

unsigned long lastSend;
void setup() {
  // initialize serial for debugging
  Serial.begin(9600);
  dht.begin();
  Serial.println("Starting Arduino web client.");
  // connection state
      boolean notConnected = true;

  while (notConnected) {
    if ((gsmAccess.begin(0000) == GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
      notConnected = false;
      Serial.println("Client Initialized");
    } else {
     Serial.println("Not connected");
      delay(1000);
    }
  }


  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}


void loop() {
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  client.loop();
}


void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");

  String temperature = String(t);
  String humidity = String(h);


  // Just debug messages
  Serial.print( "Sending temperature and humidity : [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to Thingsboard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("Arduino Uno Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}


