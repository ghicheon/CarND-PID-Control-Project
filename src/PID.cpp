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

unsigned int sum =0;
std::vector<double> ctelist;
double prev_cte=0;

void PID::UpdateError(double cte) {

    double diff=0;
    
    if( ctelist.size() > 20)
    {
            ctelist.pop_back();       
    }
    
    for(int i=0; i< ctelist.size() ; i++ )
            sum += ctelist[i];
    
    diff =  cte - prev_cte ;
    
    prev_cte = cte;
    
    p_error = -Kp*cte;
    i_error = -Ki*diff;
    d_error = -Kd*sum;
}

double PID::TotalError() {
        return p_error + i_error + d_error;
}

