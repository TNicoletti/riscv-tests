#pragma GCC optimize ("no-tree-vectorize")
int* permute(int p, int* r){
    int values[24][4] = {
        {0,1,2,3}, {0,1,3,2}, {0,2,1,3}, {0,2,3,1}, {0,3,1,2}, {0,3,2,1},
        {1,0,2,3}, {1,0,3,2}, {1,2,0,3}, {1,2,3,0}, {1,3,0,2}, {1,3,2,0},
        {2,0,1,3}, {2,0,3,1}, {2,1,0,3}, {2,1,3,0}, {2,3,0,1}, {2,3,1,0},
        {3,0,1,2}, {3,0,2,1}, {3,1,0,2}, {3,1,2,0}, {3,2,0,1}, {3,2,1,0}
    };

    if (p >= 0 && p < 24) {
        for (int i = 0; i < 4; i++) {
            r[i] = values[p][i];
        }
    }
}

#pragma GCC optimize ("no-tree-vectorize")
void get_permutation(int n, int size, int* result) {
    int numbers[size];
    int factorial[size];
    
    factorial[0] = 1;
    for (int i = 0; i < size; i++) {
        numbers[i] = i;
        if (i > 0) factorial[i] = factorial[i - 1] * i;
    }

    for (int i = 0; i < size; i++) {
        int bucket_size = factorial[size - 1 - i];
        int index = n / bucket_size;
        
        result[i] = numbers[index];
        
        for (int j = index; j < size - 1; j++) {
            numbers[j] = numbers[j + 1];
        }
        
        n %= bucket_size;
    }
}