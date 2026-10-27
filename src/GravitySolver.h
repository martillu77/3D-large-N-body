#ifndef GRAVITY_SOLVER_H
#define GRAVITY_SOLVER_H

#include "Particle.h"
#include <vector>

namespace gravity {
    struct GravityResult { 
        std::vector<Vector3> accelerations;
        double min_dynamical_time = 0.0;
    };

    namespace gravity {
        GravityResult compute_accelerations( const std::vector<Particle>& particles );
    }

    GravityResult compute_accelerations( const std::vector<Particle>& particles);

}




#endif