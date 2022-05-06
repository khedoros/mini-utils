#include<iostream>
#include<SDL2/SDL.h>
#include<cstdint>
#include<ctgmath>
#include<cstring>

using std::cout,std::cerr,std::endl;

const int SCR_SIZE = 50;
const int BYTES_PER_PX = 4;

template <class T>
class complex {
    public:
        T real;
        T imag;
        complex(T r, T i) : real(r), imag(i) {
        }

        complex<T> operator+(const complex<T>& r) {
            return complex<T>(real+r.real, imag+r.imag);
        }

        complex<T> operator*(const complex<T>& r) {
            return complex<T>(real * r.real - imag * r.imag, real * r.imag + r.real * r.imag);
        }
        T abs() {
            return sqrt(real*real+imag*imag);
        }
};


void shutdown(SDL_Surface ** s, SDL_Renderer ** r, SDL_Window ** w) {
    if(*s) {
        SDL_FreeSurface(*s);
        *s = NULL;
    }
    if(*r) {
        SDL_DestroyRenderer(*r);
        *r = NULL;
    }
    if(*w) {
        SDL_DestroyWindow(*w);
        *w = NULL;
    }
    SDL_Quit();
}

void update_screen(SDL_Renderer * rend, SDL_Surface * surf) {
    if(!rend || !surf) {
        cerr<<"Renderer or surface is non-existent"<<endl;
        return;
    }
    SDL_Texture * text = SDL_CreateTextureFromSurface(rend, surf);
    if(!text) {
        cerr<<"Texture not created appropriately"<<endl;
        cerr<<SDL_GetError()<<endl;
        return;
    }
    SDL_RenderClear(rend);
    SDL_RenderCopy(rend, text, NULL,  NULL);
    SDL_DestroyTexture(text);
    SDL_RenderPresent(rend);
}

void pset(SDL_Surface * surf, int x, int y, uint32_t col) {
    if(x>=surf->w) {
        cout<<"X of "<<x<<" is too large."<<endl;
        return;
    }
    if(y>=surf->h) {
        cout<<"Y of "<<y<<" is too large."<<endl;
        return;
    }
    if(x<0) { cout<<"X is "<<x<<endl;}
    if(y<0) { cout<<"Y is "<<y<<endl;}
    uint32_t * pixels = (uint32_t *)(surf->pixels);
    int pitch = surf->pitch;
    pixels[y*(pitch/BYTES_PER_PX)+x] = col;
    return;
}

uint32_t col(uint32_t r, uint32_t g, uint32_t b) {
    return 0xff000000 | (b&0xff) << (16) | (g&0xff) << (8) | (r&0xff);
}

void clear(SDL_Surface * surf, uint32_t col) {
    for(int i=0;i<SCR_SIZE; i++) {
        for(int j=0;j<SCR_SIZE;j++) {
            ((uint32_t *)(surf->pixels))[i*(surf->pitch/BYTES_PER_PX)+j] = col;
        }
    }
}

void startup(SDL_Surface ** s, SDL_Renderer ** r, SDL_Window ** w) {
    SDL_Init(SDL_INIT_VIDEO);
    *w = SDL_CreateWindow("Mandelbrot Render", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_SIZE, SCR_SIZE, 0);
    if(!*w) {
        cerr<<"Window creation failed."<<endl;
        cerr<<SDL_GetError()<<endl;
        return;
    }
    int wid, hei;
    SDL_GetWindowSize(*w, &wid, &hei);
    cout<<"Window size: "<<wid<<", "<<hei<<endl;
    *r = SDL_CreateRenderer(*w, -1, SDL_RENDERER_SOFTWARE);
    if(!*r) {
        cerr<<"Renderer creation failed."<<endl;
        cerr<<SDL_GetError()<<endl;
        return;
    }

    //We'll be unhappy on a big-endian system. ARM and X86 are little-endian, though ;-)
    uint32_t rmask, gmask, bmask, amask;
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    *s = SDL_CreateRGBSurface(0, SCR_SIZE, SCR_SIZE, 32, rmask, gmask, bmask, amask);
    if(!*s) {
        cerr<<"Surface creation failed."<<endl;
        cerr<<SDL_GetError()<<endl;
        return;
    }
}

int dist(double i, double j) {
    complex<double> accum(0,0);
    complex<double> pt(i,j);
    int iter=0;
    while(accum.abs()< 2.0 && iter < 256) {
        accum = accum * accum + pt;
        iter++;
    }
	if(iter == 256) return 255;
    return iter;

}

int main() {
    SDL_Renderer * rend;
    SDL_Surface  * surf;
    SDL_Window   * wind;

    startup(&surf, &rend, &wind);
    clear(surf, col(0,0,0));

	// Set palette entries
    uint32_t pal[256];
    for(int i=0;i<256;i++) pal[i] = col(i,i,i);

    for(double j = -2.0; j < 0.0; j += (4.0/static_cast<double>(SCR_SIZE))) {
        for(double i = -2.0; i < 2.0; i += (4.0/static_cast<double>(SCR_SIZE))) {
            int distance = dist(i,j) % 256;
            pset(surf, (i+2.0)*(SCR_SIZE/4), (j+2.0)*(SCR_SIZE/4), pal[distance]);
        }
    }
    int pitch=surf->pitch;
    for(int j=SCR_SIZE/2;j<SCR_SIZE;j++) {
        uint32_t * dat = (uint32_t *)(surf->pixels);
        uint32_t * src =  &dat[0+(SCR_SIZE - j)*pitch/BYTES_PER_PX];
        uint32_t * dst =  &dat[0+j*pitch/BYTES_PER_PX];
        memcpy(dst, src, pitch);
    }

    update_screen(rend, surf);
    SDL_Delay(10000);
    shutdown(&surf, &rend, &wind);
}

