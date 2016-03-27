/**The MIT License (MIT)

Copyright (c) 2016 by Seokjin Seo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://usemodj.com
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include "BrainStrokeIndexJsonClient.h"

BrainStrokeIndexJsonClient::BrainStrokeIndexJsonClient(){
  
}

/* 
 *  www.data.go.kr
 *    OPEN API: (신)동네예보정보조회서비스
 * http://newsky2.kma.go.kr/service/SecndSrtpdFrcstInfoService2/ForecastSpaceData?ServiceKey=SERVICE_KEY&base_date=20151021&base_time=0230&nx=1&ny=1
 */

void BrainStrokeIndexJsonClient::updateBrainStrokeIndex(String serviceKey, String areaNo) {
  isStrokeIndex = true;
  int pageNo = 1;
  String url = "/iros/RetrieveWhoIndexService/getBrainWhoList?_type=json&ServiceKey="
              + serviceKey + "&AreaNo=" + areaNo;

  doUpdate(url);
}

void BrainStrokeIndexJsonClient::doUpdate(String url) {
  JsonStreamingParser parser;
  parser.setListener(this);
  
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("203.247.66.146", httpPort)) {
    Serial.println("Url connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: 203.247.66.146\r\n" +
               "Connection: close\r\n\r\n");
  while(!client.available()) {
    delay(1000);
  }

  int pos = 0;
  boolean isBody = false;
  char c;

  int size = 0;
  client.setNoDelay(false);
  while(client.connected()) {
    while((size = client.available()) > 0) {
      c = client.read();
      if (c == '{' || c == '[') {
        isBody = true;
      }
      if (isBody) {
        parser.parse(c);
      }
    }
  }
}

void BrainStrokeIndexJsonClient::whitespace(char c) {
  Serial.println("whitespace");
}

void BrainStrokeIndexJsonClient::startDocument() {
  Serial.println("start document");
}

void BrainStrokeIndexJsonClient::key(String key) {
  currentKey = String(key);
  Serial.println(">> Key: " + key);
}

void BrainStrokeIndexJsonClient::value(String value) {
  Serial.println(">> Value: " + value);
  if (currentKey.equals("SuccessYN")) {
    successYN = value;
  }
  else if (currentKey.equals("ErrMsg")) {
    errMsg = value;
  }
  else if (currentKey.equals("date")) {
    date = String(value);
  }
  if (currentKey.equals("today")) {
    today = String(value);
  }
  if (currentKey.equals("tomorrow")) {
    tomorrow = String(value);
  }
  if (currentKey.equals("theDayAfterTomorrow")) {
    dayAfterTomorrow = String(value);
  }
}

void BrainStrokeIndexJsonClient::startArray() {
  Serial.println("Start Array...");
  isArray = true;
  currentParent = currentKey;
}

void BrainStrokeIndexJsonClient::endArray() {
  isArray = false;
  
}


void BrainStrokeIndexJsonClient::startObject() {
  if(!isArray) currentParent = currentKey;
  Serial.println(">>Start Object ... parent: "+ currentParent);
}

void BrainStrokeIndexJsonClient::endObject() {
    //Serial.println(">>endObject... currentParent: " + currentParent);
    if(String("IndexModel").equals(currentParent)){
      strokeIndex.date = date;
      strokeIndex.today = IsNumeric(today)? today.toInt(): -1;
      strokeIndex.tomorrow = IsNumeric(tomorrow)? tomorrow.toInt(): -1;
      strokeIndex.dayAfterTomorrow = IsNumeric(dayAfterTomorrow)? dayAfterTomorrow.toInt(): -1;
  }
          
    if(!isArray) currentParent = "";
}

void BrainStrokeIndexJsonClient::endDocument() {

}


String BrainStrokeIndexJsonClient::urlEncode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
    
}

String BrainStrokeIndexJsonClient::urlDecode(String str)
{
    
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
      yield();
    }
    
   return encodedString;
}

unsigned char BrainStrokeIndexJsonClient::h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

