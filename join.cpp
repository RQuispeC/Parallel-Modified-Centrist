#include<bits/stdc++.h>
using namespace std;
typedef vector < double > vd;
typedef vector < string > vs;
vs tokenize(string line)
{
    istringstream iss(line);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    return tokens;
}
double read_serial()
{
    ifstream myfile ("serial_tmp.dat");
    string line;
    while(getline(myfile, line))
    {
        if(line.size() != 0) break;
    }
    vs tokens = tokenize(line);
    double serial = stod(tokens[tokens.size()-1].substr(0, 8));
    myfile.close();
    return serial;
}
vd read_cuda()
{
    ifstream myfile ("cuda_tmp.dat");
    string line;
    while(getline(myfile, line)) //read cudaMalloc
    {
        if(line.size() != 0) break;
    }
    getline(myfile, line);  //read copyToDevice
    getline(myfile, line);  //read Kernel
    vs tokens = tokenize(line);
    double kernel = stod(tokens[tokens.size()-1].substr(0, 8));
    getline(myfile, line);  //read copyToDevice
    getline(myfile, line);  //read Offload
    getline(myfile, line);  //read cudaTotal
    tokens = tokenize(line);
    double total = stod(tokens[tokens.size()-1].substr(0, 8));
    myfile.close();
    return vd{kernel, total};
}
vd read_clang()
{
    vs clang_names = {"clang_none_tmp.dat", "clang_tile_tmp.dat", "clang_vect_tmp.dat"};    
    vd ans = vd();
    for(int i = 0; i < 3; i++)
    {
        ifstream myfile (clang_names[i]);
        string line;
        bool end = 0;
        double total, kernel;
        while(!end)
        {
          getline(myfile, line);
          int pos = line.find("Total");
          if(pos >= 0) 
          {
            end = 1;
            vs tokens = tokenize(line);
            total = stod(tokens[tokens.size()-1].substr(0, 8));
          }
          pos = line.find("kernel");
          if(pos >= 0) 
          {
            vs tokens = tokenize(line);
            kernel = stod(tokens[tokens.size()-2]);
          }
        }
        myfile.close();
        ans.push_back(kernel);
        ans.push_back(total);
    }
    return ans;
}
void outputResults(char* filename,double serial, vd cuda, vd clang)
{
    printf("--------- %s ---------\n", filename);
    printf("Serial: %0.6lfs\n", serial);
    puts("");
    printf("Cuda Offload: %0.6lfs\n", cuda[1] - cuda[0]);
    printf("Cuda Kernel: %0.6lfs\n", cuda[0]);
    printf("Cuda Total: %0.6lfs\n", cuda[1]);
    printf("Cuda SpeedUp: %0.6lf\n", serial/cuda[1]);
    puts("");
    vs clang_names = {"none", "tile", "vectorize"};
    for(int i = 0; i < 3; i++)
    {
        cout<< "Clang Optimization: "<< clang_names[i] << endl;
        printf("Clang Kernel: %0.6lfns\n", clang[i*2]);
        printf("Clang Total: %0.6lfs\n", clang[i*2 + 1]);
        printf("Clang SpeedUp: %0.6lf\n", serial/clang[i*2 + 1]);
        puts("");
    }
}
int main(int argc, char *argv[]){
    if( argc != 2 ) {
        printf("Too many or no one arguments supplied.\n");
    }
    char *filename = argv[1]; //Recebendo o arquivo!;

    double serial = read_serial();
    vd cuda =  read_cuda();
    vd clang = read_clang();
    outputResults(filename, serial, cuda, clang);
}
