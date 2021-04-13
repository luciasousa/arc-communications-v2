#include <iostream>
#include <stdlib.h>
#include <IPv4Layer.h>
#include <Packet.h>
#include <PcapFileDevice.h>
#include "EthLayer.h"
#include "PcapLiveDeviceList.h"
#include "HttpLayer.h"
#include "PlatformSpecificUtils.h"
#include <UdpLayer.h>
#include <PayloadLayer.h>
#include "PcapFileDevice.h"
#include "in.h"
#include "GeneralUtils.h"
#include <sstream>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <boost/circular_buffer.hpp>
#include <math.h> 
#include <time.h>
#include<json.h>
#include <curl/curl.h>
#include <fstream>
#include <chrono>
#include <tuple>





int self_station;
std::string endpoint;

using namespace std;



struct cam{
  int station_id;
  int station_type;
  int latitude;
  int longitude;
  int semiMajorAxisConfidence;
  int semiMinorAxisConfidence;
  int semiMajorOrientation;
  int altitude;
  int heading;
  int headingConfidence;
  int speed;
  int speedConfidence;
  int vehicleLength;
  int vehicleWidth;
  int longitudinalAcceleration;
  int longitudinalAccelerationConfidence;
  int yawRate;
  int yawRateConfidence;
  int vehicleRole;
  int persons;
  int airbags;
  int abs;
  int overturned;
  int hazard_lights;
  int all_seatbelts;
  int accident;
  int temperature; 
};

int gateway_selfSelection(std::unordered_map<int, boost::circular_buffer<cam>> storage, int accidented_id);
void send_to_api(boost::circular_buffer<cam> information);

struct PacketStats
{
  std::unordered_map<int, boost::circular_buffer<cam> > storage;
  std::unordered_map<int, time_t> blacklist;

    void parsePacket(pcpp::Packet& packet)
    {
        
      
        pcpp::PayloadLayer* payloadLayer = packet.getLayerOfType<pcpp::PayloadLayer>();
        if (payloadLayer == NULL)
        {
            printf("Something went wrong, couldn't find Payload layer\n");
            
        }
        
        uint8_t mensagem[payloadLayer->getPayloadLen()];
        


        for(int i = 0; i < payloadLayer->getPayloadLen(); i++){
          
          mensagem[i] = *(payloadLayer->getPayload()+i); //mensagem holds the payload
        }

        //harcoded parsing
        

        cam message;  
        string payload = pcpp::byteArrayToHexString(mensagem, sizeof(mensagem));
        try{
          message.station_id = stoul(payload.substr(2,8), nullptr, 16);
          message.station_type = stoul(payload.substr(20,8), nullptr, 16);
          message.latitude = stoul(payload.substr(28,8), nullptr, 16);
          message.longitude = stoul(payload.substr(36,8), nullptr, 16);
          message.semiMajorAxisConfidence = stoul(payload.substr(44,8), nullptr, 16);
          message.semiMinorAxisConfidence = stoul(payload.substr(52,8), nullptr, 16);
          message.semiMajorOrientation = stoul(payload.substr(60,8), nullptr, 16);
          message.altitude = stoul(payload.substr(68,8), nullptr, 16);
          message.heading = stoul(payload.substr(76,8), nullptr, 16);
          message.headingConfidence = stoul(payload.substr(84,8), nullptr, 16);
          message.speed = stoul(payload.substr(92,8), nullptr, 16);
          message.speedConfidence = stoul(payload.substr(100,8), nullptr, 16);
          message.vehicleLength = stoul(payload.substr(108,8), nullptr, 16);
          message.vehicleWidth = stoul(payload.substr(116,8), nullptr, 16);
          message.longitudinalAcceleration = stoul(payload.substr(124,8), nullptr, 16);
          message.longitudinalAccelerationConfidence = stoul(payload.substr(132,8), nullptr, 16);
          message.yawRate = stoul(payload.substr(140,8), nullptr, 16);
          message.yawRateConfidence = stoul(payload.substr(148,8), nullptr, 16);
          message.vehicleRole = stoul(payload.substr(156,8), nullptr, 16);
          message.persons = stoul(payload.substr(164,8), nullptr, 16);
          message.airbags = stoul(payload.substr(172,8), nullptr, 16);
          message.abs = stoul(payload.substr(180,2), nullptr, 16);
          message.overturned = stoul(payload.substr(182,2), nullptr, 16);
          message.hazard_lights = stoul(payload.substr(184,2), nullptr, 16);
          message.all_seatbelts = stoul(payload.substr(186,2), nullptr, 16);
          message.accident = stoul(payload.substr(188,2), nullptr, 16);
          message.temperature = stoul(payload.substr(190,8), nullptr, 16);
        }catch(...){
          cout << "Not a cam" << endl;
          return;
        }


        auto got = storage.find(message.station_id);

        if ( got == storage.end() ){
          cout << "not found" << endl;
          boost::circular_buffer<cam> cam_storage{100};
          cam_storage.push_back(message);
          storage[message.station_id] = cam_storage;
        }else{ // existe esta station_id
          boost::circular_buffer<cam> temp_storage = got->second;
          temp_storage.push_back(message);
          storage[message.station_id] = temp_storage;
        }

        /*
        cout << message.station_id << endl;
        float acc_lat = message.latitude / (pow(10.0,7.0));
        float acc_lon = message.longitude / (pow(10.0,7.0));
        cout << acc_lat << endl;
        cout << acc_lon << endl;
        */


        if(message.accident){
          
          time_t current = time(NULL);
          //cout << "current time is  " << current << endl;
          //cout << "station id " << message.station_id << endl;
          auto iterator = blacklist.find(message.station_id);
          if (iterator == blacklist.end()){
            cout << "This station id IS NOT blacklisted." << endl;
            blacklist[message.station_id] = current;
            if(gateway_selfSelection(storage, message.station_id)){
              cout << "i'm in danger" << endl;
              send_to_api(storage[message.station_id]);
            }
          
        }else{
          //cout << "This station id IS blacklisted" << endl;
          time_t last_value = blacklist[message.station_id];
          //cout << "Last Value  " << last_value << endl;
          if((current - last_value)>60){
            cout << "Turns out you should be complaining. You must drive really bad" << endl;
            blacklist[message.station_id] = current;
            if(gateway_selfSelection(storage, message.station_id)){
              cout << "i'm in danger" << endl;
              send_to_api(storage[message.station_id]);
            }
          }
        }
      }  
    }
  };

int send_information(const char *buffer){
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
    
        /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
        itself */ 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(buffer));
    	
        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);
        cout << "curl done" << endl;
        /* Check for errors */ 
        if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    
        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    return 0;
}

const char* create_json(boost::circular_buffer<cam> information){
  json_object *info = json_object_new_object();
  json_object *coords = json_object_new_object();
  int max = 0;
  for(cam temp : information){
    if(temp.speed > max){
      max = temp.speed;
    }
  }


  cam last_known_cam = information.back();
  cam first_known_cam = information.front();

  string video_id = to_string(last_known_cam.station_id);
  video_id = video_id + to_string((int)time(NULL));
  int video_id_int = (int)time(NULL);

  json_object *lat = json_object_new_double(last_known_cam.latitude / (pow(10.0,7.0) ));
  json_object *lon = json_object_new_double(last_known_cam.longitude / (pow(10.0,7.0) ));
  json_object *speed = json_object_new_double(max * 0.036);
  json_object *heading = json_object_new_double(first_known_cam.heading / (pow(10.0,7.0)));
  json_object *station_width = json_object_new_double(last_known_cam.vehicleWidth * 100);
  json_object *station_length = json_object_new_double(last_known_cam.vehicleLength * 100);
  json_object *n_people = json_object_new_int(last_known_cam.persons);
  json_object *airbags = json_object_new_int(last_known_cam.airbags);
  json_object *abs = json_object_new_boolean(last_known_cam.abs);
  json_object *overturned = json_object_new_boolean(last_known_cam.overturned);
  json_object *hazard_lights = json_object_new_boolean(last_known_cam.hazard_lights);
  json_object *all_seatbelts = json_object_new_boolean(last_known_cam.all_seatbelts);
  json_object *video_ids = json_object_new_int(video_id_int);
  json_object *temperature = json_object_new_int(last_known_cam.temperature);

  json_object_object_add(coords, "lat", lat);
  json_object_object_add(coords, "lng", lon);
  json_object_object_add(info, "location", coords);
  json_object_object_add(info, "video_id", video_ids);
  json_object_object_add(info, "velocity", speed);
  json_object_object_add(info, "n_people", n_people);
  json_object_object_add(info, "airbag", airbags);
  json_object_object_add(info, "ABS", abs);
  json_object_object_add(info, "overturned", overturned);
  json_object_object_add(info, "station_width", station_width);
  json_object_object_add(info, "station_length", station_length);
  json_object_object_add(info, "heading", heading);
  json_object_object_add(info, "hazard_lights", hazard_lights);
  json_object_object_add(info, "all_seatbelts", all_seatbelts);
  json_object_object_add(info, "temperature", temperature);
  
  const char *jsontxt = json_object_to_json_string(info);

  return jsontxt;
}

void warn_camera(const char *json){
  json_object *json_obj = json_tokener_parse(json);

  json_object *tmp;

  json_object_object_get_ex(json_obj, "video_id", &tmp);

  string video_id = json_object_get_string(tmp);

  ofstream out("/tmp/flag");

  out << video_id;
  out.close();

}

void send_to_api(boost::circular_buffer<cam> information){
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  const char *json = create_json(information);
  
  warn_camera(json);
  
  send_information(json);
  
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-begin;
  std::cout << "elapsed time upload emergency message: " << elapsed_seconds.count() << "s\n";

}

float degreesToRadians(float number)
{
  return number * M_PI / 180;
}

float radiansToDegrees(float radians){
  return radians*180/M_PI;
}

float distanceBetweenCoordinates(float lat1, float lon1, float lat2, float lon2) {
  int earth_radius = 6371;

  float lat_distance = degreesToRadians(lat2-lat1);
  float lon_distance = degreesToRadians(lon2-lon1);

  lat1 = degreesToRadians(lat1);
  lat2 = degreesToRadians(lat2);

  float a = sin(lat_distance/2) * sin(lat_distance/2) +
          sin(lon_distance/2) * sin(lon_distance/2) * cos(lat1) * cos(lat2); 
  float c = 2 * atan2(sqrt(a), sqrt(1-a)); 
  return 1000 * earth_radius * c; //return in meters
}

std::tuple<float, float> newCoordinates(float lat, float lon, float heading, float distance){
  float rEarth = 6371.01;
  float epsilon = 0.000001;

  float rlat1 = degreesToRadians(lat);
  float rlon1 = degreesToRadians(lon);
  float rheading = degreesToRadians(heading);
  float rdistance = distance / rEarth;
  float rlon;

  float rlat = asin( sin(rlat1) * cos(rdistance) + cos(rlat1) * sin(rdistance) * cos(rheading) );

  if (cos(rlat) == 0 or abs(cos(rlat)) < epsilon){ 
    rlon=rlon1;
  }else{
    rlon = fmod((rlon1 - asin( sin(rheading)* sin(rdistance) / cos(rlat) ) + M_PI ),(2*M_PI)) - M_PI;
  }

  float new_lat = radiansToDegrees(rlat);
  float new_lon = radiansToDegrees(rlon);

  std::tuple<float, float> coordinates = std::make_tuple(new_lat, new_lon);

  return coordinates;
}
  


int gateway_selfSelection(std::unordered_map<int, boost::circular_buffer<cam>> storage, int accidented_id){
  // obter iterador com todos os veiculos ao nosso lado
  // percorrer o iterador e armazenar as distâncias dos veículos ao acidente
  // filtrar os veículos por distancia
  // escolher o veiculo com menor station id
  // dos carros acidentados, filtrar por tempo
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  std::map<int, double> distances;
  boost::circular_buffer<cam> accidented_cam_storage = storage[accidented_id];
  cam last_cam = accidented_cam_storage.back();
  int accidented_lat = last_cam.latitude;
  int accidented_long = last_cam.longitude;
  int accidented_heading = last_cam.heading;
  float acc_lat = accidented_lat / (pow(10.0,7.0));
  float acc_lon = accidented_long / (pow(10.0,7.0));
  double acc_heading = accidented_heading / (pow(10.0,7.0));
  float max_heading = fmod((acc_heading + 70.0), 360);
  float min_heading = fmod((acc_heading - 70.0), 360);

  for( auto it = storage.begin(); it != storage.end(); ++it){
    cout << endl;
    int station_id = it->first;
    boost::circular_buffer<cam> cam_storage = it->second;
    // calculate the distances of all nearby vehicles
    boost::circular_buffer<cam> accidented_cam_storage = storage[station_id];
    cam last_cam = accidented_cam_storage.back();
    float station_lat = last_cam.latitude / (pow(10.0,7.0));
    float station_lon = last_cam.longitude / (pow(10.0,7.0));
    float station_heading = last_cam.heading / (pow(10.0,7.0));

    float distance = distanceBetweenCoordinates(acc_lat, acc_lon, station_lat, station_lon);

    // debug block
    cout << "station id: " << station_id << " accidented_id: " << accidented_id << endl;
    cout << "station_lat: " << station_lat << " station_lon: " << station_lon << endl;
    cout << "station_heading:   " << station_heading << endl;
    cout << "acc_lat:     " << acc_lat     << " acc_lon:     " << acc_lon << endl;
    cout << "acc_heading:   " << acc_heading << endl;
    cout << "distance: " << distance << endl;

    // all the information is gathered, start to process it
    // this version implements heading
    
    // filter by distance first
    if(distance < 50 && distance > 10 ){
        //this station meets the distance requirements, does it meet the heading requirements?
        if(station_heading > min_heading && station_heading < max_heading){
          // this station also meets the heading requirements, is it behind the accident?
          // using the position of the accidented car and it's heading "move it" 5 meters and calculate new distance
          // if new distance > previous distance it means that our gateway car is behind the accident
        
          std::tuple<float, float> coordinates = newCoordinates(acc_lat, acc_lon, acc_heading, 0.0005);
          float newDistance = distanceBetweenCoordinates(std::get<0>(coordinates), std::get<1>(coordinates), station_lat, station_lon);
          cout << "newDistance:   " << newDistance << endl;

          if((distance - newDistance) < 0){
            //this station meets all the requirements
            cout << "I'm behind the accidented car" << endl;
            distances[station_id] = distance;
          }else{
            cout << "I'm in front of the accidented car" << endl;
          }
        }
      }  
    }

  
  

  



  // am i the chosen one?
  if(distances.begin()->first == self_station){
    cout << "i'm the chosen one" << endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-begin;
    std::cout << "elapsed time gateway algorithm: " << elapsed_seconds.count() << "s\n";
    return 1;
    

  }
  
  cout << "i'm not the chosen one" << endl;
  return 0;
}




static bool onPacketArrives(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
  {
    // extract the stats object form the cookie
    PacketStats* stats = (PacketStats*)cookie;


    // parsed the raw packet
    pcpp::Packet parsedPacket(packet);

    // collect stats from packet
    stats->parsePacket(parsedPacket);

    return false;
  }

PacketStats stats;

static void show_usage(std::string name){
  
  std::cerr << "Usage:  " << name << " <option(s)>\n"
            << "Options:\n"
            << "\t-V. --vehicle-id \t\t Vehicle ID sent in CAM messages\n"
            << "\t-I, --ip \t\t IP Address of WAVE interface\n"
            << "\t-A, --api \t\t API endpoint to send accident information"
            << endl;
}

int main(int argc, char* argv[])
{

  if(argc < 7){
    show_usage(argv[0]);
    return 0;
  }
  std::string interfaceIPAddr;
  for(int i = 1; i < argc; ++i){
    string arg = argv[i];
    if((arg == "-V" ) || (arg == "--vehicle-id")){
      if(i + 1 < argc){
        self_station = strtol(argv[i+1], NULL, 10);
      }else{
        cerr << "Provide a station ID" << endl;
        return 0;
      }
    }else if((arg == "-I" ) || (arg == "--ip")){
      if(i + 1 < argc){
      interfaceIPAddr = argv[i+1];
      }else{
        cerr << "Provide an IP Address" << endl;
      }
    }else if((arg == "-A" ) || (arg == "--api")){
      if(i + 1 < argc){
      endpoint = argv[i+1];
      }else{
        cerr << "Provide an API endpoint" << endl;
      }
    }
  }

	
  // find the interface by IP address
  pcpp::PcapLiveDevice* dev = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDeviceByIp(interfaceIPAddr.c_str());
  if (dev == NULL)
  {
    printf("Cannot find interface with IPv4 address of '%s'\n", interfaceIPAddr.c_str());
    exit(1);
  }

  if (!dev->open())
  {
    printf("Cannot open device\n");
    exit(1);
  }

	dev->startCaptureBlockingMode(onPacketArrives, &stats, 0);
  

	return 0;
}
