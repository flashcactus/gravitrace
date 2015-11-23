#ifndef _3D_H_
#define _3D_H_

#include "lib/quaternions.h"
#include "lib/glm/glm.hpp"
#include "lib/pngpp/png.hpp"
#include <cmath>
#include <cstdio>

using glm::detail::tvec3;
typedef tvec3<double> vec3;

void print_quat(quaternion q, FILE* file=stdout){
    fprintf(file,"<%-5g, %-5g, %-5g, %-5g> ",q.r,q.i,q.j,q.k);   
}

template <typename V> void print_vec3(V v, char* end=NULL, FILE* file=stdout){
    char *e;
    if(end==NULL) {
        e = "\0";    
    } else {
        e = end;    
    }
    fprintf(file,"{%-5g, %-5g, %-5g}%s",v.x,v.y,v.z,e);
}

template <typename T> tvec3<T> mul_vec(tvec3<T> v, T num) { 
    return v * tvec3<T>(num,num,num); 
}

template <typename T> tvec3<T> div_vec(tvec3<T> v, T num) { 
    return v / tvec3<T>(num,num,num); 
}

template <typename T1, typename T2> T1 dotprod(tvec3<T1> v1, tvec3<T2> v2) { 
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; 
}

template <typename T> T abs(tvec3<T> v) {
    return sqrt(dotprod(v,v));
}

vec3 normalize(vec3 v, double tgtnorm=1) {
    return div_vec(v,(abs(v)/tgtnorm));
}

quaternion rot2q(double angle, quaternion axis);

class Rotation {
    private:
        quaternion rot_quaternion;
    public:
        Rotation(double yaw, double pitch, double roll); //tait-bryan (intrinsic) angles
        Rotation(double phi, vec3 axis); //axis-angle representation
        Rotation(quaternion q);
        Rotation(); 

        template<typename V> V rotate(V v);
};

struct Photon {
    vec3 pos;
    vec3 vel;
    double c;
    Photon(vec3 position=vec3(0,0,0), vec3 velocity=vec3(1,0,0)){
        pos = position;
        vel = velocity;
    }
};

struct Camera {
    public:
        int resolution_v;
        int resolution_h;
        double FOV;
        vec3 pos;
        Rotation rot;
        png::image<png::rgb_pixel> image;

        Camera(vec3 position, Rotation orientation, int xres=1024, int yres=1024, double fov=M_PI/2) {
            rot = orientation;
            pos = position;    
            FOV=fov;
            resolution_v = xres;
            resolution_h = yres;
            image = png::image<png::rgb_pixel>(xres, yres);
        }

        Photon emit_photon(int img_x, int img_y, double speed_of_light=1);
};

#include "3d.cpp"
#endif //_3D_H_
