#ifndef _SPECTRAL_H
#define _SPECTRAL_H

#include "lib/pngpp/png.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>

const int wvlen_step = 5;
const int max_wvlen = 200;//1000 nm

struct Spectre;

struct IntTable{
    double x[max_wvlen];    
    double y[max_wvlen];    
    double z[max_wvlen];    
    IntTable(const char* fname) {
        FILE* f = fopen(fname, "r");
        double cx, cy, cz;
        int wlen, i;
        while(4 == fscanf(f, "%d %lf %lf %lf\n", &wlen ,&cx, &cy, &cz)) {
            i=wlen/wvlen_step;
            x[i] = cx;
            y[i] = cy;
            z[i] = cz;
        }
        fclose(f);
    }
};

struct Spectre {
    public:
        double values[max_wvlen];
        Spectre shifted(double factor){
            Spectre newspec;
            double newwl;
            for(int i=0; i<max_wvlen; ++i) {
                newwl = i*wvlen_step/factor;//reverse
                newspec.values[i] = this->getwl(newwl);
            }
            return newspec;
        }

        Spectre(){
            for(int i=0; i<max_wvlen; ++i) {
                values[i] = 0;
            }
        }
        
        double getwl(double wl){
            double x = wl/wvlen_step;
            if(x > max_wvlen || x < 0) return 0;
            int left = floor(x);
            int right = ceil(x);
            double xx = x-left;  
            return values[left]*(1-xx) + values[right]*xx;
        }

        Spectre& operator*=(double mul){
            for(int i=0; i<max_wvlen; ++i) {
                values[i] *= mul;
            }
            return *this;
        }
        
        friend Spectre operator*(const Spectre& s, double mul);

        Spectre& operator+=(const Spectre& s){
            for(int i=0; i<max_wvlen; ++i) {
                values[i] += s.values[i];
            }
            return *this;
        }

        friend Spectre operator+(const Spectre&, const Spectre);
        
        png::rgb_pixel to_rgb(const IntTable &t, double norm_mul=0.06) {
            double x,y,z,r,g,b;
            x=y=z=0;
            for(int i=0;i<max_wvlen;++i){
                x += values[i] * t.x[i];
                y += values[i] * t.y[i];
                z += values[i] * t.z[i];
            }
            r= x*3.2404542 - y*1.5371385 - z*0.4985314;
            g=-x*0.9692660 + y*1.8760108 + z*0.0415560;
            b= x*0.0556434 - y*0.2040259 + z*1.0572252;
            int rr, gg, bb;
            rr = r*norm_mul;
            gg = g*norm_mul;
            bb = b*norm_mul;
            rr = (rr>255)?255:rr;
            rr = (rr<0)?0:rr;
            gg = (gg>255)?255:gg;
            gg = (gg<0)?0:gg;
            bb = (bb>255)?255:bb;
            bb = (bb<0)?0:bb;
            return png::rgb_pixel(rr, gg, bb);
        }
};

Spectre operator*(const Spectre& self, double mul) {
    Spectre result=self;
    result*=mul;
    return result;
}

Spectre operator+(const Spectre& self, const Spectre& other) {
    Spectre result=self;
    result+=other;
    return result;
}

class SpectralImage{
    private:
        Spectre *** pixels;
        unsigned x_res;
        unsigned y_res;
        void allocate(int xres, int yres) {
            x_res = xres;
            y_res = yres;
            pixels = static_cast<Spectre***>(malloc(x_res * sizeof(Spectre**)));//allocate row pointers
            for(int x=0; x<x_res; ++x) {//allocate rows
                pixels[x] = static_cast<Spectre**>(malloc(y_res * sizeof(Spectre*)));
                for(int y=0; y<y_res; ++y) {//allocate elements                
                    pixels[x][y] = new Spectre;
                }
            }
        }
    public:
        SpectralImage() {
            x_res = 0;
            y_res = 0;

        }
        SpectralImage(int yres, int xres) {
            allocate(xres, yres);
        }
        SpectralImage(int start, int end, const char* format) {
            if(start%wvlen_step || end%wvlen_step) {
                throw "endpoints do not divide!"    ;
            }
            int i0 = start/wvlen_step;
            int il = (end-start)/wvlen_step;

            png::image<png::gray_pixel> imgs[il];
            char filename[2048];
            for(int i=0; i<il; ++i){
                sprintf(filename, format, (i+i0)*wvlen_step);
                imgs[i] = png::image<png::gray_pixel>(filename);
            }
            allocate(imgs[0].get_height(),imgs[0].get_width());//allocate space
            for(int x=0; x<x_res; ++x){
                for(int y=0; y<y_res; ++y){
                    for(int i=0; i<il; ++i) {//fill spectre
                        pixels[x][y]->values[i0+i] = imgs[i][x][y];
                    }
                }
            }
        }
        
        unsigned get_height(){return x_res;}
        unsigned get_width(){return y_res;}
        /*
        ~SpectralImage(){
            if(instances && *instances) {--(*instances); return;}
            for(int x=0; x<x_res; ++x) {//free rows
                for(int y=0; y<y_res; ++y) {//free elements                
                    delete pixels[x][y];
                }
                free(pixels[x]);
            }
            free(pixels);
        }*/

        Spectre& getpx(int x, int y) {
            Spectre* px = pixels[x][y];
            return *px;
        }
        
        png::image<png::rgb_pixel> toRGB(const IntTable &t, double norm_mul=0.06){
            png::image<png::rgb_pixel> oi(y_res, x_res);
            for(int x=0;x<x_res;++x){
                for(int y=0;y<y_res;++y){
                    oi[x][y] = pixels[x][y]->to_rgb(t, norm_mul);
                }
            }
            return oi;
        }
};

//for testing
/*
int main(){
    png::image<png::gray_pixel> imgs[65];
    char filename[2048];
    Spectre s;
    int i=0;
    char disktext_fmt[] ="textures/spectral/stars/%d.png";
    char integ_fname[] = "textures/spectral/cie_xyz.txt";
    SpectralImage si(380,700,disktext_fmt);
    IntTable it(integ_fname);
    for(i=0;i<200;++i) fprintf(stderr,"t %lf %lf %lf\n", it.x[i],it.y[i],it.z[i]);
    png::image<png::rgb_pixel> rgb_im = si.toRGB(it,0.065);
    si.getpx(700,1400);
    rgb_im.write("rgb.png");
}
*/
#endif
