// Import wire and ESP8266 libraries 
#include <Wire.h>
#include <ESP8266WiFi.h>

// -- Setting up constants values -- 
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float ax=0, ay=0, az=0, gx=0, gy=0, gz=0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1=false; //stores if first trigger (lower threshold) has occurred
boolean trigger2=false; //stores if second trigger (upper threshold) has occurred
boolean trigger3=false; //stores if third trigger (orientation change) has occurred
byte trigger1count=0; //stores the counts past since trigger 1 was set true
byte trigger2count=0; //stores the counts past since trigger 2 was set true
byte trigger3count=0; //stores the counts past since trigger 3 was set true
int angleChange=0;
// WiFi network info.
const char *ssid =  "Joonika kim";     // Enter your WiFi Name
const char *pass =  "namjoon@123"; // Enter your WiFi Password
void send_event(const char *event);
const char *host = "maker.ifttt.com";
const char *privateKey = "drCGLsAwgYxov0-G5wooga";
// sending sms
// -- ESP8266 library --
#include <ESP8266WiFi.h>

//Constants 
WiFiClient client;
String MakerIFTTT_Key ;
String MakerIFTTT_Event;
char *append_str(char *here, String s) {  int i=0; while (*here++ = s[i]){i++;};return here-1;}
char *append_ul(char *here, unsigned long u) { char buf[20]; return append_str(here, ultoa(u, buf, 10));}
char post_rqst[256];char *p;char *content_length_here;char *json_start;int compi;

//setup function 
void setup()
{
    Serial.begin(9600);
    WiFi.disconnect();
    delay(3000);
    Serial.println("START");
    WiFi.begin("Joonika kim","namjoon@123");
    while ((!(WiFi.status() == WL_CONNECTED)))
    {
        delay(300);
        Serial.print("..");
    }
    Serial.println("Connected...");
    Serial.println("Your IP address is:");
    Serial.println((WiFi.localIP().toString()));
    if (client.connect("maker.ifttt.com",80)) 
    {
        MakerIFTTT_Key ="drCGLsAwgYxov0-G5wooga";
        MakerIFTTT_Event ="Fall_Detected_send _help";
        p = post_rqst;
        p = append_str(p, "POST /trigger/");
        p = append_str(p, MakerIFTTT_Event);
        p = append_str(p, "/with/key/");
        p = append_str(p, MakerIFTTT_Key);
        p = append_str(p, " HTTP/1.1\r\n");
        p = append_str(p, "Host: maker.ifttt.com\r\n");
        p = append_str(p, "Content-Type: application/json\r\n");
        p = append_str(p, "Content-Length: ");
        content_length_here = p;
        p = append_str(p, "NN\r\n");
        p = append_str(p, "\r\n");
        json_start = p;
        p = append_str(p, "{\"value1\":\"");
        p = append_str(p, "Mobile no with country code");
        p = append_str(p, "\",\"value2\":\"");
        p = append_str(p, "Message or SMS to send ");
        p = append_str(p, "\",\"value3\":\"");
        p = append_str(p, "");
        p = append_str(p, "\"}");

        compi= strlen(json_start);
        content_length_here[0] = '0' + (compi/10);
        content_length_here[1] = '0' + (compi%10);
        client.print(post_rqst);
    }


// Setup of sensor 
{
    Serial.begin(115200);
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    Serial.println("Wrote to IMU");
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("."); // print ... till not connected
    }
    Serial.println("");
    Serial.println("WiFi connected");
}
}

// Void loop function - running continuously 
void loop()
{
    //Reading MPU sensors data - All 6 Axis
    mpu_read();
    ax = (AcX-2050)/16384.00;
    ay = (AcY-77)/16384.00;
    az = (AcZ-1947)/16384.00;
    gx = (GyX+270)/131.07;
    gy = (GyY-351)/131.07;
    gz = (GyZ+136)/131.07;

    // calculating Amplitute vactor for 3 axis
    float Raw_Amp = pow(pow(ax,2)+pow(ay,2)+pow(az,2),0.5);
    int Amp = Raw_Amp * 10;  // Multiplied by 10 to values are between 0 to 1
    Serial.println(Amp);
    if (Amp<=2 && trigger2==false) //if AM breaks lower threshold (0.4g)
    { 
        trigger1=true;
        Serial.println("TRIGGER 1 ACTIVATED");
    }
    if (trigger1==true)
    {
        trigger1count++;
        if (Amp>=12) //if AM breaks upper threshold (3g)
        { 
            trigger2=true;
            Serial.println("TRIGGER 2 ACTIVATED");
            trigger1=false; trigger1count=0;
        }
    }
    // Trigger function 
    if (trigger2==true)
    {
        trigger2count++;
        angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5); Serial.println(angleChange);
        if (angleChange>=30 && angleChange<=400) //if orientation changes by between 80-100 degrees
        { 
            trigger3=true; trigger2=false; trigger2count=0;
            Serial.println(angleChange);
            Serial.println("TRIGGER 3 ACTIVATED");
        }
    }
    if (trigger3==true)
    {
        trigger3count++;
        if (trigger3count>=10)
        { 
            angleChange = pow(pow(gx,2)+pow(gy,2)+pow(gz,2),0.5);
            //delay(10);
            Serial.println(angleChange); 
            if ((angleChange>=0) && (angleChange<=10)) //if orientation changes remains between 0-10 degrees
            { 
                fall=true; trigger3=false; trigger3count=0;
                Serial.println(angleChange);
            }
         else  //user regained normal orientation
         {
             trigger3=false; trigger3count=0;
            Serial.println("TRIGGER 3 DEACTIVATED");
         }
        }
    }
    if (fall==true) //in event of a fall detection
    { 
        Serial.println("FALL DETECTED using MPU sensor");
        send_event("Webhooks Event Name Sending"); 
        fall=false;
    }
    if (trigger2count>=6) //allow 0.5s for orientation change
    { 
        trigger2=false; trigger2count=0;
        Serial.println("TRIGGER 2 DECACTIVATED");
    }
    if (trigger1count>=6) //allow 0.5s for AM to break upper threshold
    { 
        trigger1=false; trigger1count=0;
        Serial.println("TRIGGER 1 DECACTIVATED");
    }
    delay(100);
}
// MPU read function
void mpu_read()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}
void send_event(const char *event)
{
    Serial.print("Connecting to "); 
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) 
    {
        Serial.println("Connection failed");
        return;
    }
    
    // We now create a URI for the request
    String url = "https://maker.ifttt.com/trigger/Fall_Detected_send _help/json/with/key/drCGLsAwgYxov0-G5wooga";
    url += event;
    url += "/with/key/";
    url += privateKey;
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +"Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    while(client.connected())
    {
        if(client.available())
        {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        } 
        else {
            // No data yet, wait
            delay(50);
        };
    }
    Serial.println();
    Serial.println("closing connection");
    client.stop();
}
