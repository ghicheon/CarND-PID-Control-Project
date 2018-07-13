#include "PID.h"

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

   p_error=0;
   i_error=0;
   d_error=0;

   sum =0;
   prev_cte=0;
}


void PID::UpdateError(double cte) {

    double diff=0;

    sum += cte;
    
    diff =  cte - prev_cte ;
    
    prev_cte = cte;
    
    p_error = -Kp*cte;
    i_error = -Ki*diff;
    d_error = -Kd*sum;

}

double PID::TotalError() {
        return p_error + i_error + d_error;
}

