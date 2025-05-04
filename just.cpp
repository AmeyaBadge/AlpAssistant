#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *ssid = "Tissuepaper";
const char *password = "Dontknow";

const char *pcIP = "http://192.168.1.28";                    // Flask server IP
const char *smartAPI = "http://192.168.1.28:2001/api/smart"; // Node Smart API

String urlencode(String str)
{
    String encoded = "";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i = 0; i < str.length(); i++)
    {
        c = str.charAt(i);
        if (isalnum(c))
        {
            encoded += c;
        }
        else
        {
            code1 = (c >> 4) & 0xF;
            code2 = c & 0xF;
            code0 = '%';
            encoded += code0;
            encoded += char(code1 > 9 ? code1 - 10 + 'A' : code1 + '0');
            encoded += char(code2 > 9 ? code2 - 10 + 'A' : code2 + '0');
        }
    }
    return encoded;
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("\nConnected! IP address: ");
    Serial.println(WiFi.localIP());

    // Step 1: Trigger listening on PC
    HTTPClient http;
    WiFiClient client1;
    String startUrl = String(pcIP) + ":5000/start-listening";
    Serial.println("Triggering speech recognition...");
    http.begin(client1, startUrl);
    http.setTimeout(15000); // <-- add this
    int httpCode = http.GET();
    Serial.print("HTTP GET Code: ");
    Serial.println(httpCode);
    Serial.println(http.getString());
    Serial.println(http.errorToString(httpCode));
    http.end();

    if (httpCode == 200)
    {
        Serial.println("Listening started. Waiting for result...");

        // Step 2: Poll /result every 2 sec until text is available
        String recognizedText = "";
        int attempts = 0;

        while (attempts < 10)
        { // avoid infinite loop
            delay(2000);

            HTTPClient resultHttp;
            WiFiClient client2;
            String resultUrl = String(pcIP) + ":5000/result";
            resultHttp.begin(client2, resultUrl);
            int resultCode = resultHttp.GET();

            if (resultCode == 200)
            {
                String payload = resultHttp.getString();
                Serial.println("Raw result: " + payload);

                // crude parsing — better with ArduinoJson if needed
                int idx = payload.indexOf(":\"") + 2;
                int endIdx = payload.lastIndexOf("\"");
                recognizedText = payload.substring(idx, endIdx);
                resultHttp.end();
                break;
            }

            resultHttp.end();
            attempts++;
        }

        // Step 3: Send to Smart API
        if (recognizedText.length() > 0)
        {
            Serial.print("Recognized: ");
            Serial.println(recognizedText);

            HTTPClient smartHttp;
            WiFiClient client3;
            String apiUrl = String(smartAPI) + "?query=" + urlencode(recognizedText);
            smartHttp.begin(client3, apiUrl);
            int smartCode = smartHttp.GET();

            if (smartCode == 200)
            {
                String smartRes = smartHttp.getString();
                Serial.println("Smart Response:\n" + smartRes);
            }
            else
            {
                Serial.println("Smart API failed. Code: " + String(smartCode));
            }

            smartHttp.end();
        }
        else
        {
            Serial.println("No valid speech recognized after polling.");
        }
    }
    else
    {
        Serial.println("Failed to start listening.");
    }

    Serial.println("---- Process complete. Going to idle. ----");
}

void loop()
{
    // do nothing
}
