#include<SDL2/SDL.h>
#include<iostream>
#include<fstream>
#include<bitset>
#include<vector>
#include<cstdint>
#include<array>
#include<string>
#include<stdexcept>

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
        //cout<<"ptr set to "<<disp_ptr<<endl;
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
            //cout<<"Write "<<hex<<int(bytes[i])<<" to "<<disp_ptr+i<<", plane "<<write_plane<<endl;
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
        //cout<<"surf->pitch: "<<surf->pitch<<endl;
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
                //cout<<dec<<"output to ("<<x<<", "<<y<<") from memaddr "<<hex<<memaddr<<dec<<endl;
                bit++;
                if(bit == 8) {
                    bit = 0;
                    memaddr = (memaddr+1) % 0x10000;
                }
            }
        }
    }
};

int getsize(ifstream& in) {
    int bookmark = in.tellg();
    in.seekg(0,ios::end);
    int retval = in.tellg();
    in.seekg(bookmark);
    return retval;
}

void display_file(ega& screen, ifstream& in, int mode, int delay) {
    if(getsize(in) != (320 * 200) / 2) {
        cerr<<"Data probably doesn't represent a full screen; expect garbling"<<endl;
    }
    if(mode == 0) { //graphic-planar data
        vector<uint8_t> data((320*200)/8);
        screen.set_ptr(0);
        for(int plane = 0; plane < 4; plane++) {
            screen.set_planes(plane,plane);
            //Read a screen-plane of data
            in.read(reinterpret_cast<char *>(&data[0]), (320*200)/8);
            screen.plane_blit(data);
        }
    }
    else if(mode == 1) { //row-planar data
        vector<uint8_t> data(320/8);
        for(int line = 0; line < 200; line++) {
            //Set screen pointer to current line
            screen.set_ptr(line * (320 / 8));
            for(int plane = 0; plane < 4; plane++) {
                screen.set_planes(plane, plane);
                //Read a line-plane of data
                in.read(reinterpret_cast<char *>(&data[0]), 320/8);
                screen.plane_blit(data);
            }
        }
    }
    else if(mode == 2) { //byte-planar data
        vector<uint8_t> raw_data(320/2);
        for(int line = 0; line < 200; line++) {
            //read a line of data (all 4 planes interleaved)
            in.read(reinterpret_cast<char *>(&raw_data[0]), 320/2);
            vector<uint8_t> plane_data(320/8);
            //De-interleave the plane data, and blit 1 line-plane at a time
            for(int plane = 0; plane < 4; plane++) {
                    screen.set_planes(plane, plane);
                    screen.set_ptr(line * (320 / 8));
                    for(int byte = 0; byte < 320 / 8; byte++) {
                        plane_data[byte] = raw_data[byte * 4 + plane];
                    }
                    screen.plane_blit(plane_data);
                }
            }
    }
    else if(mode == 3) { //linear ega data
        cerr<<"Mode 3 is linear ega data. Rare. Todo."<<endl;
        return;
    }
    else {
        cerr<<"Modes are currently just 0->3"<<endl;
        return;
    }

    screen.set_ptr(0);
    screen.refresh();
    SDL_Delay(delay);
}

int open_next_valid(int argc, char **argv, int start, ifstream& in) {
    if(in.is_open()) {
        in.close();
    }
    for(int i = start; i < argc; i++) {
        in.open(argv[i]);
        if(in.is_open()) {
            return i;
        }
        else {
            cerr<<"Couldn't open file "<<argv[i]<<", moving on to the next one."<<endl;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        cerr<<"Provide a mode and some filenames to interpret as EGA screen dumps"<<endl;
        return 1;
    }

    int mode;

    try {
        mode = stoi(argv[1]);
    }
    catch(const invalid_argument& ia) {
        cerr<<argv[1]<<" is an invalid mode argument."<<endl;
        return 1;
    }
    catch(const out_of_range& oor) {
        cerr<<argv[1]<<" is out of range."<<endl;
        return 1;
    }

    ifstream in;
    int first_valid = open_next_valid(argc, argv, 2, in);

    if(first_valid < 0) {
        cerr<<"Couldn't open any provided filenames. Exiting."<<endl;
        return 1;
    }

    ega screen;

    for(int i = first_valid; i > 0 && i < argc; i++) {
        display_file(screen, in, mode, 5000);
        i = open_next_valid(argc, argv, i + 1, in) - 1;
    }

    return 0;
}
