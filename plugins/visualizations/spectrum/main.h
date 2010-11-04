
void fft_float(
    unsigned  NumSamples,       /* must be a power of 2          */
    int       InverseTransform,	/* 0=forward FFT, 1=inverse FFT  */
    float    *RealIn,			/* array of input's real samples */
    float    *ImagIn,			/* array of input's imag samples */
    float    *RealOut,			/* array of output's reals       */
    float    *ImagOut);		/* array of output's imaginaries */