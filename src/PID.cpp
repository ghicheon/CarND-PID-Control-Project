#include "PID.h"

#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include <math.h>

using namespace std;

/*
* TODO: Complete the PID class.
*/

PID::PID() {}

PID::~PID() {}

void PID::Init(double kp, double ki, double kd) {
   Kp = kp;
   Ki = ki;
   Kd = kd;
}

static double sum =0;
static unsigned int cnt=0;
static std::vector<double> ctelist;
static double prev_cte=0;

void PID::UpdateError(double cte) {

    double diff=0;

    cnt++;
    
    ctelist.insert(ctelist.begin(),cte);

//    if( ctelist.size() > 20)
    {
//            ctelist.pop_back();       
    }
    
    for(int i=0; i< ctelist.size() ; i++ )
            sum += ctelist[i];
    
    diff =  cte - prev_cte ;
    
    prev_cte = cte;
    
    p_error = -Kp*cte;
    i_error = -Ki*diff;
    if( cnt != 0 ) //when overflow, just ignore.
        d_error = -Kd*(sum/(cnt*1.0));

}

double PID::TotalError() {
        return p_error + i_error + d_error;
}

