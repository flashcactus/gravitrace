#include "lib/glm/glm.hpp"
#include "lib/pngpp/png.hpp"
#include "3d.h"
#include "spectral.h"
#include <iostream>
#include <cstdio>

using std::cerr;
using std::cout;
using std::endl;

const double SI_c = 3e8;
//coordinates are in units of light seconds, hence stretched by c

enum filtering{NEAREST_NEIGH, BILINEAR};

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

Spectre getpx_bilinear (SpectralImage& texture, double x, double y){
    unsigned xf = static_cast<unsigned>(floor(x)); 
    unsigned yf = static_cast<unsigned>(floor(y)); 
    unsigned xc = (xf+1)%(texture.get_height()-1);
    unsigned yc = (yf+1)%(texture.get_width()-1);
    double xr = x-xf;           
    double yr = y-yf;           
    Spectre px = texture.getpx(xc,yc)*(xr*yr);
    px += texture.getpx(xc,yf)*(xr*(1-yr));
    px += texture.getpx(xf,yc)*(yr*(1-xr));
    px += texture.getpx(xf,yf)*((1-xr)*(1-yr));
    return px;
}


png::gray_pixel getpx_bilinear (png::image<png::gray_pixel>& texture, double x, double y){
    unsigned xf = floor(x); 
    unsigned yf = floor(y); 
    unsigned xc = (xf+1)%(texture.get_height()-1);
    unsigned yc = (yf+1)%(texture.get_width()-1);
    double xr = x-xf;
    double yr = y-yf;
    png::gray_pixel px = texture[xc][yc]*xr*yr;
    px += texture[xc][yf]*xr*(1-yr);
    px += texture[xf][yc]*yr*(1-xr);
    px += texture[xf][yf]*(1-xr)*(1-yr);
    return px;
}

struct AccretionDisk {
    double radius;
    SpectralImage texture;
    png::image<png::gray_pixel> alpha;
    enum filtering filter;
    Spectre get_pixel (vec3 point) {
        double x = (texture.get_height()-1)*(point.x/(2*radius) + 0.5);
        double y = (texture.get_width()-1)*(point.y/(2*radius) + 0.5);
        if(filter==NEAREST_NEIGH) {
            return texture.getpx(round(x),round(y));
        } else if (filter==BILINEAR) {
            return getpx_bilinear(texture, x, y);
        }
    }
    png::gray_pixel get_alpha (vec3 point) {
        double x = (alpha.get_height()-1)*(point.x/(2*radius) + 0.5);
        double y = (alpha.get_width()-1)*(point.y/(2*radius) + 0.5);
        if(filter==NEAREST_NEIGH) {
            return alpha[round(x)][round(y)];
        } else if (filter==BILINEAR) {
            return getpx_bilinear(alpha, x, y);
        }
    }
    AccretionDisk(double r, SpectralImage tx, png::image<png::gray_pixel> alp, enum filtering fil=NEAREST_NEIGH){
        radius = r;
        texture = tx;
        alpha = alp;
        filter=fil;
    }
};

struct StarField {
    SpectralImage texture;
    enum filtering filter;
    Spectre get_pixel (vec3 velocity) {
        double ptc = asin(velocity.z)/PI;
        double yaw = atan2(velocity.x, velocity.y)/(2*PI);
        double x = (texture.get_height()-1)*(0.5-ptc);
        double y = (texture.get_width()-1)*(0.5+yaw) ;
        while (y>=texture.get_width()) {y-=texture.get_width();}
        if(filter==NEAREST_NEIGH) {
            int xx=static_cast<int>(round(x));
            int yy=static_cast<int>(round(y));
            return texture.getpx(xx,yy);
        } else if (filter==BILINEAR) {
            return getpx_bilinear(texture, x, y);
        }
    }
    StarField(SpectralImage t, enum filtering fil=NEAREST_NEIGH) {texture = t; filter=fil;}
};

struct Scene {
    Camera* cam;
    BlackHole* hole;
    AccretionDisk* disk;
    StarField* stars;
    Scene(Camera* c, BlackHole* h, AccretionDisk* d, StarField* f){
        cam = c;
        hole = h;
        disk = d;
        stars = f;
    }
};

double redshift_factor(double sch_rad, double src_rad, double dest_rad) {return sqrt((1/sch_rad - 1/dest_rad) / (1/sch_rad - 1/src_rad));}
template<typename T> T min(T x, T y) {if(x<y){ return x; }else{ return y;}}
void trace_photons(Scene &scene, double min_tick=1.0, double dyn_tick_power=0, double dyn_tick_max_factor = 5, unsigned maxsteps = 1000, bool enable_redshift=true){
    unsigned ctr,total_steps=0;
    double dt, rad;
    vec3 newpos;
    vec3 a, dv0, h;
    Spectre px, black_px, addpx;
    Photon p;
    double alpha_left, redfact;
    cerr<<"Rendering";
    for (int x=0; x<scene.cam->resolution_v; x++){
        for (int y=0; y<scene.cam->resolution_h; y++){
            p = scene.cam->emit_photon(x,y);
            px = black_px;//reset pixel
            alpha_left = 1;
            for(ctr = 0; ctr < maxsteps; ++ctr) {//cancel if too far away and going outwards or running for too long
                rad = abs(p.pos);
                dt = min_tick * pow(min(rad/scene.hole->radius, dyn_tick_max_factor), dyn_tick_power);//dt is proportional to DTP'th power of radius, but not larger than (DTMF*radius^DTP)
                newpos = p.pos + mul_vec(p.vel, dt); //move the photon
                
                if ( newpos.z * p.pos.z <= 0 ) { //intersects XY plane
                    double dz = (newpos-p.pos).z;
                    vec3 isect = mul_vec(newpos, fabs(p.pos.z / dz)) //find intersection point
                        + mul_vec(p.pos, fabs(newpos.z / dz));
                    double isec_rad=sqrt(isect.x*isect.x + isect.y*isect.y);
                    if(isec_rad < scene.hole->radius) {//goes into hole before intersection
                        break;
                    } else if(isec_rad < scene.disk->radius) { //hits actual disk
                        redfact = enable_redshift ? redshift_factor(scene.hole->radius, abs(isect), abs(scene.cam->pos)) : 1;
                        double used_alpha = scene.disk->get_alpha(isect)*alpha_left/255;
                        px += scene.disk->get_pixel(isect).shifted(redfact) * used_alpha;
                        alpha_left -= used_alpha;
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
                    redfact = enable_redshift ? redshift_factor(scene.hole->radius, INFINITY, abs(scene.cam->pos)) : 1;
                    px += scene.stars->get_pixel(p.vel).shifted(redfact) * alpha_left;
                    break;    
                }
            }
            scene.cam->image.getpx(x,y) = px;
            total_steps += ctr;
        }
        cerr<<'.';
    }
    cerr<<"Done."<<endl;
    cerr<<"Avg steps/px: "<<total_steps/(scene.cam->resolution_h * scene.cam->resolution_v)<<endl;
}

const char disk_texture_spectral_fname_fmt[]="textures/spectral/disk/%d.png";
const char disk_texture_alpha_fname[]="textures/disk_alpha.png";
const char star_texture_spectral_fname_fmt[]="textures/spectral/stars/%d.png";
const char integration_table_fname[]="textures/spectral/cie_xyz.txt";
const int texture_wvlen_first=380, texture_wvlen_last=700;


const double tracer_step_min_ratio=2e-3;
const double tracer_step_pow=4;
const double tracer_step_maxratio=5.0;

const double spectral_rgb_norm_mul=0.07;

int main(int argc, char ** argv) {
    if(argc<2) {
        cerr<<"Usage: "<<argv[0]<<" <config_file>"<<endl;
        return 65;
    }
    FILE* inf = fopen(argv[1], "r");
    double GM, x, y, z, yaw, pitch, roll, cam_fov, disk_size_ratio;
    int xres, yres, apply_redshift, use_bilinear_filtering, nparam=0;
    nparam += fscanf(inf,"%lf %lf\n",&GM,&disk_size_ratio);
    nparam += fscanf(inf,"%lf %lf %lf\n",&x,&y,&z);
    nparam += fscanf(inf,"%lf %lf %lf\n",&yaw,&pitch,&roll);
    nparam += fscanf(inf,"%lf %d %d\n",&cam_fov ,&xres,&yres);
    nparam += fscanf(inf,"%d %d\n",&apply_redshift,&use_bilinear_filtering);
    if(nparam<12) {
        cerr<<"Bad config file."<<endl;
        fclose(inf);
        return 65;    
    }
    enum filtering texture_filtering = use_bilinear_filtering ? BILINEAR : NEAREST_NEIGH;

    char* ofname;
    char buf[2048];
    char default_name[] = "out.png";
    if(0 == fscanf(inf, "%2048s", buf)) {
        ofname = default_name;
    } else {
        ofname = buf;    
    }
    fclose(inf);
    
    Camera cam(vec3(x,y,z), Rotation((M_PI/180)*yaw, (M_PI/180)*pitch, (M_PI/180)*roll), xres, yres, cam_fov*M_PI/180);
    BlackHole hole(GM);
    cerr<<"Schwarzschild radius: "<<hole.radius<<" LS"<<endl;

    cerr<<"Loading textures.";
    SpectralImage disk_texture(texture_wvlen_first, texture_wvlen_last, disk_texture_spectral_fname_fmt);
    AccretionDisk accd(
        hole.radius * disk_size_ratio, 
        disk_texture,
        png::image<png::gray_pixel>("textures/disk_alpha.png"),
        texture_filtering
    );
    cerr<<"."; 
    SpectralImage star_texture(texture_wvlen_first, texture_wvlen_last, star_texture_spectral_fname_fmt);
    StarField stars(star_texture, texture_filtering);
    cerr<<".Done."<<endl;
    
    Scene scene(&cam, &hole, &accd, &stars);
     
    double tracer_step_min = tracer_step_min_ratio * hole.radius;
    int max_steps = 5*round(abs(cam.pos)/tracer_step_min);
    
    trace_photons(scene, tracer_step_min, tracer_step_pow, tracer_step_maxratio, max_steps, apply_redshift);
    
    IntTable integ_tbl(integration_table_fname);
    cerr<<"Saving image to \""<<ofname<<"\"..."<<endl;
    cam.image.toRGB(integ_tbl,spectral_rgb_norm_mul).write(ofname);
}
