#include "lib/glm/glm.hpp"
#include "lib/pngpp/png.hpp"
#include "3d.h"
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

const double SI_c = 3e8;
//coordinates are in units of light seconds, hence stretched by c

struct BlackHole { 
    double GM_metric;
    double GM;
    double radius;
    double sqradius;
    BlackHole(double metric_GM) {
        GM_metric = metric_GM;
        GM = GM_metric/(SI_c * SI_c * SI_c);
        radius = 2*GM; //in local units(speed of light = 1)
        sqradius = radius*radius;
    }
};

struct AccretionDisk {
    double radius;
    png::image<png::rgb_pixel> texture;
    png::rgb_pixel get_pixel (vec3 point) {
        int x = round(texture.get_height()*(point.x/(2*radius) + 0.5));
        int y = round(texture.get_width()*(point.y/(2*radius) + 0.5));
        return texture[x][y];
    }
    AccretionDisk(double r, png::image<png::rgb_pixel> tx) {
        radius = r;
        texture = tx;
    }
};

struct Scene {
        Camera* cam;
        BlackHole* hole;
        AccretionDisk* disk;
        Scene(Camera* c, BlackHole* h, AccretionDisk* d){
            cam = c;
            hole = h;
            disk = d;    
        }
};

png::rgb_pixel trace_photon(Photon p, Scene &scene, double dt=1.0, unsigned maxsteps = 1000){
    unsigned ctr;
    vec3 newpos;
    vec3 dv0, h;
    for(ctr = 0; ctr < maxsteps; ++ctr) {//cancel if too far away and going outwards or running for too long
        newpos = p.pos + mul_vec(p.vel, dt);
        if ( abs(newpos) < scene.hole->radius /*|| 
                abs(newpos-p.pos) > 
                ( sqrt(dotprod(newpos,newpos) - scene.hole->sqradius) +
                  sqrt(dotprod(p.pos, p.pos ) - scene.hole->sqradius) ) */
            ){//intersects hole
            return png::rgb_pixel(0,0,0);    
        }
        if ( newpos.z * p.pos.z < 0 ) { //intersects XY plane
            double dz = (newpos-p.pos).z;
            vec3 isect = mul_vec(newpos, fabs(p.pos.z / dz)) //find intersection point
                + mul_vec(p.pos, fabs(newpos.z / dz));
            if(sqrt(isect.x*isect.x + isect.y*isect.y) < scene.disk->radius) { //hits actual disk
                return scene.disk->get_pixel(isect);
            }
        }
        //calculate new position
        p.pos = newpos;
        //new velocity
        dv0 = mul_vec( normalize(p.pos), dt * scene.hole->GM / -dotprod(p.pos,p.pos) );
        h = p.vel + div_vec(dv0, 2.0);
        p.vel += dv0 - mul_vec(h, dotprod(dv0,h)/dotprod(h,h));
        p.vel = normalize(p.vel);
        
        if (dotprod(p.vel,p.pos) > 0 && abs(p.pos) > 2*scene.disk->radius) {//going outwards, away from everything
            break;    
        }
    }
    return png::rgb_pixel(16,16,16);
}

int main() {
    vec3 X(1,0,0);
    vec3 Y(0,1,0);
    vec3 Z(0,0,1);
    Rotation rot(0, -PI/8, 0);

    Camera c(vec3(-6000,0,2000), rot, 512,512);
    
    print_vec3(rot.rotate(X),"\n");
    print_vec3(rot.rotate(Y),"\n");
    print_vec3(rot.rotate(Z),"\n\n");
    
    BlackHole hole(1.32e28);
    cout<<hole.radius<<endl;
    AccretionDisk d(5*hole.radius, png::image<png::rgb_pixel>("disk_24.png"));
    Scene scene(&c, &hole, &d);

    int holerad = 40;
    int ctr;
    vec3 newpos, isect;
    for (int x=0; x<c.resolution_v; x++){
        for (int y=0; y<c.resolution_h; y++){
            c.image[x][y] = trace_photon(c.emit_photon(x,y),scene, 10.0);
        }
    }
    
    c.image.write("out.png");
}
