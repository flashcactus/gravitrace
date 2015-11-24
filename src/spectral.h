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
    IntTable(char* fname) {
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
            int left, right;
            for(int i=0; i<max_wvlen; ++i) {
                newwl = i*wvlen_step/factor;//reverse
                newspec.values[i] = this->getwl(newwl);
            }
            return newspec;
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

        Spectre& operator+=(Spectre s){
            for(int i=0; i<max_wvlen; ++i) {
                values[i] += s.values[i];
            }
            return *this;
        }
        
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
            return png::rgb_pixel(static_cast<int>(r*norm_mul), static_cast<int>(g*norm_mul), static_cast<int>(b*norm_mul));
        }
};

class SpectralImage{
    private:
        Spectre *** pixels;
        unsigned x_res;
        unsigned y_res;
    public:
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
            x_res = imgs[0].get_height();
            y_res = imgs[0].get_width();
            pixels = static_cast<Spectre***>(malloc(x_res * sizeof(Spectre**)));//allocate row pointers
            for(int x=0; x<x_res; ++x) {//allocate rows
                pixels[x] = static_cast<Spectre**>(malloc(y_res * sizeof(Spectre*)));
                for(int y=0; y<y_res; ++y) {//allocate elements                
                    pixels[x][y] = new Spectre;
                    for(int i=0; i<il; ++i) {//fill spectre
                        pixels[x][y]->values[i0+i] = imgs[i][x][y];
                    }
                }
            }
        }
        
        unsigned get_height(){return x_res;}
        unsigned get_width(){return y_res;}

        ~SpectralImage(){
            for(int x=0; x<x_res; ++x) {//free rows
                for(int y=0; y<y_res; ++y) {//free elements                
                    delete pixels[x][y];
                }
                free(pixels[x]);
            }
            free(pixels);
        }

        Spectre& getpx(int x, int y) {
            return *(pixels[x][y]);
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

/*//for testing
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
    rgb_im.write("rgb.png");
}
*/

#endif
