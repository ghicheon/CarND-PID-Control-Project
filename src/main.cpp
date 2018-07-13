#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

#define  DEBUG_MESSAGE

#define NEW_START()   do {                                                                   \
        double dpsum = (dp[0] + dp[1] + dp[2]);\
        std::cout << "dpsum:" << dpsum << std::endl ;\
        if( dpsum < 1)\
        { \
            std::cout << "All is done!!!!!!! dpsum"  << p[0] << p[1] << p[2] << std::endl ;\
                exit(0);\
        }\
          stopped=0;\
                           pid.Init( p[0] ,p[1],p[2]);                                       \
                           std::string reset_msg = "42[\"reset\",{}]";                       \
                           ws.send(reset_msg.data(), reset_msg.length(), uWS::OpCode::TEXT); \
                      }                                                                      \
                      while(0)

#define PARAMETER_SHIFT() if(i==2) i=0; else i++;       std::cout << "i:" << i << std::endl;


#define TEST_TIMEOUT     100

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
    uWS::Hub h;
    
    PID pid;
    static int stopped=0;
    
static int i=0;//which parameter?
static int stage=0;
static int cnt=0;
    
    static double p[3] = {0,0,0};
    static double dp[3] = {1,1,1};
    
    static double besterr=9999999999999999.0;// just big number.  
    
    //TODO: Initialize the pid variable.
    pid.Init( p[0] ,p[1],p[2]);
    
    h.onMessage([&pid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {

if(stage == 0)
{
    
    if(cnt == 0 )
    {
        std::cout << "i:" << i << "p before" << p[i] << std::endl;
        p[i] += dp[i];
        std::cout << "i:" << i << "p after" << p[i] << std::endl;
    }
    else if( cnt == TEST_TIMEOUT )
    { 
        //get absolute value! the lower, the better!
       // int err = pid.sum < 0  ? (-1)*pid.sum : pid.sum ; 
        int err = pid.abs_sum;

        std::cout << "Twiddle(P:" << p[0] <<" ,I:" << p[1] <<" ,D:" << p[2] << ") err: " << err<< 
                              "pid.sum:" << pid.sum << std::endl;
        
        if( stopped == 1 )//special case
        {
                p[i] -= dp[i];
                PARAMETER_SHIFT();

                cnt=0;
                NEW_START(); //!!!!!!!
        }
        else
        {
                if( (err < besterr) )
                {
                     besterr = err;

                     dp[i] *= 1.2;
                     PARAMETER_SHIFT();

                     cnt=0;
                     NEW_START(); //!!!!!!!

                }
                else
                {
                std::cout << "i:" << i << "p before" << p[i] << std::endl;
                    p[i] -= 2*dp[i] ; 
                std::cout << "i:" << i << "p after" << p[i] << std::endl;

                    stage =1;
                    cnt=0;
                    NEW_START(); //!!!!!!!
                }
        }
    }
}
else if (stage == 1)
{
    if( cnt == TEST_TIMEOUT )
    {
        //get absolute value! the lower, the better!
        //int err = pid.sum < 0  ? (-1)*pid.sum : pid.sum ; 
        int err = pid.abs_sum;

        std::cout << "Twiddle(P:" << p[0] <<" ,I:" << p[1] <<" ,D:" << p[2] << ") err: " << err<< std::endl;

        if( stopped == 1 )//special case
        {
                p[i] += 2*dp[i] ; 
                PARAMETER_SHIFT();

                cnt=0;
                NEW_START(); //!!!!!!!
        }
        else
        {

                if( (err < besterr)  && stopped != 1)
                {
                     besterr = err;

                     dp[i] *= 1.2;
                     PARAMETER_SHIFT();

                     stage=0;
                     cnt=0;
                    NEW_START(); //!!!!!!!
                }
                else
                {
                std::cout << "i:" << i << "p before" << p[i] << std::endl;
                     p[i] += dp[i];
                std::cout << "i:" << i << "p after" << p[i] << std::endl;

                     dp[i] *= 0.8;
                     PARAMETER_SHIFT();

                     stage=0;
                     cnt=0;
                    NEW_START(); //!!!!!!!
                }
         }
    }

}


cnt++;
    
//        if( besterr < 20 )
//        { 
//            std::cout << "All is done!!!!!!! besterr"  << p[0] << p[1] << p[2] << std::endl ;
//        
//                exit(0);
//        }
///////////////////////////////////////////////////////////////

    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;


          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */

if( cnt > 20   && speed < 0.05)
{
        stopped=1;
        std::cout <<  "stopped!!!!!!!!!!!!!!!!!" <<std::endl;
}
else
{
          pid.UpdateError(cte);
          steer_value = pid.TotalError();
}

          
          // DEBUG
#ifdef DEBUG_MESSAGE
          std::cout << "*** CTE: " << cte << "SPEED:" << speed <<  "angle:" << angle << " Steering Value: " << steer_value << std::endl;

          //std::cout << "***[data for graph] p_err:" << pid.p_error << "   i_err:" << pid.i_error <<  "   d_err:" << pid.d_error << std::endl;
#endif

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.3;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          //std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
