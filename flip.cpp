#include<iostream>
#include<fstream>

int main(int argc, char *argv[]) {
    if(argc != 3) {
        std::cerr<<"Use: "<<argv[0]<<" inputFile outputFile\n";
        std::cerr<<"\n\tPurpose: Dumb little utility to byte-swap save files output by a Retrode so that they work in an emulator."
                   "\n\t         Just provide an input filename, and a filename to output the result to.\n\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);

    if(!in.is_open()) {
        std::cerr<<"Couldn't open source file "<<argv[1]<<"\n";
        return 2;
    }

    if(!out.is_open()) {
        std::cerr<<"Couldn't open destination file "<<argv[1]<<"\n";
        return 3;
    }

    char buffer[4];

    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);

    if(size % 4 != 0) {
        std::cerr<<"Filesize isn't divisible by 4. Output might not be valid.\n";
        return 4;
    }

    for(size_t i = 0; i < size; i+=4) {
        in.read(buffer, 4);
        char temp = buffer[0];
        buffer[0] = buffer[3];
        buffer[3] = temp;
        temp = buffer[1];
        buffer[1] = buffer[2];
        buffer[2] = temp;
        out.write(buffer, 4);
    }

    in.close();
    out.close();

    std::cout<<"Output "<<size<<" bytes to file "<<argv[2]<<".\n";

    return 0;
}
