#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main(){
    string label = "@Coverage.main.0";
    string func_label = "";

    for(int i=0; i < label.length(); i++){
        if (label[i] == '.'){
            label.erase(label.begin() + i, label.end());
            break;
        } 
    }

    //reverse(func_label.begin(), func_label.end());
    
    cout << "func_label: " << label <<endl;
    // string filename = "@Coverage.main.0";

    // cout << "filename[1] : " << filename[1] << endl;
    // use push_back("") for vec<str>

    // GoodHours *bruh = new GoodHours();

    // int result = bruh->howMany(b); 

    // cout << result << endl;

    return 0;
}