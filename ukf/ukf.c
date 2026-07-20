#include <stdint.h>
#include "ukf.h"
#include "arm_math.h"


void sigma_function(UKF *ukf){
    float alpha = ukf->alpha;
    float n = ukf->n;
    ukf->lambda = alpha*alpha * (n + ukf->kappa) - n;

    float in, a;
    for(int j=0;j<n;j++){
        // first sigma point is mean
        X[j][0] = mu;
        for(int i=1;i< 2*n;i++){
            in = (i + ukf->lambda)*ukf->P[i];   // double check
            arm_sqrt_f32(in, &a);   // arm sqrt function
            if (i <= n) {
                X[j][i] = mu + a;   // need to change to actual mean
            } else if (n < i <= 2*n){
                X[j][i] = mu - a;
            }
        }
    }
}

void weight_function(UKF *ukf) {
    float n = ukf->n;
    float lambda = ukf->lambda;
    float alpha = ukf->alpha;
    for (int h=0;h<n;h++){
        ukf->Wm[h][0] = lambda / (n + lambda);
        ukf->Wc[h][0] = lambda / (n + lambda) + 1 - alpha * alpha + ukf->beta;
        for (int i=0;i< 2*n;i++){
            ukf->Wm = 1 / (2 * (n + lambda));
            ukf->Wc = 1 / (2 * (n + lambda));
        }
    }
}




void ukf_predict(UKF *ukf, float delta_t) {
    sigma_function(ukf);  // need to do sigma function
    weight_function(ukf);

    process_model(ukf, delta_t);
    unscented_transform(ukf);
}

void ukf_update(UKF *ukf){
    measurement_function(ukf);
    unscented_transform(ukf);

    //y = z - u;  residual of measurement
    cross_covariance(ukf);
    
    //TODO calculate kalman gain

    // x = x_dash + Ky;  new state estimate
    // P = P_dash - KPzK^T; new covariance

}