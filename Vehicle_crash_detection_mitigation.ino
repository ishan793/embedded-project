//Code for vehicle crash detection and mitigation
#include <string.h>
#include <ctype.h>
int   ledPin = 13;                  // LED test pin
int   rxPin = 0;                    // RX PIN 
int   txPin = 1;                    // TX TX
char  linea[300] = "";
char  comandoGPR[7] = "$GPRMC";
int   cont=0;
int   bien=0;
int   conta=0;
int   indices[13];
char  c[50];
char  d[50];
int   byteGPS=-1;
int   flag =0;
char* cell_number = "+91**********"; // Phone number to send SMS

void setup() { 
  // initialize all serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);  // Initialize LED pin
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  for (int i=0;i<300;i++){  // Initialize a buffer for received data
  linea[i]=' ';
  }   
}

// the loop routine runs over and over 
void loop() {
  // Read the input on analog pin 0,1 and 2
  int x = analogRead(A0);
  int y = analogRead(A1);
  int z = analogRead(A2);
  // Print out the values read from accelerometer
  Serial.print("X= "); 
  Serial.print(x);
  Serial.print("; Y= ");
  Serial.print(y);
  Serial.print("; Z= ");
  Serial.println(z);
  // If accident is detected, the accelerometer's x and y values go
  // above 400
  // or if the range of values is exceeded, it displays zero.
  // Therefore, the three condition in the for loop.
  if(( x>=400 )||( y>=400 )||(x*y == 0)){
    Serial.println("Accident Detected!");
    GPScall();  // Call function that takes coordinates from GPS
  }
  delay(2000);  // delay in between reads for stability
}


//function for receiving input from GPS module, extracting relevant
//information and creating the message

void GPScall()
{
  char lat[200]="Emergency! Accident @ Latitude: ";
  while(flag!=1){
    char dummy[200]="Emergency! Accident @ Latitude: ";
    // initialize the "lat" string at every new iterations 
    strcpy(lat,dummy);
    char lon[200] =";  Longitude: ";
    digitalWrite(ledPin, LOW);
    byteGPS=Serial3.read(); // Read a byte of the serial port
    if (byteGPS == -1)    // See if the port is empty yet
    { 
      delay(100); 
      digitalWrite(ledPin, LOW);
    }

    else
    {
      digitalWrite(ledPin, LOW);
      // If there is serial port data, it is put in the buffer
      linea[conta]=byteGPS;
      conta++;                      

      if (byteGPS==13)
      { 
      /* 
      If the received byte is = to 13, end of transmission note:
      the actual end of transmission is <CR><LF> (i.e. 0x13 0x10)
      */
        digitalWrite(ledPin, LOW); 
        cont=0;
        bien=0;
        // The following for loop starts at 1 as 
        // the first byte is <LF> (0x10) from the previous
        //transmission.
        for (int i=1;i<7;i++)
        {
          // Verifies if the received command starts with $GPR
          if (linea[i]==comandoGPR[i-1])
          {
            bien++;
          }
        }
        if(bien==6){
          // If yes, continue and process the data
          for (int i=0;i<300;i++){
            if (linea[i]==','){
              // check for the position of the  "," separator
              // note: again, there is a potential buffer overflow 
              // here!
              indices[cont]=i;
              cont++;
            }

            if (linea[i]=='*'){
              // ... and the "*"
              indices[12]=i;
              cont++;
            }
          }

          int i=0;
          for(int j = indices[2]; j< indices[4]-1; j++){
            c[i++] = linea[j+1];
          }
          c[i]='\0';
          int k=0;
          for(int j = indices[4]; j< indices[6]-1; j++){
            d[k++] = linea[j+1];
          }
          d[k]='\0';

          if((c[10]=='N')&&(d[11]=='E')){
            strcat(lat,c);
            strcat(lon,d);
            strcat(lat,lon);
            flag =1;
            // having generated the final message to be 
            // send, the sendSMS function is invoked
            sendSMS(lat,cell_number);
            break;
          } 
        }
        conta=0;        // Reset the buffer
        for (int i=0;i<300;i++){      
          linea[i]=' ';             
        }                 
      }
    }
  }
}

//Function to issue commands to the gsm module

void sendCommand(char* atCommand){
  Serial.println("Checking");
  uint8_t x=0;
  char response[100];
  char c;
  unsigned long previous;

  memset(response, '\0', 100);  // Initialize the string

  delay(100);
  
  // Clean the input buffer

  while( Serial2.available() > 0) Serial2.read();
  Serial2.print(atCommand);   // Send the AT command 
  x = 0;
  previous = millis();

  // Following loop waits for 2 seconds for the answer
  do{
    if(Serial2.available() != 0){
      // if there is data in the UART input
      // buffer, read it and check for
      c = Serial2.read();
      response[x++] = c;
    }
    // Waits for the answer with time out
  }
  while(((millis() - previous) < 2000)); 
  Serial.println(response);
}

//Function for sending an sms

void sendSMS(char* message, char* number){
  char c[20] sms_command; 
  Serial.println(lat);
  Serial.println("Starting...");
  sendCommand("AT+CMGF=1\r\n");
  sprintf(sms_string,"AT+CMGS=\"%s\"\r\n", number);
  sendCommand(sms_string);
  sendCommand(message);
  sendCommand("\n");
  Serial2.print((char)0x1a);
  sendCommand("\n");
}
