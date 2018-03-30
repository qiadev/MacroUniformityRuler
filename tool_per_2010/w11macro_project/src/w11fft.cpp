/*
 *  w11fft.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 5/1/08.
 *
 *	This contains is my own distinct implementation of a 1D radix-2, decimation-in-time FFT,
 *	based on ideas in the paper "FFT Algorithms" by Brian Gough, May 1997.
 *
 *
 */

#include "w11fft.h"

#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;




long w11FFTBase::validatePowerOf2(long n)
{
	long k= 0;
	long m= 0;
	for (k=1; k < n; k *= 2, m++);
	assert(k == n);
	return m;
}


string w11FFTBase::binrep(const short int n, const short int nbase)
{
	ostringstream ost;
	
	long nnbase= nbase;
	if (nnbase == 0) {
		nnbase= 15;
	}
	long thebit= 0x1 << (nnbase-1);
	for (long i=0; i < nnbase; i++) {
		ost << ((n & thebit) != 0);
		thebit= thebit >> 1;
	}
	return ost.str();	
	// check that this is correct
}


long* w11FFTBase::bitReversal(const long n)
{
	long *permutation= new long[n];
	long M= n / 2;
	
	permutation[0]= 0;
	permutation[1]= M;
	long gstart= 1;		// start index for previous group
	long gcount= 1;		// # members of previous group
	long idelta= M;
	while (idelta >= 2) {
		idelta /= 2;
		long idst= gstart + gcount;	// destination index
		for (long isrc=gstart; isrc < gstart+gcount; isrc++) {
			permutation[idst++]= permutation[isrc] - idelta;
		}
		for (long isrc=gstart; isrc < gstart+gcount; isrc++) {
			permutation[idst++]= permutation[isrc] + idelta;
		}
		gstart += gcount;
		gcount *= 2;
	}
	return permutation;
}
















