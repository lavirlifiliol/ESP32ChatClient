#include "credentials.h"
#include "texts.h"
#include <Arduino.h>
#include <WiFi.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <map>



using namespace std;

WiFiClient cl;
IPAddress server;
int port;
std::map<byte, String> names;
void setup() {
  Serial.begin(115200);
  WiFi.begin();
  delay(20);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    cout << "." << flush;
    if (WiFi.status() != WL_DISCONNECTED && WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      cout << endl << "Connection Failed:" << String(WiFi.status()).c_str()<< endl;
    }
  }
  cout << '\n'<< flush;;
  cout << "connected to " << ssid << " with IP " << WiFi.localIP().toString().c_str() << endl << flush;
}



bool connected=false;
bool eOI=false;
bool ended=false;
char input[100]="";
int idx=0;
void loop(){
  if(!eOI){
    if(Serial.available()){
      char x=Serial.read();
      switch(x){
        case '\b':{
          if(idx>0){
            idx-=1;
            input[idx]=0;
            cout << '\b'<< ' ' << '\b' << flush;
            return;
          }
        }
        case '\r':
        case '\n':{
          input[idx]=0;
          idx=0;
          eOI=true;
          cout << x<<endl;
          break;
        }
        default:{
          input[idx++]=x;
          cout << x<<flush;
        }
      }
    }
  }
  else{
    int len = strlen(input);
    if(len < 3 && input[0]!='d'){
      eOI=false;
      return;
    }
    switch(input[0]){
      case 'c':{
        char *params = (char *)malloc(len - 2);
        memcpy(params, input + 2, len - 2);
        char *ip = strtok(params, " ");
        String k(ip);
        server.fromString(k);
        port = atoi(strtok(NULL, " "));

        if(connected = cl.connect(server, port)) {
          cout << "Success"<<endl;
          connected=true;
        }
        else {
          cout << "Fail"<<endl;
        }
        free(params);
        break;
      }
      case 'm':{
        char* output=(char * ) malloc(len+4);
        char temp[5]={'m','\0','\0','\0','\0'};
        memcpy(output, temp, 5);
        memcpy(output+5, input+2, len-2);
        output[len+3]=0;
        if(connected){
          cout<<"writing"<<endl;
          cl.write(output, len+4);
        }
        else{
          cout << "[E]Not connected to server"<<endl;
        }
        free(output);
        break;
      }
      case 'd':{
        if(connected){
          cl.stop();
          cout << "Disconnected"<<endl;
          connected=false;
          names.clear();
        }
        else{
          cout << "[E]Not connected to server"<<endl;
        }
        break;
      }
      case 'r':{
        if(connected){
          char * params = (char *)malloc(len);
          memcpy(params, input+2, len-2);
          char * output=(char*)malloc(len);
          int oidx = 0;
          const char s[2] = " ";
          char *token;
          token = strtok(params, s);
          /* walk through other tokens */
          while( token != NULL )
          {
            output[oidx++]=(char)strtol(token, NULL, 0);
            token = strtok(NULL, s);
          }
          cl.write((byte*)output, oidx);
          free(params);
          free(output);
        }
        else{
          cout << "[E]Not connected to server"<<endl;
        }
        break;
      }
      case 'f':{
        cout<<"\033["<<input+2<<'m';
        break;
      }
      case 'n':{
        char * output = (char *)malloc(len+4);
        output[0]='w';
        for(int i = 1; i<5; i++)output[i]=0;
        strcpy(output+5, input+2);
        cl.write(output, len+4);
        free(output);
        break;
      }
      case 'l':{
        for(auto elem:names){
          printf("%d:%s\r\n", elem.first, elem.second.c_str());
        }
        break;
      }
      case 'a':{
        names.clear();
        cl.write("a\0\0\0\0\0", 6);
      }
      case 'i':{
        byte target = strtol(input+2, NULL, 10);
        cout <<  (names[target]).c_str() << endl;
      }
      case 'e':{
        cl.write("w1234");
        cl.write(TXT1);
        cl.write("\0", 1);
      }
    }
    eOI=false;
  }
  if(cl.available()){

    cout<<'\r'<<flush;
    for(int i = 0; i < idx+2; i++)cout<<" "<<flush;
    cout<<'\r'<<flush;
    delay(40);
    while(cl.available()){//ToDebug
      char cmd=cl.read();
      unsigned char ad[4];
      for(int i = 0; i<4; i++)ad[i]=cl.read();
      IPAddress src(ad[0],ad[1],ad[2],ad[3]);
      switch(cmd){
        case 'c':{
          cout << "c:" << src.toString().c_str() << ":" <<flush;
          byte k=src[3];
          String v;
          while(true){
            char c=(char)cl.read();
            if(c==0)break;
            cout<<c;
            v+=c;
            delay(2);
          }
          names.insert(std::pair<byte, String>(k,v));
          cout<<endl;
          break;
        }
        case 'd':{
          cout << "d:" << src.toString().c_str() <<endl;
          names.erase(src[3]);
          break;
        }
        case 'm':{
          cout <<  src.toString().c_str() << ':';
          while(cl.available()){
            cout<<(char)cl.read();
          }
          cout<<endl;
          break;
        }
        case 't':{
          break;
        }
        case 'a':{
          while(1){
            char c;
            unsigned char usr[4];
            for(int i = 0; i<4; i++)usr[i]=cl.read();
            IPAddress ip(usr[0],usr[1],usr[2],usr[3]);
            cout<<ip.toString().c_str()<<':';
            String i="";
            do {
              while(!cl.available());
              c=cl.read();
              i+=c;
            } while(c!=0&&c!='\n');
            i.trim();
            cout<<i.c_str()<<endl;
            names.insert(std::pair<byte, String>(ip[3], i));
            delay(50);
            if(c==0)break;
          }
          break;
        }
        case 'k':
          break;
      }
    }  //for chat server
    /*while(cl.available()){
      Serial.write(cl.read());
    }*/
    //cout<<'\n';
    input[idx]=0;
    cout<<'\r'<<input<<flush;
  }
}
