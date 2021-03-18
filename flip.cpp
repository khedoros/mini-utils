#include<iostream>
#include<fstream>

int main(int argc, char *argv[]) {
    if(argc != 3) {
        return 1;
    }

    std::ifstream in(argv[1]);
    std::ofstream out(argv[2]);

    char buffer[4];

    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);

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

    return 0;
}
