#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *ssid = "Tissuepaper";
const char *password = "Dontknow";

const char *pcIP = "http://192.168.1.28";
const char *smartAPI = "http://192.168.1.28:2001/api/smart";

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

// Display Weather
void displayWeather(String response)
{
    int cityStart = response.indexOf("\"city\":\"") + 8;
    int cityEnd = response.indexOf("\"", cityStart);
    String city = response.substring(cityStart, cityEnd);

    int descStart = response.indexOf("\"description\":\"", cityEnd) + 15;
    int descEnd = response.indexOf("\"", descStart);
    String desc = response.substring(descStart, descEnd);

    int tempStart = response.indexOf("\"temperature\":", descEnd) + 14;
    int tempEnd = response.indexOf(",", tempStart);
    String temp = response.substring(tempStart, tempEnd);

    int humStart = response.indexOf("\"humidity\":", tempEnd) + 11;
    int humEnd = response.indexOf(",", humStart);
    String humidity = response.substring(humStart, humEnd);

    int windStart = response.indexOf("\"windSpeed\":", humEnd) + 12;
    int windEnd = response.indexOf("}", windStart);
    String wind = response.substring(windStart, windEnd);

    Serial.println("📍 Weather Report");
    Serial.println("City       : " + city);
    Serial.println("Condition  : " + desc);
    Serial.println("Temperature: " + temp + "°C");
    Serial.println("Humidity   : " + humidity + "%");
    Serial.println("Wind Speed : " + wind + " m/s");
    Serial.println("----");
}

// Display News
void displayNews(String smartResponse)
{
    int articlesStartIdx = smartResponse.indexOf("\"articles\":[") + 12;
    int articlesEndIdx = smartResponse.indexOf("]", articlesStartIdx);
    if (articlesStartIdx == -1 || articlesEndIdx == -1)
    {
        Serial.println("No articles found in response.");
        return;
    }

    String articlesSection = smartResponse.substring(articlesStartIdx, articlesEndIdx);
    int articleStartIdx = 0;

    while ((articleStartIdx = articlesSection.indexOf("{", articleStartIdx)) != -1)
    {
        int titleStartIdx = articlesSection.indexOf("\"title\":\"", articleStartIdx);
        int titleEndIdx = articlesSection.indexOf("\"", titleStartIdx + 9);
        String title = articlesSection.substring(titleStartIdx + 9, titleEndIdx);

        int urlStartIdx = articlesSection.indexOf("\"url\":\"", titleEndIdx);
        int urlEndIdx = articlesSection.indexOf("\"", urlStartIdx + 7);
        String url = articlesSection.substring(urlStartIdx + 7, urlEndIdx);

        int descriptionStartIdx = articlesSection.indexOf("\"description\":\"", urlEndIdx);
        int descriptionEndIdx = articlesSection.indexOf("\"", descriptionStartIdx + 15);
        String description = articlesSection.substring(descriptionStartIdx + 15, descriptionEndIdx);

        Serial.println("📰 Title: " + title);
        Serial.println("🔗 URL  : " + url);
        Serial.println("📄 Description: " + description);
        Serial.println("----");

        articleStartIdx = articlesSection.indexOf("}", descriptionEndIdx);
        if (articleStartIdx == -1)
            break;
        articleStartIdx++;
    }
}

// Display Mails
void displayMails(String response)
{
    int messagesStart = response.indexOf("\"messages\":[") + 12;
    int messagesEnd = response.indexOf("]", messagesStart);
    if (messagesStart == -1 || messagesEnd == -1)
    {
        Serial.println("No mail data found.");
        return;
    }

    String messagesSection = response.substring(messagesStart, messagesEnd);
    int msgIdx = 0;
    int count = 1;

    while ((msgIdx = messagesSection.indexOf("{", msgIdx)) != -1)
    {
        int snippetStart = messagesSection.indexOf("\"snippet\":\"", msgIdx);
        if (snippetStart == -1)
            break;
        snippetStart += 11;
        int snippetEnd = messagesSection.indexOf("\"", snippetStart);
        if (snippetEnd == -1)
            break;

        String snippet = messagesSection.substring(snippetStart, snippetEnd);

        Serial.print("📬 Mail ");
        Serial.print(count++);
        Serial.println(":");
        Serial.println(snippet);
        Serial.println("----");

        msgIdx = messagesSection.indexOf("}", snippetEnd);
        if (msgIdx == -1)
            break;
        msgIdx++;
    }
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

    HTTPClient http;
    WiFiClient client1;
    String startUrl = String(pcIP) + ":5000/start-listening";
    Serial.println("Triggering speech recognition...");
    http.begin(client1, startUrl);
    http.setTimeout(15000);
    int httpCode = http.GET();
    Serial.print("HTTP GET Code: ");
    Serial.println(httpCode);
    Serial.println(http.getString());
    Serial.println(http.errorToString(httpCode));
    http.end();

    if (httpCode == 200)
    {
        Serial.println("Listening started. Waiting for result...");
        String recognizedText = "";
        int attempts = 0;

        while (attempts < 10)
        {
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
                int idx = payload.indexOf(":\"") + 2;
                int endIdx = payload.lastIndexOf("\"");
                recognizedText = payload.substring(idx, endIdx);
                resultHttp.end();
                break;
            }

            resultHttp.end();
            attempts++;
        }

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

                if (smartRes.indexOf("\"action\":\"weather\"") != -1)
                {
                    displayWeather(smartRes);
                }
                else if (smartRes.indexOf("\"action\":\"news\"") != -1)
                {
                    displayNews(smartRes);
                }
                else if (smartRes.indexOf("\"action\":\"email\"") != -1)
                {
                    displayMails(smartRes);
                }
                else
                {
                    Serial.println("Unrecognized response format.");
                }
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
