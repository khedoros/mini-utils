#include<iostream>
#include<fstream>
#include<cstdint>
#include<vector>
int interpret(uint16_t, std::vector<uint8_t>&, std::vector<uint8_t>&);
uint16_t mptr = 0;
int main(int argc, char *argv[]) {
    std::ifstream in;
    std::vector<uint8_t> dat(64*1024,0);
    std::vector<uint8_t> prg(64*1024-1,0);
    size_t filesize = 0;
    if(argc == 2) {
        in.open(argv[1]);
        if(!in.is_open()) {
            std::cerr<<"Couldn't open file "<<argv[1]<<", exiting."<<std::endl;
            return 1;
        }
    }
    else {
        std::cerr<<"Provide a single filename as an argument."<<std::endl;
        return 1;
    }
    in.seekg(0,std::ios::end);
    filesize = in.tellg();
    in.seekg(0,std::ios::beg);
    if(filesize > prg.size()) {
        std::cerr<<"I don't believe that you *actually* have a BF program over 64k in size. Exiting."<<std::endl;
        return 1;
    }
    prg.resize(filesize);
    in.read(reinterpret_cast<char *>(&(prg[0])), filesize);
    uint16_t iptr = 0;
    interpret(iptr, dat, prg);
    return 0;
}

int interpret(uint16_t iptr, std::vector<uint8_t>& dat, std::vector<uint8_t>& prg) {
    uint16_t orig_iptr = iptr;
    while(iptr+1 < prg.size()) {
        switch(prg[iptr]) {
            case '>': mptr++; break;
            case '<': mptr--; break;
            case '+': dat[mptr]++; break;
            case '-': dat[mptr]--; break;
            case '.': std::cout<<dat[mptr]; break;
            case ',': dat[mptr] = 10; 
                      while(dat[mptr] == 10) dat[mptr] = std::cin.get(); 
                      break;
            case '[': 
                      if(!dat[mptr]) { //Skip the loop block
                          uint8_t count = 1;
                          while(iptr+1 < prg.size() && count > 0) {
                              iptr++;
                              if(prg[iptr] == '[') count++;
                              else if(prg[iptr] == ']') count--;
                          }
                      }
                      else iptr = interpret(iptr+1, dat, prg);
                      break;
            case ']': 
                      if(!dat[mptr]) return iptr; //Next thing to happen will be for the iptr to be incremented.
                      else iptr = orig_iptr-1; //Because it's going to increment
                      break;
            default: break;
        }
        iptr++;
    }
    return 65535;
}
