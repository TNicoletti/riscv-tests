void print_vector(int* vet, int LENGTH){
    for(int i = 0; i < LENGTH; i++){
        printf("%d ", vet[i]);
        if((i % 15) == 14)
            printf("\n");
    }
    printf("\n");
}