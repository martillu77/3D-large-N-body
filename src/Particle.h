#ifndef PARTICLE_H
#define PARTICLE_H

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    double mass = 1.0;
    double radius = 0.05;
};

#endif