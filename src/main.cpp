#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include <WiFiClientSecure.h>
#include "HTTPClient.h"
#include <ArduinoJson.h>
#include <Preferences.h>


#define SSID "RouteurCadeau"
#define PASSWD "CadeauRouteur"
#define URL "https://guardia-api.iadjedj.ovh/"  // 
#define RST_PIN         D6          // Configurable, see typical pin layout above
#define SS_PIN          D4       // Configurable, see typical pin layout above
#define GLED            D2
#define RLED            D1

const char* server_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDwjCCA0igAwIBAgISBTuWfib9kK2yVwJjhzFfxodtMAoGCCqGSM49BAMDMDIxCzAJBgNVBAYT\n" \
"AlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJFNjAeFw0yNTAzMjQxNDQ2MTJa\n" \
"Fw0yNTA2MjIxNDQ2MTFaMBYxFDASBgNVBAMTC2lhZGplZGoub3ZoMFkwEwYHKoZIzj0CAQYIKoZI\n" \
"zj0DAQcDQgAE3ymXSkKjbYqyCLoDH/3N9/hAeawe3+smejpg0x03i01MPBSVpLDILJHZ3WGXoxhS\n" \
"MqmZ4qQTCmxNqjexyAcytqOCAlgwggJUMA4GA1UdDwEB/wQEAwIHgDAdBgNVHSUEFjAUBggrBgEF\n" \
"BQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUMynPzLSa2xxyAHCch4gfff6s\n" \
"uggwHwYDVR0jBBgwFoAUkydGmAOpUWiOmNbEQkjbI79YlNIwVQYIKwYBBQUHAQEESTBHMCEGCCsG\n" \
"AQUFBzABhhVodHRwOi8vZTYuby5sZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9lNi5pLmxl\n" \
"bmNyLm9yZy8wLwYDVR0RBCgwJoIXZ3VhcmRpYS1hcGkuaWFkamVkai5vdmiCC2lhZGplZGoub3Zo\n" \
"MBMGA1UdIAQMMAowCAYGZ4EMAQIBMC4GA1UdHwQnMCUwI6AhoB+GHWh0dHA6Ly9lNi5jLmxlbmNy\n" \
"Lm9yZy8xMTAuY3JsMIIBBgYKKwYBBAHWeQIEAgSB9wSB9ADyAHcAzPsPaoVxCWX+lZtTzumyfCLp\n" \
"hVwNl422qX5UwP5MDbAAAAGVyNPbjAAABAMASDBGAiEAuRVi2rwLShXZE6mRVHF+Vr9DODoLO52R\n" \
"qe8tdQL//NMCIQC45w+WrH6tJ0px9ujsxF2WhUuj+FHK+GgvgeIs4nbAMAB3AE51oydcmhDDOFts\n" \
"1N8/Uusd8OCOG41pwLH6ZLFimjnfAAABlcjT22wAAAQDAEgwRgIhAIcQsy0omiWcBdwCSZrX97uB\n" \
"ZF+Fknf6DZOtXprC4+xqAiEA3IIwPR4n71lwhxXZPA5+xAyR9gqqhbsdgE1cJATA+UkwCgYIKoZI\n" \
"zj0EAwMDaAAwZQIxAM+/pKVrJF3MLwUzPp462e3eGygNtDtjTrI0EVKziIHmc9NskPY1jmBm9qbd\n" \
"rsjVSgIwNvqbQoObPp5BZDgDFDTxsyCdJUhCQsMV1zRE/YXutDo5hRxuRfP3PtqGdkNI583z\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEVzCCAj+gAwIBAgIRALBXPpFzlydw27SHyzpFKzgwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UE\n" \
"BhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQD\n" \
"EwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAwWhcNMjcwMzEyMjM1OTU5WjAyMQswCQYDVQQG\n" \
"EwJVUzEWMBQGA1UEChMNTGV0J3MgRW5jcnlwdDELMAkGA1UEAxMCRTYwdjAQBgcqhkjOPQIBBgUr\n" \
"gQQAIgNiAATZ8Z5Gh/ghcWCoJuuj+rnq2h25EqfUJtlRFLFhfHWWvyILOR/VvtEKRqotPEoJhC6+\n" \
"QJVV6RlAN2Z17TJOdwRJ+HB7wxjnzvdxEP6sdNgA1O1tHHMWMxCcOrLqbGL0vbijgfgwgfUwDgYD\n" \
"VR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggrBgEFBQcDATASBgNVHRMBAf8ECDAG\n" \
"AQH/AgEAMB0GA1UdDgQWBBSTJ0aYA6lRaI6Y1sRCSNsjv1iU0jAfBgNVHSMEGDAWgBR5tFnme7bl\n" \
"5AFzgAiIyBpY9umbbjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly94MS5pLmxl\n" \
"bmNyLm9yZy8wEwYDVR0gBAwwCjAIBgZngQwBAgEwJwYDVR0fBCAwHjAcoBqgGIYWaHR0cDovL3gx\n" \
"LmMubGVuY3Iub3JnLzANBgkqhkiG9w0BAQsFAAOCAgEAfYt7SiA1sgWGCIpunk46r4AExIRcMxkK\n" \
"gUhNlrrv1B21hOaXN/5miE+LOTbrcmU/M9yvC6MVY730GNFoL8IhJ8j8vrOLpMY22OP6baS1k9YM\n" \
"rtDTlwJHoGby04ThTUeBDksS9RiuHvicZqBedQdIF65pZuhpeDcGBcLiYasQr/EO5gxxtLyTmgsH\n" \
"SOVSBcFOn9lgv7LECPq9i7mfH3mpxgrRKSxHpOoZ0KXMcB+hHuvlklHntvcI0mMMQ0mhYj6qtMFS\n" \
"tkF1RpCG3IPdIwpVCQqu8GV7s8ubknRzs+3C/Bm19RFOoiPpDkwvyNfvmQ14XkyqqKK5oZ8zhD32\n" \
"kFRQkxa8uZSuh4aTImFxknu39waBxIRXE4jKxlAmQc4QjFZoq1KmQqQg0J/1JF8RlFvJas1VcjLv\n" \
"YlvUB2t6npO6oQjB3l+PNf0DpQH7iUx3Wz5AjQCi6L25FjyE06q6BZ/QlmtYdl/8ZYao4SRqPEs/\n" \
"6cAiF+Qf5zg2UkaWtDphl1LKMuTNLotvsX99HP69V2faNyegodQ0LyTApr/vT01YPE46vNsDLgK+\n" \
"4cL6TrzC/a4WcmF5SRJ938zrv/duJHLXQIku5v0+EwOy59Hdm0PT/Er/84dDV0CSjdR/2XuZM3kp\n" \
"ysSKLgD1cKiDA+IRguODCxfO9cyYIg46v9mFmBvyH04=\n" \
"-----END CERTIFICATE-----\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UE\n" \
"BhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQD\n" \
"EwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQG\n" \
"EwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMT\n" \
"DElTUkcgUm9vdCBYMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54r\n" \
"Vygch77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+0TM8ukj1\n" \
"3Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6UA5/TR5d8mUgjU+g4rk8K\n" \
"b4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sWT8KOEUt+zwvo/7V3LvSye0rgTBIlDHCN\n" \
"Aymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyHB5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ\n" \
"4Q7e2RCOFvu396j3x+UCB5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf\n" \
"1b0SHzUvKBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWnOlFu\n" \
"hjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTnjh8BCNAw1FtxNrQH\n" \
"usEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbwqHyGO0aoSCqI3Haadr8faqU9GY/r\n" \
"OPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CIrU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4G\n" \
"A1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY\n" \
"9umbbjANBgkqhkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ3BebYhtF8GaV\n" \
"0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KKNFtY2PwByVS5uCbMiogziUwt\n" \
"hDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJw\n" \
"TdwJx4nLCgdNbOhdjsnvzqvHu7UrTkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nx\n" \
"e5AW0wdeRlN8NwdCjNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZA\n" \
"JzVcoyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq4RgqsahD\n" \
"YVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPAmRGunUHBcnWEvgJBQl9n\n" \
"JEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57demyPxgcYxn/eR44/KJ4EBs+lVDR3veyJ\n" \
"m+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
"-----END CERTIFICATE-----\n";


MFRC522 mfrc522(SS_PIN, RST_PIN); // Creation de l'instance MFRC522
Preferences preferences;

bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;
String token;

String getUIDDecimal(MFRC522 &mfrc522) {
  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidString += String(mfrc522.uid.uidByte[i], DEC);
  }
  return uidString;
}

void Blink(int count, int led)
{
  int i = 0;
  while (i < count)
  {
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
    i++;
  }
}

void blinkgreen(int count)
{
  Blink(count, GLED);
}

void blinkred(int count)
{
  Blink(count, RLED);
}

void api(String fin_url, String id) {
  WiFiClientSecure client;
  client.setCACert(server_cert);  // Certificat du serveur

  HTTPClient http;
  String resp;
  String full_url = URL + fin_url + id;

  Serial.println("Tentative de connexion à : " + full_url);

  if (!http.begin(client, full_url)) {  
    Serial.println("❌ Échec de la connexion HTTPS !");
    return;
  }
  
  Serial.println("✅ Connexion HTTPS établie !");

  // Ajout du JWT Token dans l'en-tête
  http.addHeader("Authorization", String("Bearer ") + token);
  http.addHeader("Content-Type", "application/json");

  int code = http.GET();  // Envoi de la requête GET

  if (code == HTTP_CODE_OK) {
    resp = http.getString();
    Serial.println("Réponse reçue : " + resp);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, resp);

    if (error) {
      Serial.print("❌ Erreur JSON : ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    const char* level = doc["level"];  // "unauthorized", "user", "admin"
    http.end();  // connexion fermée

    if (level && (strcmp(level, "user") == 0 || strcmp(level, "admin") == 0)) {
      Serial.println("✅ Passage autorisé !");
      blinkgreen(3);
    } else {
      Serial.println("❌ Accès refusé : " + String(level));
      blinkred(3);
    }

  } else {
    Serial.print("❌ Erreur HTTP : ");
    Serial.println(code);
    blinkred(3);
  }

  http.end(); // connexion fermée
}


void setup()
{
  Serial.begin(115200);
  //NVS
  preferences.begin("myApp", false);
  token = preferences.getString("token", "pas_de_token");

  Serial.println("🔑 Token récupéré : " + token);
  //preferences.putString("token", JWT_TOKEN);
  preferences.end();

  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init(); // Init MFRC522
  pinMode(GLED, OUTPUT);
  pinMode(RLED, OUTPUT);

  WiFi.begin(SSID, PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("WiFi Error");
  }
  Serial.println("WiFi connecté !");
}

void loop()
{
  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if (_rfid_error_counter > 2)
  {
    _tag_found = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522.PCD_WriteRegister(mfrc522.TxModeReg, 0x00);
  mfrc522.PCD_WriteRegister(mfrc522.RxModeReg, 0x00);
  // Reset ModWidthReg
  mfrc522.PCD_WriteRegister(mfrc522.ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfrc522.PICC_RequestA(bufferATQA, &bufferSize);

  if (result == mfrc522.STATUS_OK)
  {
    if (!mfrc522.PICC_ReadCardSerial())
    { // Since a PICC placed get Serial and continue
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;
  }

  rfid_tag_present = _tag_found;

  // si badge detectée
  if (rfid_tag_present && !rfid_tag_present_prev)
  {
    Serial.println("Tag found");
    String uid = getUIDDecimal(mfrc522);
  //  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    Serial.println(uid);
    api("check_badge?id=", uid);
  }

  // si badge n'est plus detectée
  if (!rfid_tag_present && rfid_tag_present_prev)
  {
    Serial.println("Tag gone");
  }
  delay(100);
}