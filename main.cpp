#include "credentials.h"
#include "texts.h"
#include <Arduino.h>
#include <WiFi.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

WiFiClient cl;
IPAddress server(192, 168, 42, 113);
int port = 7777;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    cout << ".";
    if (WiFi.status() != WL_DISCONNECTED && WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      cout << endl << "Connection Failed" << endl;
    }
  }
  cout << '\n';
  cout << "connected to " << ssid << " with IP " << flush;
  Serial.println(WiFi.localIP());
}
bool connected = false;
bool eOI = false;
void loop() {
  if (Serial.available()) {
    char input[100] = "";
    int index = 0;
     // end of input
    if (!eOI) {
      while (Serial.available()) {

        char x;
        x = Serial.read();
        if (x == 8) { // backspace
          index--;
          Serial.write(x);
          continue;
        } else if (x == '\n' || x == '\r') {
          eOI = true;
          Serial.write(x);
          input[index++] = 0;
          break;
        } else if (x < 32)
          continue;
        input[index++] = x;
        Serial.write(x);
        delay(2);
      }
    } else {
      int inputlen = strlen(input);
      if (inputlen == 0){
        eOI=false;
        return;
      }
      if (input[0] == '.') {
        switch (input[1]) {
        case 'c': {
          char *params = (char *)malloc(inputlen - 3);
          memcpy(params, input + 3, inputlen - 3);
          char *ip = strtok(params, " ");
          String k(ip);
          server.fromString(k);
          port = atoi(strtok(NULL, " "));

          (connected = cl.connect(server, port)) ? Serial.write("Success\n")
                                                 : Serial.write("Fail\n");
          break;
        }
        case 'd': {
          if (connected)
            cl.stop();
          break;
        }
        case 't': {
          for (int i = 0; i < 24; i++)
            cl.write(TXT1[i]);
          delay(20);
          break;
        }
        case 'a': {
          if (connected) {
            char *val = (char *)malloc(inputlen - 3);
            char *o = (char *)malloc(12);
            strncpy(val, input + 3, inputlen - 3);
            strcat(o, "\033[");
            strcat(o, val);
            strcat(o, "m");
            cl.write(o);
          }
        }
        }
        eOI=false;
      } else {
        if (connected) {
          cl.write(input);
          eOI=false;
        }
        eOI=false;
      }
    }
  }
  if (connected) {
    while (cl.available()) {
      Serial.write(cl.read());
      delay(2);
      // Serial.println();
    }
  }
}
