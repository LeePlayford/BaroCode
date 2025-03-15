
//-------------------------------------------------
//
// Project BaroGraph
// GPS Reader Header file
//
//
//
//-------------------------------------------------


//--------------------------------------
//
//--------------------------------------
#include "Arduino.h"
#include <string>

#include <NMEA0183.h>
#include <NMEA0183Msg.h>
#include <NMEA0183Messages.h>

#include <NMEA2000.h>

#include "BoatData.h"



#define GPS_BAUD 9600
#define GPS_MAX_SENTENCE 100


struct tNMEA0183Handler {
    const char *Code;
    void (*Handler)(const tNMEA0183Msg &NMEA0183Msg); 
  };


//-------------------------------------
//
//-------------------------------------
class GPSReader
{
    public:
        GPSReader (tNMEA2000 & NMEA2000);
        ~GPSReader();

        bool Init ();
        void processNMEASentence(tNMEA0183Msg& NMEA0183Msg);



    private:

    void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg);
    void InitNMEA0183Handlers(tNMEA2000 *_NMEA2000, tBoatData *_BoatData);
    std::string GetCurrentDate(unsigned long DaysSince1970) const ;
    std::string GetCurrentTime(double secondsSinceMidnight) const;
    std::string ConvertToDegreesMinutes(double value, bool isLatitude) const;

    tNMEA2000 * m_pNMEA2000;
    tBoatData * m_pBoatData;



};