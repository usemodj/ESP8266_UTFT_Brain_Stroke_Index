## 보건기상지수
www.data.go.kr OPEN API 
기후변화에 능동적으로 신체를 대처할수 있는 보건기상지수(뇌졸중가능지수, 피부질환가능지수, 감기가능지수, 꽃가루농도위험지수 등 5종)를 특정위치(지점번호)별로 조회할 수 있는 서비스 입니다. 
 

## Required libraries

Unzip or git clone the libraries into ‘~/Arduino/libraries/’ directory and restart Arduino IDE

- XPT2046: https://github.com/spapadim/XPT2046
- ESP8266_UTFT: https://github.com/usemodj/ESP8266_UTFT


Modify: XPT2046.cpp
```
 void XPT2046::begin(uint16_t width, uint16_t height) {  
    ... 
    //SPI.begin(); //comment out 
    ... 
 }  
 ```

See the connection between ESP8266 board and 2.8 inch 240x320 Touch TFT LCD pins
and more at www.usemodj.com 

[Display 2.8 inch Touch LCD with WeMos D1 mini board](http://usemodj.com/2016/03/21/esp8266-display-2-8-inch-touch-lcd-with-wemos-d1-mini-board/)
