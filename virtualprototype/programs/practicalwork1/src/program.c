#include <stdio.h>

#define SIZE 3

void reverse(char* buf, unsigned int sz) {
    char tmp;
    for (unsigned int i = 0; i < sz / 2; i++) {
        tmp = buf[i];
        buf[i] = buf[sz - 1 - i];
        buf[sz - 1 - i] = tmp;
    }
}

unsigned int utoa(unsigned int number, char* buf, unsigned int bufsz, unsigned int base, const char* digits) {
    if (base <= 1 || bufsz <= 2) {
        return 0;
    }

    unsigned int ptr = 0;
    while (number >= base) {
        if (ptr >= bufsz - 2) {
            return 0;
        }

        unsigned int dig = number % base;
        number = number / base;

        buf[ptr] = digits[dig];
        ptr++;
    }

    buf[ptr] = digits[number];
    buf[ptr + 1] = '\0';

    unsigned int len = ptr + 1; 
    reverse(buf, len);

    return len;
}

int main(){
    const char* vigesimal_digits = "0123456789ABCDEFGHIJ";
    char buf[SIZE];

    for(int i=1; i<=100; i++){
        if(!utoa(i, buf, SIZE, 20, vigesimal_digits)){
            return 1;
        }
        
        printf("%d: %s\n", i, buf);
    }

    return 0;
}
