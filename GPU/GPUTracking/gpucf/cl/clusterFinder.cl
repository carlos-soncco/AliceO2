#include "config.h"

#include "shared/tpc.h"



constant charge_t CHARGE_THRESHOLD = 2;
constant charge_t OUTER_CHARGE_THRESHOLD = 0;





Cluster newCluster()
{
    Cluster c = {0, 0, 0, 0, 0, 0, 0, 0};
    return c;
}

void updateCluster(Cluster *cluster, float charge, int dp, int dt)
{
    cluster->Q         += charge;
    cluster->padMean   += charge*dp;
    cluster->timeMean  += charge*dt;
    cluster->padSigma  += charge*dp*dp;
    cluster->timeSigma += charge*dt*dt;
}

void updateClusterOuter(
        global const charge_t *chargeMap,
                     Cluster  *cluster, 
                     int       row,
                     int       pad,
                     int       time,
                     charge_t  innerCharge,
                     int       dpIn, 
                     int       dtIn,
                     int       dpOut,
                     int       dtOut)
{
    charge_t outerCharge = CHARGE(chargeMap, row, pad+dpOut, time+dtOut);

    if (   innerCharge >       CHARGE_THRESHOLD 
        && outerCharge > OUTER_CHARGE_THRESHOLD) 
    {
        updateCluster(cluster, outerCharge, dpOut, dtOut);
    }
}

void addCorner(
        global const charge_t *chargeMap,
                     Cluster  *myCluster,
                     int       row,
                     int       pad,
                     int       time,
                     int       dp,
                     int       dt)
{
    charge_t innerCharge = CHARGE(chargeMap, row, pad+dp, time+dt);
    updateCluster(myCluster, innerCharge, dp, dt);
    updateClusterOuter(chargeMap, myCluster, row, pad, time, innerCharge, dp, dt, 2*dp,   dt);
    updateClusterOuter(chargeMap, myCluster, row, pad, time, innerCharge, dp, dt,   dp, 2*dt);
    updateClusterOuter(chargeMap, myCluster, row, pad, time, innerCharge, dp, dt, 2*dp, 2*dt);
}

void addLine(
        global const charge_t *chargeMap,
                     Cluster  *myCluster,
                     int       row,
                     int       pad,
                     int       time,
                     int       dp,
                     int       dt)
{
    charge_t innerCharge = CHARGE(chargeMap, row, pad+dp, time+dt);
    updateCluster(myCluster, innerCharge, dp, dt);
    updateClusterOuter(chargeMap, myCluster, row, pad, time, innerCharge, dp, dt, 2*dp, 2*dt);
}

void buildCluster(
        global const charge_t *chargeMap,
                     Cluster  *myCluster,
                     int       row,
                     int       pad,
                     int       time)
{
    myCluster->Q = 0;
    myCluster->QMax = 0;
    myCluster->padMean = 0;
    myCluster->timeMean = 0;
    myCluster->padSigma = 0;
    myCluster->timeSigma = 0;

    // Add charges in top left corner:
    // O O o o o
    // O I i i o
    // o i c i o
    // o i i i o
    // o o o o o
    addCorner(chargeMap, myCluster, row, pad, time, -1, -1);

    // Add charges in top right corner:
    // o o o O O
    // o i i I O
    // o i c i o
    // o i i i o
    // o o o o o
    addCorner(chargeMap, myCluster, row, pad, time, 1, -1);

    // Add charges in bottom right corner:
    // o o o o o
    // o i i i o
    // o i c i o
    // o i i I O
    // o o o O O
    addCorner(chargeMap, myCluster, row, pad, time, 1, 1);

    // Add charges in bottom left corner:
    // o o o o o
    // o i i i o
    // o i c i o
    // O I i i o
    // O O o o o
    addCorner(chargeMap, myCluster, row, pad, time, -1, 1);

    // Add remaining charges:
    // o o O o o
    // o i I i o
    // O I c I O
    // o i I i o
    // o o O o o
    addLine(chargeMap, myCluster, row, pad, time,  0, -1);
    addLine(chargeMap, myCluster, row, pad, time,  1,  0);
    addLine(chargeMap, myCluster, row, pad, time,  0,  1);
    addLine(chargeMap, myCluster, row, pad, time, -1,  0);
}


#define HALF_NEIGHBORS_NUM 4

constant int2 LEQ_NEIGHBORS[HALF_NEIGHBORS_NUM] = 
{
    (int2)(-1, -1), 
    (int2)(-1, 0), 
    (int2)(-1, 1),
    (int2)(0, -1)
};
constant int2 LQ_NEIGHBORS[HALF_NEIGHBORS_NUM]  = 
{
    (int2)(0, 1),
    (int2)(1, -1),
    (int2)(1, 0), 
    (int2)(1, 1)
};



bool isPeak(
               const Digit    *digit,
        global const charge_t *chargeMap)
{
    const charge_t myCharge = digit->charge;
    short time = digit->time;
    uchar row = digit->row;
    uchar pad = digit->pad;

    bool peak = true;

#define CMP_NEIGHBOR(dp, dt, cmpOp) \
    do \
    { \
        const charge_t otherCharge = CHARGE(chargeMap, row, pad+dp, time+dt); \
        peak &= (otherCharge cmpOp myCharge); \
    } \
    while (false)

#define CMP_LT CMP_NEIGHBOR(-1, -1, <=)
#define CMP_T  CMP_NEIGHBOR(-1, 0, <=)
#define CMP_RT CMP_NEIGHBOR(-1, 1, <=)

#define CMP_L  CMP_NEIGHBOR(0, -1, <=)
#define CMP_R  CMP_NEIGHBOR(0, 1, < )

#define CMP_LB CMP_NEIGHBOR(1, -1, < )
#define CMP_B  CMP_NEIGHBOR(1, 0, < )
#define CMP_RB CMP_NEIGHBOR(1, 1, < )

#if defined(CHARGEMAP_TILING_LAYOUT)
    CMP_LT;
    CMP_T;
    CMP_RT;
    CMP_R;
    CMP_RB;
    CMP_B;
    CMP_LB;
    CMP_L;
#else
    CMP_LT;
    CMP_T;
    CMP_RT;
    CMP_L;
    CMP_R;
    CMP_LB;
    CMP_B;
    CMP_RB;
#endif

#undef CMP_LT
#undef CMP_T
#undef CMP_RT
#undef CMP_L
#undef CMP_R
#undef CMP_LB
#undef CMP_B
#undef CMP_RB
#undef CMP_NEIGHBOR

    peak &= (myCharge > CHARGE_THRESHOLD);

    return peak;
}


void finalizeCluster(
                     Cluster *myCluster, 
               const Digit   *myDigit, 
        global const int     *globalToLocalRow,
        global const int     *globalRowToCru)
{
    myCluster->Q += myDigit->charge;

    float totalCharge = myCluster->Q;
    float padMean     = myCluster->padMean;
    float timeMean    = myCluster->timeMean;
    float padSigma    = myCluster->padSigma;
    float timeSigma   = myCluster->timeSigma;

    padMean   /= totalCharge;
    timeMean  /= totalCharge;
    padSigma  /= totalCharge;
    timeSigma /= totalCharge;
    
    padSigma  = sqrt(padSigma  - padMean*padMean);

    timeSigma = sqrt(timeSigma - timeMean*timeMean);

    padMean  += myDigit->pad;
    timeMean += myDigit->time;

    myCluster->QMax      = round(myDigit->charge);
    myCluster->padMean   = padMean;
    myCluster->timeMean  = timeMean;
    myCluster->timeSigma = timeSigma;
    myCluster->padSigma  = padSigma;

    myCluster->cru = globalRowToCru[myDigit->row];
    myCluster->row = globalToLocalRow[myDigit->row];
}


kernel
void fillChargeMap(
       global const Digit    *digits,
       global       charge_t *chargeMap)
{
    int idx = get_global_id(0);
    Digit myDigit = digits[idx];

    DIGIT_CHARGE(chargeMap, myDigit) = myDigit.charge;
}

kernel
void resetChargeMap(
        global const Digit    *digits,
        global       charge_t *chargeMap)
{
    int idx = get_global_id(0);
    Digit myDigit = digits[idx];

    DIGIT_CHARGE(chargeMap, myDigit) = 0.0f;
}

kernel
void findPeaks(
         global const charge_t *chargeMap,
         global const Digit    *digits,
         global       int      *isPeakPredicate)
{
    int idx = get_global_id(0);
    Digit myDigit = digits[idx];

    bool peak = isPeak(&myDigit, chargeMap);

    isPeakPredicate[idx] = peak;
}

kernel
void computeClusters(
        global const charge_t *chargeMap,
        global const Digit    *digits,
        global const int      *globalToLocalRow,
        global const int      *globalRowToCru,
        global       Cluster  *clusters)
{
    int idx = get_global_id(0);

    Digit myDigit = digits[idx];

    Cluster myCluster;
    buildCluster(chargeMap, &myCluster, myDigit.row, myDigit.pad, myDigit.time);
    finalizeCluster(&myCluster, &myDigit, globalToLocalRow, globalRowToCru);

    clusters[idx] = myCluster;
}
