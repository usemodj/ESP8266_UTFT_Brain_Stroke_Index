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

#pragma once

#include <JsonListener.h>
#include <JsonStreamingParser.h>

#include <math.h>
#include <TimeLib.h>

typedef struct {
  String date; //발표시간
  int today; //오늘예측값(뇌졸중가능지수: 없음, 0:LOW, 1:NORMAL, 2:HIGH, 3:VERY HIGH)
  int tomorrow; //내일예측값
  int dayAfterTomorrow; //모레예측값
} BrainStrokeIndex; //뇌졸중가능지수


class BrainStrokeIndexJsonClient : public JsonListener {
  private:
    // brain stroke index
    boolean isStrokeIndex = false;
    String successYN; //'Y' | 'N'
    String errMsg;
    String date;
    String today;
    String tomorrow;
    String dayAfterTomorrow;
    String currentKey;
    String currentParent;
    boolean isArray = false;
    
    BrainStrokeIndex strokeIndex;
       
  protected:
    void doUpdate(String url);

  public:

    BrainStrokeIndexJsonClient();
    virtual ~BrainStrokeIndexJsonClient() {}

    inline BrainStrokeIndex getBrainStrokeIndex(){
      return strokeIndex;
    }

    inline String getStrokeIndexToday(){
      return getBrainStrokeIndexText(strokeIndex.today);
    }
    inline String getStrokeIndexTomorrow(){
      return getBrainStrokeIndexText(strokeIndex.tomorrow);
    }
    inline String getStrokeIndexDayAfterTomorrow(){
      return getBrainStrokeIndexText(strokeIndex.dayAfterTomorrow);
    }
    
    void updateBrainStrokeIndex(String serviceKey, String areaNo);
    
    inline String getBrainStrokeIndexText(int index){
      String text;
      switch(index){
        case 0:
          text = "LOW";
          break;
        case 1:
          text = "NORMAL";
          break;
        case 2:
          text = "HIGH";
          break;
        case 3:
          text = "VERY HIGH";
          break;
        default:
          text = "--";
          break;
      }
      
      return text;
    }

    inline time_t MakeTime(const char* dateTime){
      return MakeTime(String(dateTime));
    }
    
    inline time_t MakeTime(const String dateTime){
      //date: 2016060706 (yyyyMMddHH)
      tmElements_t tm;
      tm.Year = CalendarYrToTm(dateTime.substring(0,4).toInt());
      tm.Month = dateTime.substring(4, 6).toInt();
      tm.Day = dateTime.substring(6, 8).toInt();
      tm.Hour = dateTime.substring(8).toInt();
      return makeTime(tm);
    }
    
    inline boolean IsNumeric(String str) {
        for(char i = 0; i < str.length(); i++) {
            if ( !(isDigit(str.charAt(i)) || str.charAt(i) == '.' )) {
                return false;
            }
        }
        return true;
    }

    virtual void whitespace(char c);

    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
    
    String urlEncode(String str);
    String urlDecode(String str);
    unsigned char h2int(char c);
   
};


