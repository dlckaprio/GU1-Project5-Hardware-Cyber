/*
 * RFID Card Reader with ESP32-C3 and MFRC522
 * 
 * Description:
 * This program interfaces an ESP32-C3 microcontroller with an MFRC522 RFID module 
 * to read UID (Unique Identification) information from RFID cards. It then makes
 * HTTP requests to a specified API to check the validity of the UID.
 * 
 * Wiring:
 * ESP32C3      MFRC522
 * 3.3v         3.3v
 * GND          GND
 * D2           IRQ
 * D6           RST
 * D4           SDA(SS)
 * D8           SCK
 * D9           MISO
 * D10          MOSI
 *
 * Developped by DlCK Aprio
 *
 */

#include <SPI.h>
#include <MFRC522.h>

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define RST_PIN               D6          // Configurable, see typical pin layout above
#define SS_PIN                D4          // Configurable, see typical pin layout above
#define LED_PIN               D3

#define SSID                  "tecktonhack"
#define PASSWORD              "KTMY!lp1XWJb&q"
#define API                   "https://tecktonhack.com/projets/projet-arduino/api_uid.php"

#define MAX_WIFI_ATTEMPTS     20
#define HTTP_TIMEOUT          5000        //5 seconds
#define MAX_HTTP_RETRIES      5
#define JSON_SIZE_FACTOR      2


MFRC522 mfrc522(SS_PIN, RST_PIN);         // Create MFRC522 instance

const char* ssid= SSID;
const char* password = PASSWORD;
const char* api = API;

const char* ssl_certificate = "-----BEGIN CERTIFICATE-----\n" \
"MIIHgTCCBWmgAwIBAgIRAMND2C/O3xpIw8AiCHFzzgkwDQYJKoZIhvcNAQEMBQAw\n" \
"SzELMAkGA1UEBhMCQVQxEDAOBgNVBAoTB1plcm9TU0wxKjAoBgNVBAMTIVplcm9T\n" \
"U0wgUlNBIERvbWFpbiBTZWN1cmUgU2l0ZSBDQTAeFw0yMzEwMTkwMDAwMDBaFw0y\n" \
"NDAxMTcyMzU5NTlaMBoxGDAWBgNVBAMTD3RlY2t0b25oYWNrLmNvbTCCAiIwDQYJ\n" \
"KoZIhvcNAQEBBQADggIPADCCAgoCggIBAKmt0h7MlAAbmuPWNy+Hjl7r6ZlevJec\n" \
"PIb+bY1tV/vTzAq9PZSljEx4L5+IQLV/jB0duoqO6KPfouXOMSUgCayErkuZi2r0\n" \
"547X94cfvO/SLzEKFth7I7KXGmBJJmX3a5mei/OpnvoTMnpObyXgTySd+Vhc3FFP\n" \
"r/t7L/zM6BiZX2hgxhMIrCZuaYhI3ymkyueuFPuJC1MW0aHcFehAG5WCZdlMQXn7\n" \
"60BTbzw6SMQg6GXnBgRMhKX6tsI+oUlM6kfdXnKFGUFyVviN7rcG7qvwcjOkkGN+\n" \
"QWJYAkP8TPlhd8Ll5r4czHxnspfJXl/12qUUxM9aVCyx50KE20bkDz6fwNicrfoD\n" \
"FYdEraotpgUWjU//g0SOb/JyC9K/FyAXa9PV0jsJl62yOGzgR/aHwDCZY7/VR4b/\n" \
"SvXNsI6hBBkH3M/jA9V2xPwngYgu3m+xkHd5j7M2d5rKgT+1asUl7F+KZkCvc3Kh\n" \
"uXVM6MlDsSEpgBzGgsOAmTIKTrMJsejB0wNElNowYodftDkzhP8X4C96aSUK3+Og\n" \
"6UOps4ESxFkuaVWdQjewQhSujbMnxOkjjm8aZ+eQU1aI62iWSxaUUP9KWSDvtFAL\n" \
"dB2tF/8p1ixCtNNzCjP+ScH/OBVARmGK6lNb1yChvfqW613WjZ/E6PmIVfGJ0oNM\n" \
"R6CLV36U05vvAgMBAAGjggKPMIICizAfBgNVHSMEGDAWgBTI2XhootkZaNU9ct5f\n" \
"Cj7ctYaGpjAdBgNVHQ4EFgQUyLEddG8A4SZwKm64TWnD9/5zq90wDgYDVR0PAQH/\n" \
"BAQDAgWgMAwGA1UdEwEB/wQCMAAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUF\n" \
"BwMCMEkGA1UdIARCMEAwNAYLKwYBBAGyMQECAk4wJTAjBggrBgEFBQcCARYXaHR0\n" \
"cHM6Ly9zZWN0aWdvLmNvbS9DUFMwCAYGZ4EMAQIBMIGIBggrBgEFBQcBAQR8MHow\n" \
"SwYIKwYBBQUHMAKGP2h0dHA6Ly96ZXJvc3NsLmNydC5zZWN0aWdvLmNvbS9aZXJv\n" \
"U1NMUlNBRG9tYWluU2VjdXJlU2l0ZUNBLmNydDArBggrBgEFBQcwAYYfaHR0cDov\n" \
"L3plcm9zc2wub2NzcC5zZWN0aWdvLmNvbTCCAQMGCisGAQQB1nkCBAIEgfQEgfEA\n" \
"7wB1AHb/iD8KtvuVUcJhzPWHujS0pM27KdxoQgqf5mdMWjp0AAABi0geVcYAAAQD\n" \
"AEYwRAIgVQB2rnsX2r0PybT/Scx54lLCGJJQDRtrCZLmaU4mIKACIErmPKBc1yrk\n" \
"/5m8ZLGIrwt2V8RW4ypFZBxYufwSLeliAHYA2ra/az+1tiKfm8K7XGvocJFxbLtR\n" \
"hIU0vaQ9MEjX+6sAAAGLSB5WHgAABAMARzBFAiEA59ZK166W3pJ9Hp9tbvX4vS3u\n" \
"xZzKw0zx9FAFR0ORW1MCICMrPaAtqwJPB6m1STGQUqq/VsBX8Nf7K22t+MIDw++3\n" \
"MC8GA1UdEQQoMCaCD3RlY2t0b25oYWNrLmNvbYITd3d3LnRlY2t0b25oYWNrLmNv\n" \
"bTANBgkqhkiG9w0BAQwFAAOCAgEAefMOsybdTtU7mEYU3W5AM/5jut81fknctqan\n" \
"MBb9a2zomi4fywmqK9u8FDwdqWX3TXs6/Y6zBYmUCAeCkysPskLC3QKfp96g2qJ2\n" \
"XxUne4J79T+PFXKiDzwu3sKIhCSR7IDvCj1Fs2UMUNDw6MhF3F3x80veFEJiWl5S\n" \
"3IQfu2DB5+haMI7R2U8MJHL+wW2V6D44JQ6BLNRWyYdjuGQ4JfCp4xOqWsGxKy3z\n" \
"36mCowi09X25hzyr0ANvQ9dhAu7z7fxPHnDEJNzYanLvKrkduNT7oiMY0AEcsmeB\n" \
"0z2Op3l4u9dROiZP0A5A5H82UDAMOTYxCDaRR+SQ+CfVM35Y3il89KTNsMnDYFVu\n" \
"UWSOAq47/+CpmfEH4TDt3zDMdw6z7Bs8R1eNCisl81GDTeD6QA6HlOXM0kpAu6fk\n" \
"4vg+FdPz9DSJ9TlvDvJfCfTLOyl+WHdzPrYKfxNqGFfl5taZEHPlEnLmfshqEtbq\n" \
"AKZj3wRh+mrCKvcp7kAUCM5d4yNxWO6Fvj6KUa2QmYOljOZ1Gq/prnmVmT7HXElE\n" \
"HSdy55G+2qPVpAiAQ5ZcHvpsVj6k4qg/d+0biJwFMy+qZ5ZHPglJc2p7ABfEXSpE\n" \
"iZRL7N+ThAkXY66pppNJso4acJ0cGkKr4g1p8/ZJKNjOzb0NObT+FXsMGRdZIviA\n" \
"sjiUufs=\n" \
"-----END CERTIFICATE-----\n";


bool waiting_for_badge = true;
bool add_uid_to_db_mode = false;

// Functions prototypes
void array_to_string(byte array[], unsigned int len, char buffer[]);
void connectToWiFi();
void performHTTPConnection(const char* uid);

// Classes
class RFIDReader{
  public:
    void init();
    void waitForBadge();
    void readUID(char uid[]);
};

void setup() {
  // Serial setup
	Serial.begin(115200);		// Initialize serial communications with the PC
	while (!Serial) {delay(100);}		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  Serial.println("\n**********************************************************");

  // WiFi setup
  while (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
    delay(1000);
  }

  // https setup

  // Led setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // MFRC522 setup
  RFIDReader rfidReader;
  rfidReader.init();
}

void loop() {
	// Step 1: get the uid of a card nearby
  RFIDReader rfidReader;
  rfidReader.waitForBadge();

	if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
	}
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  char uid[10*2+1] = "";
  rfidReader.readUID(uid);


  //Step 2: ask the api if the uid is recognized
  performHTTPSConnection(uid);

  waiting_for_badge = true;
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
   for (unsigned int i = 0; i < len; i++)
   {
      byte nib1 = (array[i] >> 4) & 0x0F;
      byte nib2 = (array[i] >> 0) & 0x0F;
      buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
      buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
   }
   buffer[len*2] = '\0';
}

void connectToWiFi() {
  
  Serial.print("[WiFi] Connection to " + String(ssid));

  WiFi.begin(ssid, password);
    
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > MAX_WIFI_ATTEMPTS) {
      Serial.println("\n[WiFi] Failed to connect. Check your credentials or try again later.");
      return;
    }
  }

  Serial.println("\n[WiFi] Connected");
  Serial.println("[WiFi] IP address: " + String(WiFi.localIP()));
}

void performHTTPSConnection(const char* uid) {
  WiFiClientSecure *client = new WiFiClientSecure;
  // set secure client with certificate
  //client->setCACert(ssl_certificate);
  client->setInsecure();
  //Serial.println(ssl_certificate);
  //create an HTTPClient instance
  HTTPClient https;

  char url[100];
  /* first method
  strcpy(url, api);
  if (!add_uid_to_db_mode) {
    strcat(url, "?uid=");
  } else {
    strcat(url, "?new_uid=");
  }
  strcat(url, uid);
  */ //second method
  snprintf(url, sizeof(url), "%s%s%s", api, (!add_uid_to_db_mode ? "?uid=" : "?new_uid="), uid);

  Serial.println("\n[HTTPS] Beginning connection to " + String(url));

  https.begin(*client, url);

  int httpCode = https.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      
      if (!add_uid_to_db_mode) {
        bool recognized;
        String payload = https.getString();
        int payloadSize = payload.length();

        DynamicJsonDocument* doc = new DynamicJsonDocument(payloadSize * JSON_SIZE_FACTOR);
        DeserializationError err = deserializeJson(*doc, payload);
        //must acces the data like this: int someValue = (*doc)["someKey"];

        if (err) {
          Serial.println("[JSON] JSON deserialization failed. Error: " + String(err.c_str()));
          return;
        }

        recognized = (*doc)["recognized"];

        if (recognized) {
          Serial.println("[API] Badge exist in database and is authorized");
          digitalWrite(LED_PIN, HIGH);
          delay(2000);
          digitalWrite(LED_PIN, LOW);
        } else {
          Serial.println("[API] Badge not recognized, please try another one");
          digitalWrite(LED_PIN, HIGH);
          delay(100);
          digitalWrite(LED_PIN, LOW);
          delay(100);
          digitalWrite(LED_PIN, HIGH);
          delay(100);
          digitalWrite(LED_PIN, LOW);
          delay(100);
          digitalWrite(LED_PIN, HIGH);
          delay(100);
          digitalWrite(LED_PIN, LOW);

        }

        delete doc;
        } else {
          //code
        }

    } else {
      switch (httpCode) {
        case HTTP_CODE_NOT_FOUND:
          Serial.println("[HTTPS] API endpoint not found.");
          break;
        case HTTP_CODE_FORBIDDEN:
          Serial.println("[HTTPS] Access to the API is forbidden.");
          break;
        case HTTP_CODE_UNAUTHORIZED:
          Serial.println("[HTTPS] Unauthorized access. Check API credentials.");
          break;
        case HTTP_CODE_INTERNAL_SERVER_ERROR:
          Serial.println("[HTTPS] Internal Server Error. Try again later.");
          break;
        default:
          Serial.println("[HTTPS] HTTP request failed. Status code: " + String(httpCode));
          break;
        }
      }
    } else {
      Serial.println("[HTTPS] Unable to connect to the server. Error Code: " + https.errorToString(httpCode));
    }

  https.end();
}

void RFIDReader::init() {
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
}

void RFIDReader::waitForBadge() {
  if (waiting_for_badge) {
	  Serial.println("\n[MRFC522] Waiting for badge...");
    waiting_for_badge = false;
  }
}

void RFIDReader::readUID(char uid[]) {
  array_to_string(mfrc522.uid.uidByte, 4, uid); //Insert (byte array, length, char array for output)
  mfrc522.PICC_HaltA(); //stop connection with the badge

  Serial.println("[MFRC522] UID has been found: " + String(uid));
}