#ifndef PARTICLE_H
#define PARTICLE_H

struct Vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    Vector3& operator+=(const Vector3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3 operator-(const Vector3& other) const {
        return {
            x - other.x,
            y - other.y,
            z - other.z
        };
    }

    Vector3 operator*(double scalar) const {
        return {
            x * scalar,
            y * scalar,
            z * scalar
        };
    }

    Vector3 operator/(double scalar) const {
        return {
            x / scalar,
            y / scalar,
            z / scalar
        };
    }

    double dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};


struct Particle {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    double mass = 1.0;
    double radius = 0.05;
};


#endif