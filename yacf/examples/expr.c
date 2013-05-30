
int main()
{
    int a, b, c, d;

    if ((a * b + c) < ( a * (b + c) )) {
        d = c - b * a;
    } else {
        d = (c - b) * a;
    }

}
