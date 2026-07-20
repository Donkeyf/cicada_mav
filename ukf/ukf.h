#include <stdint.h>

typedef struct {
    uint32_t n; // state dimension
    uint32_t m; //measurement dimension

    float *x;   //state vector
    float *P;   // covariance

    float *Q;   // process noise
    float *R;   // measurement noise

    float *Wm;   // mean weights
    float *Wc;   // covariance weights

    float *X;   // sigma points (2n+1)*n
    float *Y;   // transformed sigma points

    float alpha;
    float beta;
    float lambda;
    float kappa;
}UKF;

void ukf_init(UKF *ukf);

void ukf_predict(UKF *ukf);  // add process model
void ukf_update(UKF *ukf);   // add measurement model