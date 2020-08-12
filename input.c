{
    foo = 40;
    bar = foo + 2;
    i = 1;

    if (bar > 20) {
        bar = bar + 1;
        return bar;
    }

    while (i < 10) {
        i = i * 3;
        i = i - 1;
    }

    while (0) {
    }

    return i;
}
