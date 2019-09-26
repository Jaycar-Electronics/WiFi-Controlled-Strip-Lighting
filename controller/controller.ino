// Make sure this is the *edited* library, otherwise it won't work.
// for ease, we have included it in our code
#include "Adafruit_NeoPixel.h"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include "customTypes.h"
#include "lightEffects.h"

// -- Macro definitions -----------------

#define PARAM_MATCH(str, retval) \
  if (strcmp(param, str) == 0)   \
    return retval;

#define FILE_MATCH(fp, ret)  \
  if (filepath.endsWith(fp)) \
    return ret;

// --------------------------------------

/* 
Important!

  WeMos ( designers of this XC3802 board) have numbered the pins on the board
  to connect to pins that the ESP8266 in a non-one-to-one manner,

  So pin 7 on XC3802 does not mean you can access it by pinMode(7, ...); 

  If you change the Board type to "WeMos D1 R2 & Mini" it will define "D7" correctly;

  Have a look at this table: https://www.twobitarcade.net/article/wemos-d1-pins-micropython/

  Simply: as we want D7, we can access it by 13; OR we can use 'D7' if Wemos is set as board type
  */

const int led_pin = 13;
const int led_count = 39; //how many pixels do you have on this strip?

long colorOne = 0;
long colorTwo = 0;

LightEffect currentEffect = Solid;
LightEffectMix currentMix = Single;
long int currentRate = 500;

// Global "neopixel" object will handle the striplighting.
// SL3954 is GRB ordered, at 400khz; other devices (ie: XC3730) would differ
Adafruit_NeoPixel strip(led_count, led_pin, NEO_KHZ400 + NEO_RGB);

// Web server to serve our app
ESP8266WebServer server(80);

long timer = 0;

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  SPIFFS.begin();

  setupLights();
  setupNetwork("LAB", "lab12345");
  setupServer();

  // set up the mDNS server; ( not for andriod, but works on PC, Mac, and iphone )
  // this means you will be able to access http://striplight.local/  on the same network
  // change this to change the web-address; it will always end in .local
  setupMDNS("striplight");

  timer = millis(); //start timer now
}

void loop()
{
  //handle clients
  server.handleClient();
  MDNS.update();

  // rate timeout

  long durationSinceLast = millis() - timer;

  if (durationSinceLast >= rate)
  {
    //change effects as needed
    if (currentEffect == Solid)
    {
      solidEffect(strip, colorOne, colorTwo, currentMix);
    }
    else if (currentEffect == Rainbow)
    {
      rainbowEffect(strip, colorOne);
    }

    //show what we have changed the strip to
    strip.show();
    //restart the timer
    timer = millis();
  }
}

// Private functions are below
// ------------------------------------------------------------

void setupMDNS(const char *name)
{
  if (!MDNS.begin(name))
  {
    Serial.println("mDNS failed to be started");
  }
  else
  {
    Serial.print("mDNS started, hostname:");
    Serial.println(name);
    Serial.printf("(try typing 'http://%s.local/' on your computer or iphone)\n", name);
  }
}

// Lights -----------------------------------
void setupLights()
{
  strip.begin();            //starts up the connection to the strip lighting
  strip.show();             // show ( 0, 0, 0)  = (zero colour = black)
  strip.setBrightness(100); // Set brightness to be about 2/5 ( max = 255)
}

//setup network, if you want to set it as a hotspot you would do so here.
void setupNetwork(const char *SSID, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, password);
  Serial.print("Waiting to connect to network: ");
  Serial.println(SSID);

  // todo: add a timeout?
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected!\nAddress: ");
  Serial.println(WiFi.localIP());
}

void setupServer()
{
  /* This function will prepare the server for what to do on different connections.
  */
  server.on("/command", processCommand);

  /* Otherwise, on 'not found' - we can check the filesystem for a file
  and deliver that instead; 
  */
  server.onNotFound(searchFileSystem);

  server.begin();
}

void processCommand()
{
  /* in this function we look for arguments and apply them to the ESP, 
    so each iteration the ESP knows how to display the strip lighting effect.
    */

  String errorMessage = "Sent to lights!"; //lets set an error message to send if there's anything wrong.

  Serial.println("Received Request:");

  for (int i = 0; i < server.args(); i++)
  {

    const String arg = server.argName(i);
    const char *param = server.arg(i).c_str();

    //just for internal debugging
    Serial.printf("%s:%s\n", arg.c_str(), param);

    if (arg == "effect")
    {
      currentEffect = getEffectFromParams(param);
    }
    else if (arg == "mix")
    {
      currentMix = getMixFromParams(param);
    }
    else if (arg == "colorone")
    {
      colorOne = getColorFromParams(param);
    }
    else if (arg == "colortwo")
    {
      colorTwo = getColorFromParams(param);
    }
    else if (arg == "rate")
    {
      currentRate = getRateFromParams(param);
      Serial.println("set current rate to:");
      Serial.println(currentRate, DEC);
    }
    else
    {
      Serial.printf("arg '%s' is not tied to any function", arg.c_str());
    }
  }

  timer = 0; // force a time out

  server.send(200, "text/plain", errorMessage);
}

long getRateFromParams(const char *param)
{
  long retval = strtoul(param, 0, 10);

  if (retval < 10 || retval > 10000)
    retval = 500;

  return retval; //default half a second
}

LightEffect getEffectFromParams(const char *param)
{

  PARAM_MATCH("solid", Solid)
  PARAM_MATCH("flash", Strobe)
  PARAM_MATCH("fade", Fade)
  PARAM_MATCH("rainbow", Rainbow)
  //always return *something*
  return Solid;
}

LightEffectMix getMixFromParams(const char *param)
{
  PARAM_MATCH("single", Single);
  PARAM_MATCH("alternate", Alternate);
  PARAM_MATCH("mix", Mix);

  return Single; //always return something
}

long getColorFromParams(const char *param)
{
  unsigned long int colourCode = strtoul(param, 0, 16);
  Serial.printf("Converting colorParam %s as (%d,%d,%d) \n", param, (colourCode >> 16) & 0xFF, (colourCode >> 8) & 0xFF, colourCode & 0xFF);
  return colourCode;
}

// ---------------------------------------------------------------------------
void searchFileSystem()
{
  //server.uri() is the param accepted; ie:
  //    http://10.20.30.40/somefile.txt - uri is /somefile.txt
  // we will put it into a string for the string utility functions

  String filepath = server.uri();

  if (filepath.endsWith("/")) //is this a folder?
  {
    filepath += "index.html"; //index page of the folder being accessed
  }

  if (SPIFFS.exists(filepath))
  {
    String content = getFileContentType(filepath);

    Serial.printf("Serving file: %s as (%s)\n", filepath.c_str(), content.c_str());

    //Here, we've found that we have to deliver the stream manually.
    // this might be caused with an issue with the `streamFile()` function
    // so we manually implemented it for performance.

    // perhaps server.client().setNoDelay(true); solved the issue 
    // in which we can change back to streamFile
    // so we will leave this up to the pull requests 
    // to clean up this code 
    File fp = SPIFFS.open(filepath, "r");
    size_t sent = 0;
    //write our own sending loop:
    char buf[2048];
    size_t total = sent = fp.size();
    server.client().setNoDelay(true);
    while (total > 0)
    {
      size_t len = _min((int)(sizeof(buf) - 1), total);
      fp.read((byte *)buf, len);
      server.client().write_P((const char *)buf, len);

      total -= len;
    }

    Serial.printf("sent %d of %d filesize\n", sent, fp.size());
  }
  else
  {
    Serial.printf("%s was not found, plaintext response", filepath.c_str());
    server.send(404, "text/plain", filepath + " File not found, have you uploaded data to the ESP correctly?");
  }
}

String getFileContentType(String &filepath)
{

  //got the following from:
  //https://stackoverflow.com/questions/23714383/what-are-all-the-possible-values-for-http-content-type-header
  FILE_MATCH(".html", "text/html")
  FILE_MATCH(".txt", "text/plain")
  FILE_MATCH(".css", "text/css")
  FILE_MATCH(".js", "application/javascript")

  // the following is provided as a "just in case" -
  // it doesn't hurt to include them, but we haven't used them in this project (yet)
  FILE_MATCH(".mp4", "audio/mpeg")
  FILE_MATCH(".jpg", "image/jpeg")
  FILE_MATCH(".png", "image/png")

  //at the very least just return something
  return "text/plain";
}
