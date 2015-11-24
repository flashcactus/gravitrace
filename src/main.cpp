#include "lib/glm/glm.hpp"
#include "lib/pngpp/png.hpp"
#include "3d.h"
#include <iostream>
#include <cstdio>

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
        int x = round((texture.get_height()-1)*(point.x/(2*radius) + 0.5));
        int y = round((texture.get_width()-1)*(point.y/(2*radius) + 0.5));
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


template<typename T> T min(T x, T y) {if(x<y){ return x; }else{ return y;}}
png::rgb_pixel trace_photon(Photon p, Scene &scene, double min_tick=1.0, double dyn_tick_power=0, double dyn_tick_max_factor = 5, unsigned maxsteps = 1000){
    unsigned ctr;
    double dt, rad;
    vec3 newpos;
    vec3 a, dv0, h;
    static long total_steps=0;
    static int total_calls=0;
    total_calls++;
    png::rgb_pixel px(0,0,0);
    for(ctr = 0; ctr < maxsteps; ++ctr) {//cancel if too far away and going outwards or running for too long
        rad = abs(p.pos);
        dt = min_tick * pow(min(rad/scene.hole->radius, dyn_tick_max_factor), dyn_tick_power);//dt is proportional to DTP'th power of radius, but not larger than (DTMF*radius^DTP)
        newpos = p.pos + mul_vec(p.vel, dt);
        
        if ( newpos.z * p.pos.z <= 0 ) { //intersects XY plane
            double dz = (newpos-p.pos).z;
            vec3 isect = mul_vec(newpos, fabs(p.pos.z / dz)) //find intersection point
                + mul_vec(p.pos, fabs(newpos.z / dz));
            double isec_rad=sqrt(isect.x*isect.x + isect.y*isect.y);
            if(isec_rad < scene.hole->radius) {//goes into hole before intersection
                break;
            } else if(isec_rad < scene.disk->radius) { //hits actual disk
                px = scene.disk->get_pixel(isect);
                break;
            }
        }
        if ( abs(newpos) < scene.hole->radius || 
                abs(newpos-p.pos) > 
                ( sqrt(dotprod(newpos,newpos) - scene.hole->sqradius) +
                  sqrt(dotprod(p.pos, p.pos ) - scene.hole->sqradius) ) 
            ){//intersects hole
            break;
        }
        //calculate new position
        p.pos = newpos;
        //new velocity
        dv0 = mul_vec( normalize(p.pos), dt * scene.hole->GM / (-rad*rad) );
        h = p.vel + div_vec(dv0, 2.0);
        p.vel += dv0 - mul_vec(h, dotprod(dv0,h)/dotprod(h,h));
        p.vel = normalize(p.vel);
        
        if (dotprod(p.vel,p.pos) > 0 && abs(p.pos) > 2*scene.disk->radius) {//going outwards, away from everything
            break;    
        }
    }
    total_steps += ctr;
    if(total_calls == scene.cam->resolution_h * scene.cam->resolution_v) {cerr<<total_steps/total_calls<<endl;}
    return px;
}

int main(int argc, char ** argv) {
    if(argc<2) {
        cerr<<"Usage: "<<argv[0]<<" <config_file>"<<endl;
        return 65;
    }
    FILE* inf = fopen(argv[1], "r");
    double GM,x,y,z,yaw,pitch,roll;
    int xres,yres,nparam=0;
    nparam += fscanf(inf,"%lf\n",&GM);
    nparam += fscanf(inf,"%lf %lf %lf\n",&x,&y,&z);
    nparam += fscanf(inf,"%lf %lf %lf\n",&yaw,&pitch,&roll);
    nparam += fscanf(inf,"%d %d\n",&xres,&yres);
    if(nparam<9) {
        cerr<<"Bad config file."<<endl;
        fclose(inf);
        return 65;    
    }
    char* ofname;
    char buf[2048];
    char default_name[] = "out.png";
    if(0 == fscanf(inf, "%2048s", buf)) {
        ofname = default_name;
    } else {
        ofname = buf;    
    }
    fclose(inf);
    Camera c(vec3(x,y,z), Rotation((PI/180)*yaw, (PI/180)*pitch, (PI/180)*roll), xres, yres);
    
    BlackHole hole(GM);
    AccretionDisk d(5*hole.radius, png::image<png::rgb_pixel>("textures/disk_24.png"));
    Scene scene(&c, &hole, &d);

    for (int x=0; x<c.resolution_v; x++){
        for (int y=0; y<c.resolution_h; y++){
            c.image[x][y] = trace_photon(c.emit_photon(x,y),scene, 1.0, 4.0, 5, round(abs(c.pos)));
        }
        cerr<<'.';
    }
    cerr<<endl;
    
    c.image.write(ofname);
}
