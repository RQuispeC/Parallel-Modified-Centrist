#include<bits/stdc++.h>
using namespace std;
typedef vector < double > vd;
typedef vector < string > vs;
double total[5][5], kernel[5][5];
vs tokenize(string line)
{
    istringstream iss(line);
    vector<string> tokens{istream_iterator<string>{iss}, istream_iterator<string>{}};
    return tokens;
}
void read_data()
{
    memset(total, 0, sizeof total);
    memset(kernel, 0, sizeof kernel);
    ifstream myfile ("results.txt");
    string line;
    for(int j = 0; j < 5; j++)
    {
        getline(myfile, line); // read method title 
        cout<< line << endl;
        for(int i = 0; i < 5; i++)
        {
            double tot = 0;
            double ker = 0;
            getline(myfile, line); // read image title
            for(int k = 0; k < 5; k++)
            {
                getline(myfile, line); // read shell line    
                if(j == 0) //serial
                {   
                    getline(myfile, line); //read time
                    vs tokens = tokenize(line);
                    tot += stod(tokens[tokens.size()-1].substr(0, 8));
                }
                else if(j == 1) //cuda
                {
                    getline(myfile, line);  //read cudaMalloc
                    getline(myfile, line);  //read copyToDevice
                    getline(myfile, line);  //read Kernel
                    vs tokens = tokenize(line);
                    ker += stod(tokens[tokens.size()-1].substr(0, 8));
                    getline(myfile, line);  //read copyToDevice
                    getline(myfile, line);  //read Offload
                    getline(myfile, line);  //read cudaTotal
                    tokens = tokenize(line);
                    tot += stod(tokens[tokens.size()-1].substr(0, 8));
                }
                else //clang
                {
                    bool end = 0;
                    while(!end)
                    {
                        getline(myfile, line);
                        int pos = line.find("Total");
                        if(pos >= 0) 
                        {
                            end = 1;
                            vs tokens = tokenize(line);
                            tot += stod(tokens[tokens.size()-1].substr(0, 8));
                        }
                        pos = line.find("kernel");
                        if(pos >= 0) 
                        {
                            vs tokens = tokenize(line);
                            ker += stod(tokens[tokens.size()-2]);
                        }
                    }
                }
            }   
            total[i][j] = tot/5.0;
            kernel[i][j] = ker/5.0;
        }
        getline(myfile, line); //read blanck line
    }
    myfile.close();
}
void outputResults()
{
    string method[5] = {"serial", "cuda", "clang none", "clang tile", "clang vect"};
    string image[5] = {"image 00", "image 01", "image 02", "image 03", "image 04"};
    for(int j = 0; j < 5; j++)
    {
        cout << method[j] << endl;
        for(int i = 0; i < 5; i++)
        {
            cout << image[i];
            printf("  --> serial %.6f, parallel %.6f, speedup %.6f\n", total[i][0], total[i][j], total[i][0]/total[i][j]);
        }
    }   
}
int main(){
    read_data();
    outputResults();
    return 0;
}