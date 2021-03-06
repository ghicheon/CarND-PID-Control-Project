#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

/************************************************** 
 * for running normally, uncomment it!!
 * for traing, comment it.
 **************************************************/
#define  RUN_NORMAL

//print some debug messages
//#define  DEBUG_MESSAGE

//how long do we train?
#define TRAINING_TIMEOUT     500


//it makes the simulater restart.
#define NEW_START()  \
    do {                                                 \
        double dpsum = (dp[0] + dp[1] + dp[2]); \
        if( dpsum < 0.1)/*end condition*/  \
        { \
            std::cout << "All is done!!!!!!! p  "  << p[0]  << "," << p[1] <<  "," << p[2] << std::endl ;\
            std::cout << "All is done!!!!!!! dp "  << dp[0]  << "," << dp[1] <<  "," << dp[2] << std::endl ;\
            exit(0);\
        }\
        pid.Init( p[0] ,p[1],p[2]);\
        std::cout << "[NEW_START i:" << i << " stage:" << stage << "]     " << "p:" << p[0] << "/" << p[1] << "/" << p[2] <<  " /// dp:" << dp[0] << "/" << dp[1] << "/" << dp[2] << std::endl ;\
        \
        cnt=0;\
        stopped=0;\
        off_line_error=0;\
        std::string reset_msg = "42[\"reset\",{}]";                       \
        ws.send(reset_msg.data(), reset_msg.length(), uWS::OpCode::TEXT); \
        return ;\
    }                                                                      \
    while(0)

#define PARAMETER_SHIFT() if(i==2) i=0; else i++;    //   std::cout << "i:" << i << std::endl;



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
    static int off_line_error = 0;
    static int i=0;//which parameter? 0? 1? 2?

    /*  [twiddle stages] 
     *   - there is 2 stages in twiddle algoritim.  0 and 1
     *   - 999 is just for normal test. I mean training is ignored when 999.
     */
    static int stage=999;  
    static int cnt=0;   //it is increased when getting a data from simulator.
    static double dp[3] = {1,1,0.0001};
    static double besterr=9999999999999999.0;// just big number.  

#ifdef  RUN_NORMAL
    static double p[3] = {0.3,3.0,0.001}; //good. found it manually.
#else
    static double p[3] = {1,1,0.001}; //initial values for training!

    stage=0;//for training!
#endif


    //TODO: Initialize the pid variable.
    pid.Init( p[0] ,p[1],p[2]);
    
    h.onMessage([&pid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
       if(stage == 0)
       {
           if(cnt == 0 )
           {
               //std::cout << "p[" << i << "] before:" << p[i] << std::endl;
               p[i] += dp[i];
               //std::cout << "p[" << i << "] after:" << p[i] << std::endl;
           }
           else if( cnt == TRAINING_TIMEOUT ) 
           { 
               //get absolute value! the lower, the better!
               // int err = pid.sum < 0  ? (-1)*pid.sum : pid.sum ; 
               double err = pid.abs_sum;
       
               std::cout << "     [result i:" << i << " stage:" << stage << "]     " 
                          <<"p:" << p[0] << "/" << p[1] << "/" << p[2] << 
                      " *** dp:" << dp[0] << "/" << dp[1] << "/" << dp[2]  << 
                      " *** err:" << pid.abs_sum << std::endl;
               
               if( stopped == 1   || off_line_error == 1 )//critical error!!
               {
                   p[i] -= dp[i];
                   dp[i] *= 0.95;

                   stage=0; //for real new start!
                   PARAMETER_SHIFT();//next parameter
                   NEW_START();  //!!!
               }
               else if( err < besterr )
               {
                    besterr = err;
    
                    dp[i] *= 1.2;

                    PARAMETER_SHIFT();
                    NEW_START(); //!!!
    
               }
               else
               {
                   p[i] -= 2*dp[i] ; 

                   stage =1;
                   NEW_START(); //!!!
               }
           }
       }
       else if (stage == 1)
       {
           if( cnt == TRAINING_TIMEOUT )
           {
               //get absolute value! the lower, the better!
               //int err = pid.sum < 0  ? (-1)*pid.sum : pid.sum ; 
               int err = pid.abs_sum;
       
               std::cout << "     [result i:" << i << " stage:" << stage << "]     " 
                          <<"p:" << p[0] << "/" << p[1] << "/" << p[2] << 
                      " *** dp:" << dp[0] << "/" << dp[1] << "/" << dp[2]  << 
                      " *** err:" << pid.abs_sum << std::endl;
       
       
               if( stopped == 1   || off_line_error == 1 )//critical error!!
               {
                   p[i] += dp[i];
                   dp[i] *= 0.95;
               }
               else if( err < besterr )
               {
                   besterr = err;
                   dp[i] *= 1.2;
               }
               else
               {
                   p[i] += dp[i];//set original value
                   dp[i] *= 0.8;
               }

               stage=0; //for fresh new start!
               PARAMETER_SHIFT();
               NEW_START(); //!!!
           }
       
       }
    
       cnt++;
    
       //if( besterr < 20 )
       //{ 
       //    std::cout << "All is done!!!!!!! besterr"  << p[0] << p[1] << p[2] << std::endl ;
       //
       //        exit(0);
       //}

       // "42" at the start of the message means there's a websocket message event.
       // The 4 signifies a websocket message
       // The 2 signifies a websocket event
       if (length && length > 2 && data[0] == '4' && data[1] == '2')
       {
         auto s = hasData(std::string(data).substr(0, length));
         if (s != "") 
         {
           auto j = json::parse(s);
           std::string event = j[0].get<std::string>();
           if (event == "telemetry") 
           {
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
                     if( stopped == 0) //print only 1 time.
                             std::cout <<  "stopped!!!!!!!!!!!!!!!!!" ;

                     stopped=1;
             }
             else if( cte > 3 || cte < -3 )
             {
                     if( off_line_error == 0) //print only 1 time.
                             std::cout <<  "off_line_error!!!!!!!!!!!!!!!!!" ;

                     off_line_error = 1;
             }
             else
             {
                       pid.UpdateError(cte);
                       steer_value = pid.TotalError();
             }

#ifdef DEBUG_MESSAGE
             std::cout << "*** CTE: " << cte  ;
             //std::cout << "*** CTE: " << cte << "SPEED:" << speed <<  "angle:" << angle << " Steering Value: " << steer_value << std::endl;
             //std::cout << "***[data for graph] p_err:" << pid.p_error << "   i_err:" << pid.i_error <<  "   d_err:" << pid.d_error << std::endl;
#endif
             json msgJson;
             msgJson["steering_angle"] = steer_value;
             msgJson["throttle"] = 0.3;
             auto msg = "42[\"steer\"," + msgJson.dump() + "]";
             //std::cout << msg << std::endl;
             ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
           }
         } 
         else 
         {
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
