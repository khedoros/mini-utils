#include<iostream>
#include<fstream>
#include<cstdint>
#include<cassert>
#include<vector>

using namespace std;
int interpret(uint16_t, vector<uint8_t>&, vector<uint8_t>&);

uint16_t mptr = 0;

int main(int argc, char *argv[]) {
    assert(sizeof(uint16_t) == 2);
    ifstream in;
    vector<uint8_t> dat(64*1024,0);
    vector<uint8_t> prg(64*1024-1,0);
    size_t filesize = 0;
    if(argc == 2) {
        in.open(argv[1]);
        if(!in.is_open()) {
            cerr<<"Couldn't open file "<<argv[1]<<", exiting."<<endl;
            return 1;
        }
    }
    else {
        cerr<<"Provide a single filename as an argument."<<endl;
        return 1;
    }
    in.seekg(0,ios::end);
    filesize = in.tellg();
    in.seekg(0,ios::beg);
    if(filesize > prg.size()) {
        cerr<<"I don't believe that you *actually* have a BF program over 64k in size. Exiting."<<endl;
        return 1;
    }
    prg.resize(filesize);
    in.read(reinterpret_cast<char *>(&(prg[0])), filesize);

    uint16_t iptr = 0;
    interpret(iptr, dat, prg);
    return 0;
}

int interpret(uint16_t iptr, vector<uint8_t>& dat, vector<uint8_t>& prg) {
    uint16_t orig_iptr = iptr;
    while(iptr+1 < prg.size()) {
        //if(prg[iptr] == '>' ||prg[iptr] == '<' ||prg[iptr] == '+' ||prg[iptr] == '-' ||prg[iptr] == '.' ||prg[iptr] == ',' ||prg[iptr] == '[' ||prg[iptr] == ']')
        //        cout<<"iptr: "<<iptr<<" (inst: "<<prg[iptr]<<"), mptr: "<<mptr<<" (data: "<<uint32_t(dat[mptr])<<")"<<endl;
        switch(prg[iptr]) {
            case '>': mptr++; break;
            case '<': mptr--; break;
            case '+': dat[mptr]++; break;
            case '-': dat[mptr]--; break;
            case '.': cout<<dat[mptr]; break;
            case ',': dat[mptr] = 10; 
                      while(dat[mptr] == 10) dat[mptr] = cin.get(); 
                      //cout<<"Got char "<<uint32_t(dat[mptr])<<endl;
                      break;
            case '[': 
                      if(!dat[mptr]) { //Skip the loop block
                          //cout<<"Skip loop"<<endl;
                          uint8_t count = 1;
                          while(iptr+1 < prg.size() && count > 0) {
                              iptr++;
                              if(prg[iptr] == '[') count++;
                              else if(prg[iptr] == ']') count--;
                          }
                      }
                      else { //Interpret the loop block
                          //cout<<"Enter loop"<<endl;
                          iptr = interpret(iptr+1, dat, prg);
                      }
                      break;
            case ']': 
                      if(!dat[mptr]) { //Exit end of loop
                          //cout<<"Exit loop"<<endl;
                          return iptr; //Next thing to happen will be for the iptr to be incremented.
                      }
                      else { //Return to beginning of loop
                          //cout<<"Iterate loop"<<endl;
                          iptr = orig_iptr-1; //Because it's going to increment
                      }
                      break;
            default: break; //cerr<<"Invalid character found)"<<endl; return (1024*64)-1;
        }
        iptr++;
    }
    return 65535;
}
