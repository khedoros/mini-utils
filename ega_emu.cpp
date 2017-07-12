#include<SDL2/SDL.h>
#include<iostream>
#include<fstream>
#include<bitset>
#include<vector>
#include<cstdint>
#include<array>

using namespace std;

class ega {
public:
    ega() {
        bitplanes.resize(0x10000);
        SDL_Init(SDL_INIT_VIDEO);
        wind = SDL_CreateWindow("EGA Screen", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
        if(!wind) {
            cerr<<"Window creation failed."<<endl;
            cerr<<SDL_GetError()<<endl;
            return;
        }
        rend = SDL_CreateRenderer(wind, -1, SDL_RENDERER_SOFTWARE);
        if(!rend) {
            cerr<<"Renderer creation failed."<<endl;
            cerr<<SDL_GetError()<<endl;
            return;
        }
        surf = SDL_CreateRGBSurface(0, 320, 200, 8, 0, 0, 0, 0);
        if(!surf) {
            cerr<<"Surface creation failed."<<endl;
            cerr<<SDL_GetError()<<endl;
            return;
        }
        if(surf->format->palette && SDL_SetPaletteColors(surf->format->palette, &ega_cols[0], 0, 64) != 0) {
            cerr<<"Setting palette colors failed."<<endl;
            cerr<<SDL_GetError()<<endl;
            return;
        }
        for(int i=0;i<200;i++) {
            for(int j=0;j<320;j++) {
                raw_pset(j, i, j % 64);
                //((char *)(surf->pixels))[i*(320+(surf->pitch)) + j] = j % 64;
            }
        }
        text = NULL;
        if(SDL_MUSTLOCK(surf)) {
            cout<<"Remember to keep your surfaces all locked up!"<<endl;
        }
        update_screen();
    }

    ~ega() {
        if(surf) {
            //SDL_FreeSurface(surf);
            surf = NULL;
        }
        if(text) {
            SDL_DestroyTexture(text);
            text = NULL;
        }
        if(rend) {
            SDL_DestroyRenderer(rend);
            rend = NULL;
        }
        if(wind) {
            SDL_DestroyWindow(wind);
            wind = NULL;
        }
        SDL_Quit();
    }

    void set_planes(unsigned int read, unsigned int write) {
        read_plane = read & 0x03;
        write_plane = read & 0x03;
    }

    void set_ptr(unsigned int ptr) {
        disp_ptr = ptr & 0xffff;
    }

    void set_pal(unsigned int index, unsigned int palnum) {
        palmap[index&0xf] = (palnum & 0x3f);
    }

    void raw_pset(int x, int y, int col) {
        col %= 64;
        if(x>=surf->w) {
            cout<<"X of "<<x<<" is too large."<<endl;
            return;
        }
        if(y>=surf->h) {
            cout<<"Y of "<<y<<" is too large."<<endl;
            return;
        }
        char * pixels = (char *)(surf->pixels);
        int pitch = surf->pitch;
        pixels[y*pitch+x] = char(col);
        return;
    }

    void pset(int x, int y, int col) {
        col %= 16;
        if(x>=surf->w) {
            cout<<"X of "<<x<<" is too large."<<endl;
            return;
        }
        if(y>=surf->h) {
            cout<<"Y of "<<y<<" is too large."<<endl;
            return;
        }
        char * pixels = (char *)(surf->pixels);
        int pitch = surf->pitch;
        pixels[y*pitch+x] = palmap[char(col)];
        return;
    }

    void plane_blit(const vector<uint8_t>& bytes) {
        if(bytes.size() > 0x10000) return;
        for(int i=0;i<bytes.size();i++) {
            bitplanes[(disp_ptr+i) % 0x10000][write_plane] = bytes[i];
        }
    }

    void refresh() {
        bits_to_surf();
        update_screen();
    }

private:
    int read_plane = 0;
    int write_plane = 0;
    int disp_ptr = 0x0000;
    vector<std::array<uint8_t, 4>> bitplanes;
    int palmap[16] {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};
    SDL_Color ega_cols[ 64] {SDL_Color{0x00,0x00,0x00,0xff}, SDL_Color{0x00,0x00,0xaa,0xff}, SDL_Color{0x00,0xaa,0x00,0xff}, SDL_Color{0x00,0xaa,0xaa,0xff},
                             SDL_Color{0xaa,0x00,0x00,0xff}, SDL_Color{0xaa,0x00,0xaa,0xff}, SDL_Color{0xaa,0xaa,0x00,0xff}, SDL_Color{0xaa,0xaa,0xaa,0xff},
                             SDL_Color{0x00,0x00,0x55,0xff}, SDL_Color{0x00,0x00,0xff,0xff}, SDL_Color{0x00,0xaa,0x55,0xff}, SDL_Color{0x00,0xaa,0xff,0xff},
                             SDL_Color{0xaa,0x00,0x55,0xff}, SDL_Color{0xaa,0x00,0xff,0xff}, SDL_Color{0xaa,0xaa,0x55,0xff}, SDL_Color{0xaa,0xaa,0xff,0xff},
                             SDL_Color{0x00,0x55,0x00,0xff}, SDL_Color{0x00,0x55,0xaa,0xff}, SDL_Color{0x00,0xff,0x00,0xff}, SDL_Color{0x00,0xff,0xaa,0xff},
                             SDL_Color{0xaa,0x55,0x00,0xff}, SDL_Color{0xaa,0x55,0xaa,0xff}, SDL_Color{0xaa,0xff,0x00,0xff}, SDL_Color{0xaa,0xff,0xaa,0xff},
                             SDL_Color{0x00,0x55,0x55,0xff}, SDL_Color{0x00,0x55,0xff,0xff}, SDL_Color{0x00,0xff,0x55,0xff}, SDL_Color{0x00,0xff,0xff,0xff},
                             SDL_Color{0xaa,0x55,0x55,0xff}, SDL_Color{0xaa,0x55,0xff,0xff}, SDL_Color{0xaa,0xff,0x55,0xff}, SDL_Color{0xaa,0xff,0xff,0xff},
                             SDL_Color{0x55,0x00,0x00,0xff}, SDL_Color{0x55,0x00,0xaa,0xff}, SDL_Color{0x55,0xaa,0x00,0xff}, SDL_Color{0x55,0xaa,0xaa,0xff},
                             SDL_Color{0xff,0x00,0x00,0xff}, SDL_Color{0xff,0x00,0xaa,0xff}, SDL_Color{0xff,0xaa,0x00,0xff}, SDL_Color{0xff,0xaa,0xaa,0xff},
                             SDL_Color{0x55,0x00,0x55,0xff}, SDL_Color{0x55,0x00,0xff,0xff}, SDL_Color{0x55,0xaa,0x55,0xff}, SDL_Color{0x55,0xaa,0xff,0xff},
                             SDL_Color{0xff,0x00,0x55,0xff}, SDL_Color{0xff,0x00,0xff,0xff}, SDL_Color{0xff,0xaa,0x55,0xff}, SDL_Color{0xff,0xaa,0xff,0xff},
                             SDL_Color{0x55,0x55,0x00,0xff}, SDL_Color{0x55,0x55,0xaa,0xff}, SDL_Color{0x55,0xff,0x00,0xff}, SDL_Color{0x55,0xff,0xaa,0xff},
                             SDL_Color{0xff,0x55,0x00,0xff}, SDL_Color{0xff,0x55,0xaa,0xff}, SDL_Color{0xff,0xff,0x00,0xff}, SDL_Color{0xff,0xff,0xaa,0xff},
                             SDL_Color{0x55,0x55,0x55,0xff}, SDL_Color{0x55,0x55,0xff,0xff}, SDL_Color{0x55,0xff,0x55,0xff}, SDL_Color{0x55,0xff,0xff,0xff},
                             SDL_Color{0xff,0x55,0x55,0xff}, SDL_Color{0xff,0x55,0xff,0xff}, SDL_Color{0xff,0xff,0x55,0xff}, SDL_Color{0xff,0xff,0xff,0xff}
    };
    SDL_Palette  * sdlpal;
    SDL_Renderer * rend;
    SDL_Surface  * surf;
    SDL_Texture  * text;
    SDL_Window   * wind;

    void update_screen() {
        SDL_DestroyTexture(text);
        if(!rend || !surf) {
            cerr<<"Renderer or surface is non-existent"<<endl;
            return;
        }
        text = SDL_CreateTextureFromSurface(rend, surf);
        if(!text) {
            cerr<<"Texture not created appropriately"<<endl;
            cerr<<SDL_GetError()<<endl;
            return;
        }
        SDL_RenderClear(rend);
        SDL_RenderCopy(rend, text, NULL,  NULL);
        SDL_RenderPresent(rend);
        cout<<"surf->pitch: "<<surf->pitch<<endl;
    }

    void bits_to_surf() {
        int memaddr = disp_ptr;
        for(int y=0;y<200;y++) {
            int bit=0;
            for(int x=0;x<320;x++) {
                int val = 128>>(bit);
                int index = 1 * (bitplanes[memaddr][0] & val) +   //blue
                            2 * (bitplanes[memaddr][1] & val) +   //green
                            4 * (bitplanes[memaddr][2] & val) +   //red
                            8 * (bitplanes[memaddr][3] & val);    //value
                index /= val;
                pset(x,y,index);
                bit++;
                if(bit == 8) {
                    bit = 0;
                    memaddr = (memaddr+1) % 0x10000;
                }
            }
        }
    }
};

void display_file(ega& screen, char * filename, int delay) {
    ifstream in(filename);
    vector<uint8_t> data(0x2000);
    screen.set_planes(0,0);
    screen.set_ptr(0);
    in.read(reinterpret_cast<char *>(&data[0]), (320*200)/8);
    screen.plane_blit(data);
    
    screen.set_planes(1,1);
    in.read(reinterpret_cast<char *>(&data[0]), (320*200)/8);
    screen.plane_blit(data);

    screen.set_planes(2,2);
    in.read(reinterpret_cast<char *>(&data[0]), (320*200)/8);
    screen.plane_blit(data);

    screen.set_planes(3,3);
    in.read(reinterpret_cast<char *>(&data[0]), (320*200)/8);
    screen.plane_blit(data);
    in.close();

    screen.refresh();
    SDL_Delay(delay);
}

int main(int argc, char *argv[]) {
    if(argc == 1) {
        cerr<<"Provide some filenames to interpret as EGA screen dumps"<<endl;
        return 1;
    }

    ega screen;

    for(int i = 1; i < argc; i++) {
        display_file(screen, argv[i], 5000);
    }

    return 0;
}
