#include <stdio.h>
#include "math.h"

typedef int (*Operation)(int, int);

int main() {
    Operation operation;
    operation = add;
    printf("%d\n", operation(10, 3));
    operation = subtract;
    printf("%d\n", operation(10, 3));
    return 0;
}
