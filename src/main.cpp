/*
// File			: main.cpp
// Project		: Page.Me-Server
// By			: Benjamin Schropp (Ben) and Andrei Pana (Andrei)
// Date			: 
// Description	: Creates the WiFi network and handles network requests from Page.Me Clients
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <WiFiUdp.h>
#include <map>
#include <iterator>

extern "C" {
    #include <user_interface.h>
}

struct teacher
{
    String tName;
    String address;
};

//SSID and Password to ESP Access Point
const char *ssid = "ESPWebServer";
const char *password = "12345678";

ESP8266WebServer server(80);

//DECLARATION OF DEVICES AND TEACHERS ASSOCIATED WITH SAID DEVICES
std::map<int, teacher> teachers;

struct station_info *stat_info;
struct ip_addr *IPaddress;
IPAddress address;

//UDP COMMS DECLARATION
/*WiFiUDP Udp;
unsigned int localUdpPort = 4210;                     // local port to listen on
char incomingPacket[255];                             // buffer for incoming packets
char replyPacekt[] = "Hi there! Got the message :-)"; // a reply string to send back
IPAddress broadcastIp;*/

void addTeacher(String teacherName, String teacherAddress);
void printTeachers();

// function prototypes for HTTP handlers
void handleRoot();              
void handleNotFound();
void handleTeacherConnectRequest();
//void dumpClients();

//===============================================================
//                  SETUP
//===============================================================
void setup(void){
    Serial.begin(9600);
    Serial.println("");
    WiFi.mode(WIFI_AP);           //Only Access point
    WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
    
    IPAddress myIP = WiFi.softAPIP(); //Get IP address
    Serial.print("Network IP: ");
    Serial.println(myIP);
    
    server.on("/", handleRoot);      //Which routine to handle at root location
    server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
    server.on("/add", handleTeacherConnectRequest);
    
    server.begin();
    Serial.println("HTTP server started.");
}
//===============================================================
//                     LOOP
//===============================================================
void loop(void){
    server.handleClient();          //Handle client requests
    //Serial.printf("Stations connected to soft-AP = %d\n", WiFi.softAPgetStationNum());
}
void handleRoot() {
    String val = server.arg("keypad");
    Serial.println(val);
    int a = atoi(val.c_str());
    Serial.println(a);
    val = a + " received.";
    server.send(200, "text/plain", val);   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
    server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void handleTeacherConnectRequest() {
    String name = server.arg("name");
    //TODO
    //add to map of teachers and addresses
    server.send(200, "text/plain", name); 
}

void addTeacher(String teacherName, String teacherAddress) {
    teacher myTeacher;
    myTeacher.tName = teacherName;
    myTeacher.address = teacherAddress;
    teachers.insert(std::pair<int, teacher>(teachers.size(), myTeacher)); 
}

void printTeachers() {
    Serial.println("Teachers contains:");    
    std::map<int, teacher>::iterator it = teachers.begin();
    for (it = teachers.begin(); it != teachers.end(); ++it) {
        Serial.printf("ID: %d\n", it->first);
        Serial.print("Name: ");
        Serial.println(it->second.tName);
        Serial.print("Address: ");                
        Serial.println(it->second.address);
    }
}

/*void dumpClients()
{
  Serial.print(" Clients:\r\n");
  stat_info = wifi_softap_get_station_info();
  while (stat_info != NULL)
  {
    IPaddress = &stat_info->ip;
    address = IPaddress->addr;
    Serial.print("\t");
    Serial.print(address);
    Serial.print("\r\n");
    stat_info = STAILQ_NEXT(stat_info, next);
  } 
}*/