{
    static FILE* fp = NULL;
    static int   x = 0;

    if ( fp == NULL ) fp = fopen ( "cvd.txt", "a" );
    fprintf ( fp, "%7.3f  %6.2f %7.2f\n", (x>>1)*1152./44100, res1, res2 );

    x++;
}
