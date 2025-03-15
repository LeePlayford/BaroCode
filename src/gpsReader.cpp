//-------------------------------------
//
//-------------------------------------
#include "gpsReader.h"
#include "ui.h"

#include <N2kTypes.h>
#include <N2kMessages.h>

#include <NMEA0183.h>

#include <sstream>
#include <iomanip>

//-------------------------------------
// Handler vars
//-------------------------------------
tBoatData* pBD;
tNMEA2000* pNMEA2000;


void HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg);

// Predefinition for functions to make it possible for constant definition for NMEA0183Handlers
void HandleRMC(const tNMEA0183Msg &NMEA0183Msg);
void HandleGGA(const tNMEA0183Msg &NMEA0183Msg);
void HandleHDT(const tNMEA0183Msg &NMEA0183Msg);
void HandleVTG(const tNMEA0183Msg &NMEA0183Msg);


tNMEA0183Handler  NMEA0183Handlers[]={
{"GGA",&HandleGGA},
{"HDT",&HandleHDT},
{"VTG",&HandleVTG},
{"RMC",&HandleRMC},
{0,0}
};


//-------------------------------------
//
//-------------------------------------
GPSReader::GPSReader (tNMEA2000 & p_NMEA2000) : m_pNMEA2000 (&p_NMEA2000)
{
    m_pBoatData = new tBoatData;
    pBD = m_pBoatData;
    pNMEA2000 = m_pNMEA2000;




}

//-------------------------------------
//
//-------------------------------------
GPSReader::~GPSReader() 
{
}

//-------------------------------------
//
//-------------------------------------
bool GPSReader::Init ()
{
    bool l_success (false);
    return l_success;
}


void GPSReader::processNMEASentence(tNMEA0183Msg& NMEA0183Msg)
{
    // Add your NMEA sentence processing code here
    // For example, you can parse the sentence and extract GPS data
    //NMEA0183Msg.


    // Process the NMEA0183 sentence
    //lv_textarea_set_text (ui_TextArea2 , NMEA0183Msg.Sentence());

    // Add your NMEA sentence processing code here
    // For example, you can parse the sentence and extract GPS data

    // Process the NMEA0183 sentence
    //lv_textarea_set_text (ui_TextArea2 , sentence);
    HandleNMEA0183Msg(NMEA0183Msg);

    char buf [20];

    lv_obj_t * screen = lv_scr_act();

    if (screen == ui_Screen1)
    {
      // update date / time and position
        // date
        std::string date = GetCurrentDate(m_pBoatData->DaysSince1970);
        lv_textarea_set_text (ui_scr1textAreaDate , date.c_str());
        // time
        std::string time = GetCurrentTime(m_pBoatData->GPSTime);
        lv_textarea_set_text (ui_scr1textAreaTime , time.c_str());

        std::string position = "No Fix Available";
        if (m_pBoatData->GPSQualityIndicator != 0 )
        {
            position  = ConvertToDegreesMinutes(m_pBoatData->Latitude , true) + "\n" + ConvertToDegreesMinutes(m_pBoatData->Longitude , false);
        }
        lv_textarea_set_text (ui_scr1textAreaPosition , position.c_str());
    }
    else if (screen == ui_Screen2)
    {
        // update the fields
        sprintf (buf , "%0.1f" , m_pBoatData->Altitude);
        lv_textarea_set_text (ui_Field2Data , buf);

        sprintf (buf , "%0.1f" , m_pBoatData->HDOP);
        lv_textarea_set_text (ui_Field3Data , buf);

        sprintf (buf , "%d" , m_pBoatData->SatelliteCount);
        lv_textarea_set_text (ui_Field4Data , buf);
    }
}

//-------------------------------------
//
//-------------------------------------
std::string GPSReader::GetCurrentDate(unsigned long DaysSince1970) const 
{
    time_t rawtime = DaysSince1970 * 86400; // Convert days to seconds
    struct tm * timeinfo = gmtime(&rawtime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d %b %Y", timeinfo);
    return std::string(buffer);
}

//-------------------------------------
//
//-------------------------------------
std::string GPSReader::GetCurrentTime(double secondsSinceMidnight) const
{
    int hours = static_cast<int>(secondsSinceMidnight / 3600);
    int minutes = static_cast<int>((secondsSinceMidnight - (hours * 3600)) / 60);
    int seconds = static_cast<int>(secondsSinceMidnight) % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;

    return oss.str();
}

//-------------------------------------
//
//-------------------------------------
std::string GPSReader::ConvertToDegreesMinutes(double value, bool isLatitude) const
{
    int degrees = static_cast<int>(value);
    double minutes = (value - degrees) * 60.0;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << degrees << "°"
        << std::fixed << std::setprecision(3) << minutes << "'"
        << (isLatitude ? (value >= 0 ? "N" : "S") : (value >= 0 ? "E" : "W"));

    return oss.str();
}



  
  // Internal variables
  Stream* NMEA0183HandlersDebugStream=0;
  
  
  
  void DebugNMEA0183Handlers(Stream* _stream) {
    NMEA0183HandlersDebugStream=_stream;
  }
  
  tN2kGNSSmethod GNSMethofNMEA0183ToN2k(int Method) {
    switch (Method) {
      case 0: return N2kGNSSm_noGNSS;
      case 1: return N2kGNSSm_GNSSfix;
      case 2: return N2kGNSSm_DGNSS;
      default: return N2kGNSSm_noGNSS;  
    }
  }
  
  void GPSReader::HandleNMEA0183Msg(const tNMEA0183Msg &NMEA0183Msg) {
    int iHandler;
    // Find handler
    for (iHandler=0; NMEA0183Handlers[iHandler].Code!=0 && !NMEA0183Msg.IsMessageCode(NMEA0183Handlers[iHandler].Code); iHandler++);
    if (NMEA0183Handlers[iHandler].Code!=0) {
      NMEA0183Handlers[iHandler].Handler(NMEA0183Msg); 
    }
  }
  
  // NMEA0183 message Handler functions
  
  void HandleRMC(const tNMEA0183Msg &NMEA0183Msg) {
    if (pBD==0) return;
    
    if (NMEA0183ParseRMC_nc(NMEA0183Msg,pBD->GPSTime,pBD->Latitude,pBD->Longitude,pBD->COG,pBD->SOG,pBD->DaysSince1970,pBD->Variation)) {
    } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse RMC"); }
  }
  
  void HandleGGA(const tNMEA0183Msg &NMEA0183Msg) {
    if (pBD==0) return;
    
    if (NMEA0183ParseGGA_nc(NMEA0183Msg,pBD->GPSTime,pBD->Latitude,pBD->Longitude,
                     pBD->GPSQualityIndicator,pBD->SatelliteCount,pBD->HDOP,pBD->Altitude,pBD->GeoidalSeparation,
                     pBD->DGPSAge,pBD->DGPSReferenceStationID)) {
      if (pNMEA2000!=0) {
        tN2kMsg N2kMsg;
        SetN2kGNSS(N2kMsg,1,pBD->DaysSince1970,pBD->GPSTime,pBD->Latitude,pBD->Longitude,pBD->Altitude,
                  N2kGNSSt_GPS,GNSMethofNMEA0183ToN2k(pBD->GPSQualityIndicator),pBD->SatelliteCount,pBD->HDOP,0,
                  pBD->GeoidalSeparation,1,N2kGNSSt_GPS,pBD->DGPSReferenceStationID,pBD->DGPSAge
                  );
        pNMEA2000->SendMsg(N2kMsg); 
      }
  
      if (NMEA0183HandlersDebugStream!=0) {
        NMEA0183HandlersDebugStream->print("Time="); NMEA0183HandlersDebugStream->println(pBD->GPSTime);
        NMEA0183HandlersDebugStream->print("Latitude="); NMEA0183HandlersDebugStream->println(pBD->Latitude,5);
        NMEA0183HandlersDebugStream->print("Longitude="); NMEA0183HandlersDebugStream->println(pBD->Longitude,5);
        NMEA0183HandlersDebugStream->print("Altitude="); NMEA0183HandlersDebugStream->println(pBD->Altitude,1);
        NMEA0183HandlersDebugStream->print("GPSQualityIndicator="); NMEA0183HandlersDebugStream->println(pBD->GPSQualityIndicator);
        NMEA0183HandlersDebugStream->print("SatelliteCount="); NMEA0183HandlersDebugStream->println(pBD->SatelliteCount);
        NMEA0183HandlersDebugStream->print("HDOP="); NMEA0183HandlersDebugStream->println(pBD->HDOP);
        NMEA0183HandlersDebugStream->print("GeoidalSeparation="); NMEA0183HandlersDebugStream->println(pBD->GeoidalSeparation);
        NMEA0183HandlersDebugStream->print("DGPSAge="); NMEA0183HandlersDebugStream->println(pBD->DGPSAge);
        NMEA0183HandlersDebugStream->print("DGPSReferenceStationID="); NMEA0183HandlersDebugStream->println(pBD->DGPSReferenceStationID);
      }
    } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse GGA"); }
  }
  
  #define PI_2 6.283185307179586476925286766559
  
  void HandleHDT(const tNMEA0183Msg &NMEA0183Msg) {
    if (pBD==0) return;
    
    if (NMEA0183ParseHDT_nc(NMEA0183Msg,pBD->TrueHeading)) {
      if (pNMEA2000!=0) { 
        tN2kMsg N2kMsg;
        double MHeading=pBD->TrueHeading-pBD->Variation;
        while (MHeading<0) MHeading+=PI_2;
        while (MHeading>=PI_2) MHeading-=PI_2;
        // Stupid Raymarine can not use true heading
        SetN2kMagneticHeading(N2kMsg,1,MHeading,0,pBD->Variation);
  //      SetN2kPGNTrueHeading(N2kMsg,1,pBD->TrueHeading);
        pNMEA2000->SendMsg(N2kMsg);
      }
      if (NMEA0183HandlersDebugStream!=0) {
        NMEA0183HandlersDebugStream->print("True heading="); NMEA0183HandlersDebugStream->println(pBD->TrueHeading);
      }
    } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse HDT"); }
  }
  
  void HandleVTG(const tNMEA0183Msg &NMEA0183Msg) {
   double MagneticCOG;
    if (pBD==0) return;
    
    if (NMEA0183ParseVTG_nc(NMEA0183Msg,pBD->COG,MagneticCOG,pBD->SOG)) {
        pBD->Variation=pBD->COG-MagneticCOG; // Save variation for Magnetic heading
      if (pNMEA2000!=0) { 
        tN2kMsg N2kMsg;
        SetN2kCOGSOGRapid(N2kMsg,1,N2khr_true,pBD->COG,pBD->SOG);
        pNMEA2000->SendMsg(N2kMsg);
  //      SetN2kBoatSpeed(N2kMsg,1,SOG);
  //      NMEA2000.SendMsg(N2kMsg);
      }
      if (NMEA0183HandlersDebugStream!=0) {
        NMEA0183HandlersDebugStream->print("True heading="); NMEA0183HandlersDebugStream->println(pBD->TrueHeading);
      }
    } else if (NMEA0183HandlersDebugStream!=0) { NMEA0183HandlersDebugStream->println("Failed to parse VTG"); }
  }
  
  
  
    




