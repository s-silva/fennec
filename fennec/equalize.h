/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
 Copyright (C) 2007 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/



#ifdef never_define_this

EqualizerBand EqBands[31];
EqualizerBand Preamp;

float EqDecibelValues[31][2];
float LeftPreampDecibel = 0;
float RightPreampDecibel = 0;

eq_bands_data eb;

HistoryXY DataHistory[31][2];

int datab[0x32000];

int i,j,k;

float __inline db_to_band(float value)
{
	float rv = (float) (2.5220207857061455181125E-01 * exp(8.0178361802353992349168E-02 * value) - 2.5220207852836562523180E-01);
	return rv;
}

float __inline db_to_band_amp(float value)
{
	float rv =  (float)(9.9999946497217584440165E-01 * exp(6.9314738656671842642609E-02 * value) + 3.7119444716771825623636E-07);
	return rv;
}

/* -12db to 12db */
void SetDecibelBandValue(int bnid, int chid, float value)
{
	if(chid == 0)
	{
		EqDecibelValues[bnid][0] = value;
		EqBands[bnid].cleft  = (float) (2.5220207857061455181125E-01 * exp(8.0178361802353992349168E-02 * value) - 2.5220207852836562523180E-01);
	}else{
		EqDecibelValues[bnid][1] = value;
		EqBands[bnid].cright = (float) (2.5220207857061455181125E-01 * exp(8.0178361802353992349168E-02 * value) - 2.5220207852836562523180E-01);
	}
}

/* -12db to 12db */
void SetDecibelPreampValue(int chid, float value)
{
	if(chid == 0)
	{
		LeftPreampDecibel = value;
		Preamp.cleft  = (float) (9.9999946497217584440165E-01 * exp(6.9314738656671842642609E-02 * value) + 3.7119444716771825623636E-07);
	}else{
		RightPreampDecibel = value;
		Preamp.cright = (float) (9.9999946497217584440165E-01 * exp(6.9314738656671842642609E-02 * value) + 3.7119444716771825623636E-07);
	}
}

float GetDecibelPreampValue(int chid)
{
	if(chid == 0)
	{
		return LeftPreampDecibel;
	}else{
		return RightPreampDecibel;
	}
}

float GetDecibelBandValue(int bnid, int chid)
{
	//if(chid != 0 || chid != 1)return 0.0f;
	//if(bnid < 0 || bnid >= 10)return 0.0f;
	return EqDecibelValues[bnid][chid];
}

int wrap16Bit(int data)
{
    if(data > 32767)       data =   32767;
    else if(data < -32768) data =  -32768;
    if(data < 0)           data +=  65536;
    return data;
}

void InitializeEqualizer(void)
{
	int db,dc;

	for(db=0; db<31; db++)
	{
		for(dc=0; dc<2; dc++)
		{
			DataHistory[db][dc].x[0] = 0;
			DataHistory[db][dc].x[1] = 0;
			DataHistory[db][dc].x[2] = 0;
			DataHistory[db][dc].x[3] = 0;

			DataHistory[db][dc].y[0] = 0;
			DataHistory[db][dc].y[1] = 0;
			DataHistory[db][dc].y[2] = 0;
			DataHistory[db][dc].y[3] = 0;
		}
	}

	i = 0; j = 2; k = 1;

	SetEqualizer(0, settings.player.equalizer_last_bands, 10);
}

int SetEqualizer(int rvs, float* bsets, int bcount)
{
	int db,dc;

	if(rvs)
	{
		for(db=0; db<31; db++)
		{
			for(dc=0; dc<2; dc++)
			{
				DataHistory[db][dc].x[0] = 0;
				DataHistory[db][dc].x[1] = 0;
				DataHistory[db][dc].x[2] = 0;
				DataHistory[db][dc].x[3] = 0;

				DataHistory[db][dc].y[0] = 0;
				DataHistory[db][dc].y[1] = 0;
				DataHistory[db][dc].y[2] = 0;
				DataHistory[db][dc].y[3] = 0;
			}
		}

		i = 0;
		j = 2;
		k = 1;
	}

	if(bcount == 10)
	{
		SetDecibelPreampValue(0,bsets[0]);
		SetDecibelPreampValue(1,bsets[1]);

		SetDecibelBandValue(0,0,bsets[2]);
		SetDecibelBandValue(0,1,bsets[12]);

		SetDecibelBandValue(1,0,bsets[3]);
		SetDecibelBandValue(1,1,bsets[13]);

		SetDecibelBandValue(2,0,bsets[4]);
		SetDecibelBandValue(2,1,bsets[14]);

		SetDecibelBandValue(3,0,bsets[5]);
		SetDecibelBandValue(3,1,bsets[15]);

		SetDecibelBandValue(4,0,bsets[6]);
		SetDecibelBandValue(4,1,bsets[16]);

		SetDecibelBandValue(5,0,bsets[7]);
		SetDecibelBandValue(5,1,bsets[17]);

		SetDecibelBandValue(6,0,bsets[8]);
		SetDecibelBandValue(6,1,bsets[18]);

		SetDecibelBandValue(7,0,bsets[9]);
		SetDecibelBandValue(7,1,bsets[19]);

		SetDecibelBandValue(8,0,bsets[10]);
		SetDecibelBandValue(8,1,bsets[20]);

		SetDecibelBandValue(9,0,bsets[11]);
		SetDecibelBandValue(9,1,bsets[21]);
		return 1;
	}
	return 0;
}

/* dlen = byte length of data */
void EqualizeBuffer(void* dbuf, int chan, int freq, int bps, unsigned long dlen)
{
	fennec_sample             *dpointer;
	register unsigned long     di;
	register int               ci;
	register int               band;
	register unsigned long     rlen = dlen / (bps / 8);
	register double            pcm;
	fennec_sample              sout;
	IIRCoefficients           *ccoeffs;

	if(!settings.player.equalizer_enable)return;

	switch(freq)
	{
	case 11000:
	case 11025:
		ccoeffs = (IIRCoefficients*)&irrcef_10band11k;
		break;
	case 22000:
	case 22050:
		ccoeffs = (IIRCoefficients*)&irrcef_10band22k;
		break;
	case 44000:
	case 44100:
		ccoeffs = (IIRCoefficients*)&irrcef_10band44k;
		break;
	case 48000:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	default:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	}

	dpointer = (fennec_sample*)dbuf;

	if(bps == fennec_sample_bits) /* floating point, -1 to +1 */
	{
		for(di=0; di<rlen; di+=chan)
		{
			for(ci=0; ci<chan ; ci++)
			{
				pcm = (double)dpointer[di + ci] * 32768.0;

				/* optimized to get left/right without using an 'if' */
				
				pcm *= *(((double*)&Preamp) + ci);

                sout = 0.0;

                for(band=0; band<10; band++)
				{
                    DataHistory[band][ci].x[i] = pcm;
                    DataHistory[band][ci].y[i] = (ccoeffs[band].ialpha * (pcm - DataHistory[band][ci].x[k]) + ccoeffs[band].igamma * DataHistory[band][ci].y[j] - ccoeffs[band].ibeta * DataHistory[band][ci].y[k]);                            

					/* optimized to get left/right without using an 'if' */

					sout += (int)(DataHistory[band][ci].y[i] * (*(((double*)&EqBands[band]) + ci)) );
				}

				sout += pcm * 0.25;
				sout *= 4.0;

				sout /= 32767.0;

				if      (sout >  1.0) sout =  1.0;
				else if (sout < -1.0) sout = -1.0;

				dpointer[di + ci] = sout;
								

				//rintd = (int)pcm;

				//out += (rintd >> 2); /* out += v x 0.25 */
                //out <<= 2; /* out *= 4 */

				/* clip */
				
				//if(out > 32767)       out =   32767;
				//else if(out < -32768) out =  -32768;
				//if(out < 0)           out +=  65536;

				//dtwordpointer[di + ci] = (short)out;
			
			} /* channel */

			i++;
            j++;
            k++;

            /* wrap around the indexes */
            if(i >= 3)      i = 0;
            else if(j >= 3) j = 0;
            else            k = 0;
		}

	} /* </16 bit> */
}

void EqualizeBufferVariable(void* dbuf, int chan, int freq, int bps, unsigned long dlen, float *veqbands, int *vi, int *vj, int *vk, eq_bands_data *eqhistory)
{
	short* dtwordpointer;
	unsigned long  di;
	int    ci;//,bi;
	int    band;
	unsigned long  rlen = dlen / (bps / 8);
	double pcm,out;
	int    rintd;

	IIRCoefficients* ccoeffs;

	switch(freq)
	{
	case 11000:
	case 11025:
		ccoeffs = (IIRCoefficients*)&irrcef_10band11k;
		break;
	case 22000:
	case 22050:
		ccoeffs = (IIRCoefficients*)&irrcef_10band22k;
		break;
	case 44000:
	case 44100:
		ccoeffs = (IIRCoefficients*)&irrcef_10band44k;
		break;
	case 48000:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	default:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	}

	dtwordpointer = (short*)dbuf;

	if(bps == 16)
	{

		for(di=0; di<rlen;di += chan)
		{
			for(ci=0; ci<chan ; ci++)
			{
				rintd = (int)dtwordpointer[di + ci];

				if(!ci)
				{
					pcm = rintd * db_to_band_amp(veqbands[0]);
				}else{
					pcm = rintd * db_to_band_amp(veqbands[1]);
				}

                out = 0;

                /* For each band */

                for (band = 0; band < 10; band++)
				{
                    /* Store Xi(n) */

                    eqhistory->bdata[band][ci].x[(*vi)] = pcm;

                    /* Calculate and store Yi(n) */

                    eqhistory->bdata[band][ci].y[(*vi)] =
                            (
                                    /* 		= alpha * [x(n)-x(n-2)] */
                                    ccoeffs[band].ialpha * (pcm - eqhistory->bdata[band][ci].x[(*vk)])
                                            /* 		+ gamma * y(n-1) */
                                            + ccoeffs[band].igamma * eqhistory->bdata[band][ci].y[(*vj)]
                                            /* 		- beta * y(n-2) */
                                            - ccoeffs[band].ibeta * eqhistory->bdata[band][ci].y[(*vk)]
                            );

					if(!ci)
					{
						out += (eqhistory->bdata[band][ci].y[(*vi)] * db_to_band(veqbands[band + 1]));
					}else{
						out += (eqhistory->bdata[band][ci].y[(*vi)] * db_to_band(veqbands[band + 11]));
					}
				} /* band */

				out += (pcm * 0.25);

                /* Normalize the output */
                out *= 4;

                /* Round and convert to integer */
                //datab[di + ci] = (int) out;
				rintd = (int) out;

				dtwordpointer[di + ci] = (short)wrap16Bit(rintd);
			
			} /* channel */

			(*vi)++;
            (*vj)++;
            (*vk)++;

            /* Wrap around the indexes */
            if ((*vi) == 3)
                (*vi) = 0;
            else if ((*vj) == 3)
                (*vj) = 0;
            else
                (*vk) = 0;

		} /* sample */
	} /* if 16 bit switch */
}

int EqualizeBufferEx(void* outbuf, void* inbuf, int chan, int freq, int bps, unsigned long dlen)
{
	const short* dtwordpointer_in;
	short* dtwordpointer_out;
	unsigned long di;
	int   ci;//,bi;
	int band;
	unsigned long rlen = dlen / (bps / 8);
	double pcm,out;
	int   rintd;

	IIRCoefficients* ccoeffs;

	switch(freq)
	{
	case 11000:
	case 11025:
		ccoeffs = (IIRCoefficients*)&irrcef_10band11k;
		break;
	case 22000:
	case 22050:
		ccoeffs = (IIRCoefficients*)&irrcef_10band22k;
		break;
	case 44000:
	case 44100:
		ccoeffs = (IIRCoefficients*)&irrcef_10band44k;
		break;
	case 48000:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	default:
		ccoeffs = (IIRCoefficients*)&irrcef_10band48k;
		break;
	}

	dtwordpointer_in  = (const short*)inbuf;
	dtwordpointer_out = (short*)outbuf;

	if(bps == 16)
	{

		for(di=0; di<rlen;di += chan)
		{
			for(ci=0; ci<chan ; ci++)
			{
				rintd = (int)dtwordpointer_in[di + ci];
				//pcm = datab[di + ci] * Preamp.cleft;

				if(!ci)
				{
					pcm = rintd * Preamp.cleft;
				}else{
					pcm = rintd * Preamp.cright;
				}

                out = 0;

                /* For each band */

                for (band = 0; band < 10; band++)
				{
                    /* Store Xi(n) */

                    DataHistory[band][ci].x[i] = pcm;

                    /* Calculate and store Yi(n) */

                    DataHistory[band][ci].y[i] =
                            (
                                    /* 		= alpha * [x(n)-x(n-2)] */
                                    ccoeffs[band].ialpha * (pcm - DataHistory[band][ci].x[k])
                                            /* 		+ gamma * y(n-1) */
                                            + ccoeffs[band].igamma * DataHistory[band][ci].y[j]
                                            /* 		- beta * y(n-2) */
                                            - ccoeffs[band].ibeta * DataHistory[band][ci].y[k]
                            );

					if(!ci)
					{
						out += (DataHistory[band][ci].y[i] * EqBands[band].cleft);
					}else{
						out += (DataHistory[band][ci].y[i] * EqBands[band].cright);
					}
				} /* band */

				out += (pcm * 0.25);

                /* Normalize the output */
                out *= 4;

                /* Round and convert to integer */
                //datab[di + ci] = (int) out;
				rintd = (int) out;

				dtwordpointer_out[di + ci] = (short)wrap16Bit(rintd);

			} /* channel */

			i++;
            j++;
            k++;

            /* Wrap around the indexes */
            if (i == 3)
                i = 0;
            else if (j == 3)
                j = 0;
            else
                k = 0;

		} /* sample */
	} /* if 16 bit switch */

	return 1;
}
#endif 

typedef struct _IIRCoefficients
{
	double ibeta;
	double ialpha;
	double igamma;
}IIRCoefficients;

typedef struct _EqualizerBand
{
	double cleft;
	double cright;
}EqualizerBand;

typedef struct _HistoryXY
{
	double x[4];
	double y[4];
}HistoryXY;

/* beta, alpha, gamma */
IIRCoefficients irrcef_10band11k[] = {
        /* 31 Hz*/  {9.8758524689e-01, 6.2073765555e-03, 1.9872750693e+00},
        /* 62 Hz*/  {9.7532461998e-01, 1.2337690008e-02, 1.9740916593e+00},
        /* 125 Hz*/ {9.5087485437e-01, 2.4562572817e-02, 1.9459267562e+00},
        /* 250 Hz*/ {9.0416308662e-01, 4.7918456688e-02, 1.8848691023e+00},
        /* 500 Hz*/ {8.1751373987e-01, 9.1243130064e-02, 1.7442229115e+00},
        /* 1k Hz*/  {6.6840529852e-01, 1.6579735074e-01, 1.4047189863e+00},
        /* 2k Hz*/  {4.4858358977e-01, 2.7570820511e-01, 6.0517475334e-01},
        /* 3k Hz*/  {3.1012671838e-01, 3.4493664081e-01, -1.8141012760e-01},
        /* 4k Hz*/  {2.4198119087e-01, 3.7900940457e-01, -8.0845085113e-01},
        /* 5.5k Hz*/{3.3453245058e-01, 3.3273377471e-01, -1.3344985880e+00}
};

IIRCoefficients irrcef_10band22k[] = {
        /* 31 Hz*/  {9.9377323686e-01, 3.1133815717e-03, 1.9936954495e+00},
        /* 62 Hz*/  {9.8758524689e-01, 6.2073765555e-03, 1.9872750693e+00},
        /* 125 Hz*/ {9.7512812040e-01, 1.2435939802e-02, 1.9738753198e+00},
        /* 250 Hz*/ {9.5087485437e-01, 2.4562572817e-02, 1.9459267562e+00},
        /* 500 Hz*/ {9.0416308662e-01, 4.7918456688e-02, 1.8848691023e+00},
        /* 1k Hz*/  {8.1751373987e-01, 9.1243130064e-02, 1.7442229115e+00},
        /* 2k Hz*/  {6.6840529852e-01, 1.6579735074e-01, 1.4047189863e+00},
        /* 4k Hz*/  {4.4858358977e-01, 2.7570820511e-01, 6.0517475334e-01},
        /* 8k Hz*/  {2.4198119087e-01, 3.7900940457e-01, -8.0845085113e-01},
        /* 11k Hz*/ {3.3453245058e-01, 3.3273377471e-01, -1.3344985880e+00}
};
IIRCoefficients irrcef_10band44k[] = {
        /* 31 Hz*/  {9.9688176273e-01, 1.5591186337e-03, 1.9968622855e+00},
        /* 62 Hz*/  {9.9377323686e-01, 3.1133815717e-03, 1.9936954495e+00},
        /* 125 Hz*/ {9.8748575691e-01, 6.2571215431e-03, 1.9871705722e+00},
        /* 250 Hz*/ {9.7512812040e-01, 1.2435939802e-02, 1.9738753198e+00},
        /* 500 Hz*/ {9.5087485437e-01, 2.4562572817e-02, 1.9459267562e+00},
        /* 1k Hz*/  {9.0416308662e-01, 4.7918456688e-02, 1.8848691023e+00},
        /* 2k Hz*/  {8.1751373987e-01, 9.1243130064e-02, 1.7442229115e+00},
        /* 4k Hz*/  {6.6840529852e-01, 1.6579735074e-01, 1.4047189863e+00},
        /* 8k Hz*/  {4.4858358977e-01, 2.7570820511e-01, 6.0517475334e-01},
        /* 16k Hz*/ {2.4198119087e-01, 3.7900940457e-01, -8.0845085113e-01}
};
IIRCoefficients irrcef_10band48k[] = {
        /* 31 Hz*/  {9.9713475915e-01, 1.4326204244e-03, 1.9971183163e+00},
        /* 62 Hz*/  {9.9427771143e-01, 2.8611442874e-03, 1.9942120343e+00},
        /* 125 Hz*/ {9.8849666727e-01, 5.7516663664e-03, 1.9882304829e+00},
        /* 250 Hz*/ {9.7712566171e-01, 1.1437169144e-02, 1.9760670839e+00},
        /* 500 Hz*/ {9.5477456091e-01, 2.2612719547e-02, 1.9505892385e+00},
        /* 1k Hz*/  {9.1159452679e-01, 4.4202736607e-02, 1.8952405706e+00},
        /* 2k Hz*/  {8.3100647694e-01, 8.4496761532e-02, 1.7686164442e+00},
        /* 4k Hz*/  {6.9062328809e-01, 1.5468835596e-01, 1.4641227157e+00},
        /* 8k Hz*/  {4.7820368352e-01, 2.6089815824e-01, 7.3910184176e-01},
        /* 16k Hz*/ {2.5620076154e-01, 3.7189961923e-01, -6.2810038077e-01}
};
IIRCoefficients irrcef_15band44k[] = {
        /* 25 Hz*/  {9.9834072702e-01, 8.2963648917e-04, 1.9983280505e+00},
        /* 40 Hz*/  {9.9734652663e-01, 1.3267366865e-03, 1.9973140908e+00},
        /* 63 Hz*/  {9.9582396353e-01, 2.0880182333e-03, 1.9957435641e+00},
        /* 100 Hz*/ {9.9337951306e-01, 3.3102434709e-03, 1.9931771947e+00},
        /* 160 Hz*/ {9.8942832039e-01, 5.2858398053e-03, 1.9889114258e+00},
        /* 250 Hz*/ {9.8353109588e-01, 8.2344520610e-03, 1.9822729654e+00},
        /* 400 Hz*/ {9.7378088082e-01, 1.3109559588e-02, 1.9705764276e+00},
        /* 630 Hz*/ {9.5901979676e-01, 2.0490101620e-02, 1.9511333590e+00},
        /* 1k Hz*/  {9.3574903986e-01, 3.2125480071e-02, 1.9161350100e+00},
        /* 1.6k Hz*/{8.9923630641e-01, 5.0381846793e-02, 1.8501014162e+00},
        /* 2.5k Hz*/{8.4722457681e-01, 7.6387711593e-02, 1.7312785699e+00},
        /* 4k Hz*/  {7.6755471307e-01, 1.1622264346e-01, 1.4881981417e+00},
        /* 6.3k Hz*/{6.6125377473e-01, 1.6937311263e-01, 1.0357747868e+00},
        /* 10k Hz*/ {5.2683267950e-01, 2.3658366025e-01, 2.2218349322e-01},
        /* 16k Hz*/ {4.0179628792e-01, 2.9910185604e-01, -9.1248032613e-01}
};
IIRCoefficients irrcef_15band48k[] = {
        /* 25 Hz*/  {9.9847546664e-01, 7.6226668143e-04, 1.9984647656e+00},
        /* 40 Hz*/  {9.9756184654e-01, 1.2190767289e-03, 1.9975344645e+00},
        /* 63 Hz*/  {9.9616261379e-01, 1.9186931041e-03, 1.9960947369e+00},
        /* 100 Hz*/ {9.9391578543e-01, 3.0421072865e-03, 1.9937449618e+00},
        /* 160 Hz*/ {9.9028307215e-01, 4.8584639242e-03, 1.9898465702e+00},
        /* 250 Hz*/ {9.8485897264e-01, 7.5705136795e-03, 1.9837962543e+00},
        /* 400 Hz*/ {9.7588512657e-01, 1.2057436715e-02, 1.9731772447e+00},
        /* 630 Hz*/ {9.6228521814e-01, 1.8857390928e-02, 1.9556164694e+00},
        /* 1k Hz*/  {9.4080933132e-01, 2.9595334338e-02, 1.9242054384e+00},
        /* 1.6k Hz*/{9.0702059196e-01, 4.6489704022e-02, 1.8653476166e+00},
        /* 2.5k Hz*/{8.5868004289e-01, 7.0659978553e-02, 1.7600401337e+00},
        /* 4k Hz*/  {7.8409610788e-01, 1.0795194606e-01, 1.5450725522e+00},
        /* 6.3k Hz*/{6.8332861002e-01, 1.5833569499e-01, 1.1426447155e+00},
        /* 10k Hz*/ {5.5267518228e-01, 2.2366240886e-01, 4.0186190803e-01},
        /* 16k Hz*/ {4.1811888447e-01, 2.9094055777e-01, -7.0905944223e-01}
};
IIRCoefficients irrcef_25band44k[] = {
        /* 20 Hz*/   {9.9934037157e-01, 3.2981421662e-04, 1.9993322545e+00},
        /* 31.5 Hz*/ {9.9896129025e-01, 5.1935487310e-04, 1.9989411587e+00},
        /* 40 Hz*/   {9.9868118265e-01, 6.5940867495e-04, 1.9986487252e+00},
        /* 50 Hz*/   {9.9835175161e-01, 8.2412419683e-04, 1.9983010452e+00},
        /* 80 Hz*/   {9.9736411067e-01, 1.3179446674e-03, 1.9972343673e+00},
        /* 100 Hz*/  {9.9670622662e-01, 1.6468866919e-03, 1.9965035707e+00},
        /* 125 Hz*/  {9.9588448566e-01, 2.0577571681e-03, 1.9955679690e+00},
        /* 160 Hz*/  {9.9473519326e-01, 2.6324033689e-03, 1.9942169198e+00},
        /* 250 Hz*/  {9.9178600786e-01, 4.1069960678e-03, 1.9905226414e+00},
        /* 315 Hz*/  {9.8966154150e-01, 5.1692292513e-03, 1.9876580847e+00},
        /* 400 Hz*/  {9.8689036168e-01, 6.5548191616e-03, 1.9836646251e+00},
        /* 500 Hz*/  {9.8364027156e-01, 8.1798642207e-03, 1.9786090689e+00},
        /* 800 Hz*/  {9.7395577681e-01, 1.3022111597e-02, 1.9611472340e+00},
        /* 1k Hz*/   {9.6755437936e-01, 1.6222810321e-02, 1.9476180811e+00},
        /* 1.25k Hz*/{9.5961458750e-01, 2.0192706249e-02, 1.9286193446e+00},
        /* 1.6k Hz*/ {9.4861481164e-01, 2.5692594182e-02, 1.8982024567e+00},
        /* 2.5k Hz*/ {9.2095325455e-01, 3.9523372724e-02, 1.8003794694e+00},
        /* 3.15k Hz*/{9.0153642498e-01, 4.9231787512e-02, 1.7132251201e+00},
        /* 4k Hz*/   {8.7685876255e-01, 6.1570618727e-02, 1.5802270232e+00},
        /* 5k Hz*/   {8.4886734822e-01, 7.5566325889e-02, 1.3992391376e+00},
        /* 8k Hz*/   {7.7175298860e-01, 1.1412350570e-01, 7.4018523020e-01},
        /* 10k Hz*/  {7.2627049462e-01, 1.3686475269e-01, 2.5120552756e-01},
        /* 12.5k Hz*/{6.7674787974e-01, 1.6162606013e-01, -3.4978377639e-01},
        /* 16k Hz*/  {6.2482197550e-01, 1.8758901225e-01, -1.0576558797e+00},
        /* 20k Hz*/  {6.1776148240e-01, 1.9111925880e-01, -1.5492465594e+00}
};
IIRCoefficients irrcef_25band48k[] = {
        /* 20 Hz*/   {9.9939388451e-01, 3.0305774630e-04, 1.9993870327e+00},
        /* 31.5 Hz*/ {9.9904564663e-01, 4.7717668529e-04, 1.9990286528e+00},
        /* 40 Hz*/   {9.9878827195e-01, 6.0586402557e-04, 1.9987608731e+00},
        /* 50 Hz*/   {9.9848556942e-01, 7.5721528829e-04, 1.9984427652e+00},
        /* 80 Hz*/   {9.9757801538e-01, 1.2109923088e-03, 1.9974684869e+00},
        /* 100 Hz*/  {9.9697343933e-01, 1.5132803374e-03, 1.9968023538e+00},
        /* 125 Hz*/  {9.9621823598e-01, 1.8908820086e-03, 1.9959510180e+00},
        /* 160 Hz*/  {9.9516191728e-01, 2.4190413595e-03, 1.9947243453e+00},
        /* 250 Hz*/  {9.9245085008e-01, 3.7745749576e-03, 1.9913840669e+00},
        /* 315 Hz*/  {9.9049749914e-01, 4.7512504310e-03, 1.9888056233e+00},
        /* 400 Hz*/  {9.8794899744e-01, 6.0255012789e-03, 1.9852245824e+00},
        /* 500 Hz*/  {9.8495930023e-01, 7.5203498850e-03, 1.9807093500e+00},
        /* 800 Hz*/  {9.7604570090e-01, 1.1977149551e-02, 1.9652207158e+00},
        /* 1k Hz*/   {9.7014963927e-01, 1.4925180364e-02, 1.9532947360e+00},
        /* 1.25k Hz*/{9.6283181641e-01, 1.8584091793e-02, 1.9366149237e+00},
        /* 1.6k Hz*/ {9.5268463224e-01, 2.3657683878e-02, 1.9100137880e+00},
        /* 2.5k Hz*/ {9.2711765003e-01, 3.6441174983e-02, 1.8248457659e+00},
        /* 3.15k Hz*/{9.0912548757e-01, 4.5437256213e-02, 1.7491177803e+00},
        /* 4k Hz*/   {8.8619860800e-01, 5.6900696000e-02, 1.6334959111e+00},
        /* 5k Hz*/   {8.6010264114e-01, 6.9948679430e-02, 1.4757186436e+00},
        /* 8k Hz*/   {7.8757448309e-01, 1.0621275845e-01, 8.9378724155e-01},
        /* 10k Hz*/  {7.4415362476e-01, 1.2792318762e-01, 4.5142017567e-01},
        /* 12.5k Hz*/{6.9581428034e-01, 1.5209285983e-01, -1.1091156053e-01},
        /* 16k Hz*/  {6.4120506488e-01, 1.7939746756e-01, -8.2060253244e-01},
        /* 20k Hz*/  {6.0884213704e-01, 1.9557893148e-01, -1.3932981614e+00}
};
IIRCoefficients irrcef_31band44k[] = {
        /* 20 Hz*/   {9.9934037157e-01, 3.2981421662e-04, 1.9993322545e+00},
        /* 25 Hz*/   {9.9917555233e-01, 4.1222383516e-04, 1.9991628705e+00},
        /* 31.5 Hz*/ {9.9896129025e-01, 5.1935487310e-04, 1.9989411587e+00},
        /* 40 Hz*/   {9.9868118265e-01, 6.5940867495e-04, 1.9986487252e+00},
        /* 50 Hz*/   {9.9835175161e-01, 8.2412419683e-04, 1.9983010452e+00},
        /* 63 Hz*/   {9.9792365217e-01, 1.0381739160e-03, 1.9978431682e+00},
        /* 80 Hz*/   {9.9736411067e-01, 1.3179446674e-03, 1.9972343673e+00},
        /* 100 Hz*/  {9.9670622662e-01, 1.6468866919e-03, 1.9965035707e+00},
        /* 125 Hz*/  {9.9588448566e-01, 2.0577571681e-03, 1.9955679690e+00},
        /* 160 Hz*/  {9.9473519326e-01, 2.6324033689e-03, 1.9942169198e+00},
        /* 200 Hz*/  {9.9342335280e-01, 3.2883236020e-03, 1.9926141028e+00},
        /* 250 Hz*/  {9.9178600786e-01, 4.1069960678e-03, 1.9905226414e+00},
        /* 315 Hz*/  {9.8966154150e-01, 5.1692292513e-03, 1.9876580847e+00},
        /* 400 Hz*/  {9.8689036168e-01, 6.5548191616e-03, 1.9836646251e+00},
        /* 500 Hz*/  {9.8364027156e-01, 8.1798642207e-03, 1.9786090689e+00},
        /* 630 Hz*/  {9.7943153305e-01, 1.0284233476e-02, 1.9714629236e+00},
        /* 800 Hz*/  {9.7395577681e-01, 1.3022111597e-02, 1.9611472340e+00},
        /* 1k Hz*/   {9.6755437936e-01, 1.6222810321e-02, 1.9476180811e+00},
        /* 1.25k Hz*/{9.5961458750e-01, 2.0192706249e-02, 1.9286193446e+00},
        /* 1.6k Hz*/ {9.4861481164e-01, 2.5692594182e-02, 1.8982024567e+00},
        /* 2k Hz*/   {9.3620971896e-01, 3.1895140519e-02, 1.8581325022e+00},
        /* 2.5k Hz*/ {9.2095325455e-01, 3.9523372724e-02, 1.8003794694e+00},
        /* 3.15k Hz*/{9.0153642498e-01, 4.9231787512e-02, 1.7132251201e+00},
        /* 4k Hz*/   {8.7685876255e-01, 6.1570618727e-02, 1.5802270232e+00},
        /* 5k Hz*/   {8.4886734822e-01, 7.5566325889e-02, 1.3992391376e+00},
        /* 6.3k Hz*/ {8.1417575446e-01, 9.2912122771e-02, 1.1311200817e+00},
        /* 8k Hz*/   {7.7175298860e-01, 1.1412350570e-01, 7.4018523020e-01},
        /* 10k Hz*/  {7.2627049462e-01, 1.3686475269e-01, 2.5120552756e-01},
        /* 12.5k Hz*/{6.7674787974e-01, 1.6162606013e-01, -3.4978377639e-01},
        /* 16k Hz*/  {6.2482197550e-01, 1.8758901225e-01, -1.0576558797e+00},
        /* 20k Hz*/  {6.1776148240e-01, 1.9111925880e-01, -1.5492465594e+00}
};
IIRCoefficients irrcef_31band48k[] = {
        /* 20 Hz*/   {9.9939388451e-01, 3.0305774630e-04, 1.9993870327e+00},
        /* 25 Hz*/   {9.9924247917e-01, 3.7876041632e-04, 1.9992317740e+00},
        /* 31.5 Hz*/ {9.9904564663e-01, 4.7717668529e-04, 1.9990286528e+00},
        /* 40 Hz*/   {9.9878827195e-01, 6.0586402557e-04, 1.9987608731e+00},
        /* 50 Hz*/   {9.9848556942e-01, 7.5721528829e-04, 1.9984427652e+00},
        /* 63 Hz*/   {9.9809219264e-01, 9.5390367779e-04, 1.9980242502e+00},
        /* 80 Hz*/   {9.9757801538e-01, 1.2109923088e-03, 1.9974684869e+00},
        /* 100 Hz*/  {9.9697343933e-01, 1.5132803374e-03, 1.9968023538e+00},
        /* 125 Hz*/  {9.9621823598e-01, 1.8908820086e-03, 1.9959510180e+00},
        /* 160 Hz*/  {9.9516191728e-01, 2.4190413595e-03, 1.9947243453e+00},
        /* 200 Hz*/  {9.9395607757e-01, 3.0219612131e-03, 1.9932727986e+00},
        /* 250 Hz*/  {9.9245085008e-01, 3.7745749576e-03, 1.9913840669e+00},
        /* 315 Hz*/  {9.9049749914e-01, 4.7512504310e-03, 1.9888056233e+00},
        /* 400 Hz*/  {9.8794899744e-01, 6.0255012789e-03, 1.9852245824e+00},
        /* 500 Hz*/  {9.8495930023e-01, 7.5203498850e-03, 1.9807093500e+00},
        /* 630 Hz*/  {9.8108651246e-01, 9.4567437704e-03, 1.9743538683e+00},
        /* 800 Hz*/  {9.7604570090e-01, 1.1977149551e-02, 1.9652207158e+00},
        /* 1k Hz*/   {9.7014963927e-01, 1.4925180364e-02, 1.9532947360e+00},
        /* 1.25k Hz*/{9.6283181641e-01, 1.8584091793e-02, 1.9366149237e+00},
        /* 1.6k Hz*/ {9.5268463224e-01, 2.3657683878e-02, 1.9100137880e+00},
        /* 2k Hz*/   {9.4122788957e-01, 2.9386055213e-02, 1.8750821533e+00},
        /* 2.5k Hz*/ {9.2711765003e-01, 3.6441174983e-02, 1.8248457659e+00},
        /* 3.15k Hz*/{9.0912548757e-01, 4.5437256213e-02, 1.7491177803e+00},
        /* 4k Hz*/   {8.8619860800e-01, 5.6900696000e-02, 1.6334959111e+00},
        /* 5k Hz*/   {8.6010264114e-01, 6.9948679430e-02, 1.4757186436e+00},
        /* 6.3k Hz*/ {8.2760520925e-01, 8.6197395374e-02, 1.2405797786e+00},
        /* 8k Hz*/   {7.8757448309e-01, 1.0621275845e-01, 8.9378724155e-01},
        /* 10k Hz*/  {7.4415362476e-01, 1.2792318762e-01, 4.5142017567e-01},
        /* 12.5k Hz*/{6.9581428034e-01, 1.5209285983e-01, -1.1091156053e-01},
        /* 16k Hz*/  {6.4120506488e-01, 1.7939746756e-01, -8.2060253244e-01},
        /* 20k Hz*/  {6.0884213704e-01, 1.9557893148e-01, -1.3932981614e+00},
};

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/