#include "NoiseSuppressionOverArea.h"


using namespace gpucf;


NoiseSuppressionOverArea::NoiseSuppressionOverArea(
        int radPad, 
        int radTime, 
        int cutoff, 
        int epsilon)
    : NoiseSuppression("noiseSuppression{" + std::to_string(radPad*2+1) 
        + "x" + std::to_string(radTime*2+1) + "}")
    , radPad(radPad)
    , radTime(radTime)
    , cutoff(cutoff)
    , epsilon(epsilon)
{
    outerToInner = {
        { {-2, -2}, {{-1, -1}} },
        { {-2,  2}, {{-1,  1}} },
        { { 2, -2}, {{ 1, -1}} },
        { {-2, -2}, {{-1, -1}} },

        { {-2,  0}, {{-1,  0}} },
        { { 0, -2}, {{ 0, -1}} },
        { { 0,  2}, {{ 0,  1}} },
        { { 2,  0}, {{ 1,  0}} },

        { {-2,  1}, {{-1,  1}} },
        { {-2, -1}, {{-1, -1}} },
        { {-1, -2}, {{-1, -1}} },
        { {-1,  2}, {{-1,  1}} },
        { { 1, -2}, {{ 1, -1}} },
        { { 1,  2}, {{ 1,  1}} },
        { {-2, -1}, {{-1, -1}} },
        { {-2,  1}, {{-1,  1}} },


        { {-3, -3}, {{-2, -2}, {-1, -1}} },
        { {-3,  3}, {{-2,  2}, {-1,  1}} },
        { { 3, -3}, {{ 2, -2}, { 1, -1}} },
        { { 3,  3}, {{ 2,  2}, { 1,  1}} },

        { {-3,  0}, {{-2,  0}, {-1,  0}} },
        { { 0, -3}, {{ 0, -2}, { 0, -1}} },
        { { 0,  3}, {{ 0,  2}, { 0,  1}} },
        { { 3,  0}, {{ 2,  0}, { 1,  0}} },

        { {-3, -2}, {{-2, -1}, {-1, -1}} },
        { {-2, -3}, {{-1, -2}, {-1, -1}} },
        { {-3,  2}, {{-2,  1}, {-1,  1}} },
        { {-2,  3}, {{-1,  2}, {-1,  1}} },
        { { 3, -2}, {{ 2, -1}, { 1, -1}} },
        { { 2, -3}, {{ 1, -2}, { 1, -1}} },
        { { 3,  2}, {{ 2,  1}, { 1,  1}} },
        { { 2,  3}, {{ 1,  2}, { 1,  1}} },

        { { 3,  1}, {{ 2,  1}, { 1,  1}} },
        { { 1,  3}, {{ 1,  2}, { 1,  1}} },
        { {-3,  1}, {{-2,  1}, {-1,  1}} },
        { {-1,  3}, {{-1,  2}, {-1,  1}} },
        { { 3, -1}, {{ 2, -1}, { 1, -1}} },
        { { 1, -3}, {{ 1, -2}, { 1, -1}} },
        { {-3, -1}, {{-2, -1}, {-1, -1}} },
        { {-1, -3}, {{-1, -2}, {-1, -1}} },

        { { 4,  3}, {{ 3,  2}, { 2,  2}, { 1,  1}} },
        { { 4,  2}, {{ 3,  2}, { 2,  1}, { 1,  1}} },
        { { 4,  1}, {{ 3,  1}, { 2,  1}, { 1,  1}} },
        { {-4,  3}, {{-3,  2}, {-2,  2}, {-1,  1}} },
        { {-4,  2}, {{-3,  2}, {-2,  1}, {-1,  1}} },
        { {-4,  1}, {{-3,  1}, {-2,  1}, {-1,  1}} },
        { { 4, -3}, {{ 3, -2}, { 2, -2}, { 1, -1}} },
        { { 4, -2}, {{ 3, -2}, { 2, -1}, { 1, -1}} },
        { { 4, -1}, {{ 3, -1}, { 2, -1}, { 1, -1}} },
        { {-4, -3}, {{-3, -2}, {-2, -2}, {-1, -1}} },
        { {-4, -2}, {{-3, -2}, {-2, -1}, {-1, -1}} },
        { {-4, -1}, {{-3, -1}, {-2, -1}, {-1, -1}} },

        { { 4,  0}, {{ 3,  0}, { 2,  0}, { 1,  0}} },
        { {-4,  0}, {{-3,  0}, {-2,  0}, {-1,  0}} },
    };

    for (auto &p : outerToInner)
    {
        if (p.first.pad <= radPad && p.first.time <= radTime)
        {
            neighbors.push_back(p);    
        }
    }
}


std::vector<Digit> NoiseSuppressionOverArea::runImpl(
        View<Digit> peaks,
        const Map<bool> &peakMap,
        const Map<float> &chargeMap)
{
    std::vector<Digit> filtered;

    for (const Digit &p : peaks)
    {
        if (p.charge <= cutoff)
        {
            continue;
        }

        bool removeMe = false;

        for (const auto &n : neighbors)
        {
            if (removeMe)
            {
                break;
            }

            int dp = n.first.pad;
            int dt = n.first.time;

            Position other(p, dp, dt);

            bool otherIsPeak = peakMap[other];
            if (!otherIsPeak)
            {
                continue;
            }

            float q = p.charge;
            float oq = chargeMap[other];
            if (oq <= q)
            {
                continue;
            }

            for (const Delta &b : n.second)
            {
                Position between(p, b.pad, b.time);

                float bq = chargeMap[between];

                removeMe |= (q - bq <= epsilon);
            }
        }

        if (!removeMe)
        {
            filtered.push_back(p);
        }

    }

    return filtered;
}

// vim: set ts=4 sw=4 sts=4 expandtab:
