int main() {
    int a = 10;
    int *p = &a;
    {
        int *pp = &p;
        **pp = 20;  
    }
    return a; 
}