int main() {
    int *ptr;
    {
        int x = 5;
        ptr = &x; 
    }
    {
        int y = 10; 
        if (*ptr == 10) { 
            return 1;
        }
    }
    return 0;
}