#include "GravitySolver.h"

#include <cmath>
#include <cstddef>
#include "cstdio"

namespace gravity {

    namespace {
        constexpr double G = 1.0;  // This is possible because we re-scaled everything
    }

    GravityResult compute_accelerations( const std::vector<Particle>& particles ) {
        const std::size_t N = particles.size();

        // Output accelerations and min_dynamical time:
        std::vector<Vector3> accelerations(N);
        double min_dynamical_time_squared = std::numeric_limits<double>::max();

        #ifdef DEBUG_FLAG
            double e_kin=0., e_pot=0., l_totx=0., l_toty=0., l_totz=0.;
            FILE* log = std::fopen("debugo.log", "a");
            //std::fprintf(log, "particles size      %zu\n", N);
        #endif

        for (std::size_t i = 0; i < N; ++i) {
            #ifdef DEBUG_FLAG
                e_kin += 0.5 * particles[i].mass * (particles[i].velocity.x*particles[i].velocity.x + 
                                                    particles[i].velocity.y*particles[i].velocity.y + 
                                                    particles[i].velocity.z*particles[i].velocity.z);         // Kinetic energy for particle "i"
                l_totx += particles[i].mass * (particles[i].position.y * particles[i].velocity.z - 
                                               particles[i].position.z * particles[i].velocity.y);
                l_toty += particles[i].mass * (particles[i].position.z * particles[i].velocity.x - 
                                               particles[i].position.x * particles[i].velocity.z);
                l_totz += particles[i].mass * (particles[i].position.x * particles[i].velocity.y - 
                                               particles[i].position.y * particles[i].velocity.x);            // Angular momentum, taken from  (x, y, z) = (0, 0, 0)
            #endif

            for (std::size_t j = i + 1; j < N; ++j) {

                const double dx = particles[j].position.x - particles[i].position.x;
                const double dy = particles[j].position.y - particles[i].position.y;
                const double dz = particles[j].position.z - particles[i].position.z;

                const double r2 = dx * dx + dy * dy + dz * dz;

                if (r2 == 0.0) continue;  // Avoid division by zero if two particles coincide

                const double r = std::sqrt(r2);
                const double inv_r3 = 1.0 / (r2 * r);

                const double factor_i = G * particles[j].mass * inv_r3;
                const double factor_j = G * particles[i].mass * inv_r3;

                accelerations[i].x += factor_i * dx;
                accelerations[i].y += factor_i * dy;
                accelerations[i].z += factor_i * dz;

                accelerations[j].x -= factor_j * dx;
                accelerations[j].y -= factor_j * dy;
                accelerations[j].z -= factor_j * dz;

                double thisdynamical_time_squared = 1./factor_i;  // min [ sqrt( l_i / abs(a_i) ) ]
                if (thisdynamical_time_squared < min_dynamical_time_squared) min_dynamical_time_squared = thisdynamical_time_squared;

                #ifdef DEBUG_FLAG
                    e_pot -= particles[i].mass * particles[j].mass / r;                                       // Gravitational potential Energy
                    //std::fprintf(log, "factor_i      %.10e\n", factor_i);
                    //std::fprintf(log, "distance_ij   %.10e\n", r);
                    //std::fprintf(log, "mass          %.10e\n", particles[j].mass);
                #endif
            }
        }

        #ifdef DEBUG_FLAG
            //FILE* log = std::fopen("debug.log", "a");

            if (log != nullptr) {
                std::fprintf(log, "Kinetic energy:   %.10e\n", e_kin);
                std::fprintf(log, "Angular momentum:\n");
                std::fprintf(log, "  L_x : %.10e\n", l_totx);
                std::fprintf(log, "  L_y : %.10e\n", l_toty);
                std::fprintf(log, "  L_z : %.10e\n", l_totz);
                std::fprintf(log, "Potential energy: %.10e\n", e_pot);
                std::fprintf(log, "Total energy: %.10e\n", e_pot + e_kin);
                std::fprintf(log, "------------------------------\n");

                std::fclose(log);
            }
        #endif

        return {accelerations, std::sqrt(min_dynamical_time_squared)};
    }

}