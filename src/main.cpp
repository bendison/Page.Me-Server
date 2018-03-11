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
#include <list>
#include <iterator>

extern "C" {
    #include <user_interface.h>
}

struct teacher
{
    String tName;
    String address;
};

String requestedTeacher = "";
String requestedTeacherResponse = "";

//TODO
//Add struct for requests

//SSID and Password to ESP Access Point
const char *ssid = "ESPWebServer";
const char *password = "12345678";

ESP8266WebServer server(80);

//DECLARATION OF DEVICES AND TEACHERS ASSOCIATED WITH SAID DEVICES
std::map<int, teacher> teachers;

std::map<int, String> responses;

String request = "";

String testResponse = "";

struct station_info *stat_info;
struct ip_addr *IPaddress;
IPAddress address;

//UDP COMMS DECLARATION
/*WiFiUDP Udp;
unsigned int localUdpPort = 4210;                     // local port to listen on
char incomingPacket[255];                             // buffer for incoming packets
char replyPacekt[] = "Hi there! Got the message :-)"; // a reply string to send back
IPAddress broadcastIp;*/

int findTeacher(String teacherName, String teacherAddress);
void replaceAddress(String teacherName, String teacherAddress);
void addTeacher(String teacherName, String teacherAddress);
void printTeachers();

// function prototypes for HTTP handlers
void handleRoot();              
void handleNotFound();
void handleCheck();
void handleTeacherConnectRequest();
void handleTest();
void handleTeacherResponse();
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
    server.on("/check", handleCheck);
    server.on("/test", handleTest);
    server.on("/response", handleTeacherResponse);
    
    server.begin();
    Serial.println("HTTP server started.");

    responses.insert(std::pair<int, String>(1, "Available."));
    responses.insert(std::pair<int, String>(2, "Back in 5 min."));
    responses.insert(std::pair<int, String>(3, "Back in 10 min."));
    responses.insert(std::pair<int, String>(4, "Back in 15 min."));
    responses.insert(std::pair<int, String>(5, "Not available."));
}
//===============================================================
//                     LOOP
//===============================================================
void loop(void){
    server.handleClient();          //Handle client requests
    if (Serial.available() > 0) {
        String message = Serial.readString();
        request = message;
        Serial.println(request);
    }
    //Serial.printf("Stations connected to soft-AP = %d\n", WiFi.softAPgetStationNum());
}

void handleTest() {
    server.send(200, "text/plain", testResponse);    
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

void handleTeacherResponse() {
    String val = server.arg("response");
    Serial.println(val);
    int a = atoi(val.c_str());
    Serial.println(responses.at(a));
    server.send(200, "text/plain", val); //send back for confirmation of delivery
    //write over serial to server backend to show response to student
}

void handleCheck() {
    Serial.print("Sending response: ");
    Serial.print(request);
    server.send(200, "text/plain", request);
    //String val = server.arg("teacher");
    //if request, send teacher
    //if response write over serial
}

void handleTeacherConnectRequest() {
    String teacherName = server.arg("name");
    String teacherAddress = server.arg("address");
    Serial.println("New Teacher Connect Request");
    int key = findTeacher(teacherName, teacherAddress);
    if (key == -1) {
        Serial.println("New teacher attempting to join the network: ");
        Serial.print(teacherName);
        Serial.print(" : ");
        Serial.println(teacherAddress);
        addTeacher(teacherName, teacherAddress);
        //Send confirmation
        server.send(200, "text/plain", teacherName); 
    }
    else if (key == -2) {
        Serial.println("Teacher already exists in the network but there is an address mismatch, updating the address.");        
        replaceAddress(teacherName, teacherAddress);
        server.send(200, "text/plain", "updated");   
    }
    else {
        Serial.println("Teacher already exists in the network.");
        server.send(200, "text/plain", "existError");         
    }
    printTeachers();
}

int findTeacher(String teacherName, String teacherAddress) {
    std::map<int, teacher>::const_iterator it;
    int key = -1;
    for (it = teachers.begin(); it != teachers.end(); ++it)
    {
        Serial.println(it->second.tName);
        Serial.println(teacherName);
        //If names match we assume the teacher already exists in the map
        if (it->second.tName.compareTo(teacherName) == 0 && it->second.address.compareTo(teacherAddress) == 0)
        {
            key = it->first;
            break;
        }
        else if (it->second.tName.compareTo(teacherName) == 0 && it->second.address.compareTo(teacherAddress) != 0) {
            key = -2;
            break;
        }
    }
    return key;
}

void replaceAddress(String teacherName, String teacherAddress) {
    std::map<int, teacher>::iterator it; 
    for (it = teachers.begin(); it != teachers.end(); ++it)
    {
        if (it != teachers.end()) {
            it->second.address = teacherAddress;
            Serial.print("Replaced address for ");
            Serial.print(teacherName);
            Serial.print(" with ");
            Serial.println(teacherAddress);
            break;
        }
    }
}

void addTeacher(String teacherName, String teacherAddress) {
    teacher myTeacher;
    myTeacher.tName = teacherName;
    myTeacher.address = teacherAddress;
    teachers.insert(std::pair<int, teacher>(teachers.size(), myTeacher)); 
    Serial.println("New teacher added to list of teachers.");
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