#include <bits/stdc++.h> 
int allocateBooks(vector<int> arr, int n, int m) {
    int students[m];
    sort(arr.begin(),arr.end());
    for(int i = 0; i < arr.size(); i++){
        students[i % m] += arr[i];
    }
    
    int max = INT_MIN;
    for(auto a : arr)
        if(a > max)
            max = a;

    return max;

}

int main(){
    vector<int> arr = {10,20,30,40};
    allocateBooks(arr,4,2);
}