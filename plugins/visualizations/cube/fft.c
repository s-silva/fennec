/*-----------------------------------------------------------------------------

 FFT.c  -  Don Cross <dcross@intersrv.com>

 http://www.intersrv.com/~dcross/fft.html

 Contains definitions for doing Fourier transforms
 and inverse Fourier transforms.

 This module performs operations on arrays of 'float'.

 Revision history:
 
 1998 September 19 [Don Cross]
  Updated coding standards.
  Improved efficiency of trig calculations.

 Readme (by Don Cross):

 The file FFT.ZIP contains C source code for performing Discrete Fast Fourier
 Transforms (DFFTs) and inverse DFFTs.  This source code is public domain.
 Use at your own risk.  For more information, point your web browser at:

 http://www.intersrv.com/~dcross/fft.html

 Also, feel free to send questions/comments about this source code to me
  by e-mail at the address above.

     -----------------------------------------------------------------------
 
                          *** IMPORTANT NOTE ***

 There are two different ways to define the FFT (and inverse FFT) that
 result in having the same outputs, only the imaginary parts are
 negated.  In other words, the two different algorithms produce results
 that are complex conjugates of each other.
 
 The popular mathematical software tool MATLAB and my FFT source routines
 use opposite definitions.  However, it is easy to make my FFT source
 code compatible with MATLAB.  To do this, use your favorite text editor
 to change the following files:

     fourierd.c
     fourierf.c
 
 Look for the line of text that says:
 
     double angle_numerator = 2.0 * DDC_PI;
 
 And change it to:
 
     double angle_numerator = -2.0 * DDC_PI;
 
 That is, just make the angle numerator negative instead of positive.
 
 Be sure to do this in both fourierd.c AND fourierf.c so you have
 consistent routines for both single- and double-precision math.

 I am considering changing my code to be compatible with MATLAB, but
 first I would like to find out if there is some definitely "correct"
 or "preferred" definition within the mathematical community.  If you
 have any comments on this issue, please let me know at the following 
 email address, because I'm stumped and need help!
 
    dcross@intersrv.com
 
 Thanks!
 
    -----------------------------------------------------------------------
 
                         *** SMALL REQUESTS ****

 If you want to give away copies of this source code, that's fine, so long
 as you do the following:
 
 - Do not charge any money for this source code, except for possibly a
   reasonable fee to cover postage, disk duplication, etc.  I wrote this
   code and I want it to be free to EVERYONE!
 
 - Do not remove my name, e-mail address, or URL from any of the files in
   this collection.
 
 - Please keep this readme.txt file with the source and headers so that others
   can get in touch with me to ask questions and/or find my web page to read
   the online tutorial.
 
 - If you make any modifications to the source code, please add comments to
   it which include your name, e-mail address, web page URL (if any), and
   explain what you did to the code.
 
 - If you use this source code in an interesting program, please let me know.
   I promise will never try to get money from you, even if you use this code
   in a commercial program.  I just want to know what kind of clever and
   creative things people do with Fourier Transforms.
 
    -----------------------------------------------------------------------

 (formatted for fennec)
-----------------------------------------------------------------------------*/

#include <math.h>
#include "main.h" /* just holds the declaration of fft */

#define  DDC_PI  (3.14159265358979323846)

int fft_local_IsPowerOfTwo ( unsigned x )
{
    if ( x < 2 )
        return 0;

    if ( x & (x-1) ) /* Thanks to 'byang' for this cute trick! */
        return 0;

    return 1;
}

unsigned fft_local_NumberOfBitsNeeded ( unsigned PowerOfTwo )
{
    unsigned i;

    if ( PowerOfTwo < 2 )
    {
		return 0;
    }

    for ( i=0; ; i++ )
    {
        if ( PowerOfTwo & (1 << i) )
            return i;
    }
}

unsigned fft_local_ReverseBits ( unsigned index, unsigned NumBits )
{
    unsigned i, rev;

    for ( i=rev=0; i < NumBits; i++ )
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }

    return rev;
}

/*
**   The following function returns an "abstract frequency" of a
**   given index into a buffer with a given number of frequency samples.
**   Multiply return value by sampling rate to get frequency expressed in Hz.
*/

double fft_local_Index_to_frequency ( unsigned NumSamples, unsigned Index )
{
    if ( Index >= NumSamples )
        return 0.0;
    else if ( Index <= NumSamples/2 )
        return (double)Index / (double)NumSamples;

    return -(double)(NumSamples-Index) / (double)NumSamples;
}

/*
**   fft() computes the Fourier transform or inverse transform
**   of the complex inputs to produce the complex outputs.
**   The number of samples must be a power of two to do the
**   recursive decomposition of the FFT algorithm.
**   See Chapter 12 of "Numerical Recipes in FORTRAN" by
**   Press, Teukolsky, Vetterling, and Flannery,
**   Cambridge University Press.
**
**   Notes:  If you pass ImaginaryIn = NULL, this function will "pretend"
**           that it is an array of all zeroes.  This is convenient for
**           transforming digital samples of real number data without
**           wasting memory.
*/

void fft_float (
    unsigned  NumSamples,       /* must be a power of 2          */
    int       InverseTransform,	/* 0=forward FFT, 1=inverse FFT  */
    float    *RealIn,			/* array of input's real samples */
    float    *ImagIn,			/* array of input's imag samples */
    float    *RealOut,			/* array of output's reals       */
    float    *ImagOut )			/* array of output's imaginaries */
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, n;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = 2.0 * DDC_PI;
    double tr, ti;     /* temp real, temp imaginary */

    if ( !fft_local_IsPowerOfTwo(NumSamples) )
    {
       return;
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    /*CHECKPOINTER ( RealIn );
      CHECKPOINTER ( RealOut );
      CHECKPOINTER ( ImagOut ); */

    NumBits = fft_local_NumberOfBitsNeeded ( NumSamples );

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = fft_local_ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == 0) ? 0.0f : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        double ar[3], ai[3];

        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar[0] = w*ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w*ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + BlockEnd;
                tr = ar[0]*RealOut[k] - ai[0]*ImagOut[k];
                ti = ar[0]*ImagOut[k] + ai[0]*RealOut[k];

                RealOut[k] = RealOut[j] - (float)tr;
                ImagOut[k] = ImagOut[j] - (float)ti;

                RealOut[j] += (float)tr;
                ImagOut[j] += (float)ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        float denom = (float)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}

/* end of the file */