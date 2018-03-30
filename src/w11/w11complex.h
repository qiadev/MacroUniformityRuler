/*
 *  w11complex.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/9/08.
 *
 */

#ifndef _w11complex__h__

template<typename T> class w11complex {
	T re;
	T im;
public:
	typedef T value_type;
	
	w11complex(const T& r=T(), const T& i=T()) : re(r), im(i) {};
	template<typename X> w11complex(const w11complex<X>& a) : re(a.real()), im(a.imag()) {}; // take care of mixing float and double
	
	inline T real(void) const {return re;};
	inline T imag(void) const {return im;};
	inline T& real(void) {return re;};
	inline T& imag(void) {return im;};
	
	inline w11complex& operator=(const w11complex& z) {
		re= z.real();
		im= z.imag();
		return *this;
	};
	
	inline w11complex& operator+=(const w11complex& z) {
		re += z.real();
		im += z.imag();
		return *this;
	};
	
	inline w11complex& operator-=(const w11complex& z) {
		re -= z.real();
		im -= z.imag();
		return *this;
	};
	
	inline w11complex<T> operator*(w11complex<T> z)
	{
		w11complex<T> zz;
		zz.real() = re * z.real() - im * z.imag();
		zz.imag() = re * z.imag() + im * z.real();
		return z;
	}

};

////
inline w11complex<double> operator*(w11complex<double> x, w11complex<float> y)
{
	w11complex<double> z;
	z.real() = x.real() * y.real() - x.imag() * y.imag();
	z.imag() = x.real() * y.imag() + x.imag() * y.real();
	return z;
}

inline w11complex<double> operator*(w11complex<float> x, w11complex<double> y)
{
	w11complex<double> z;
	z.real() = x.real() * y.real() - x.imag() * y.imag();
	z.imag() = x.real() * y.imag() + x.imag() * y.real();
	return z;
}

inline w11complex<double> operator+(w11complex<double> x, w11complex<float> y)
{
	w11complex<double> z(x.real() + y.real(), x.imag() + y.imag());
	return z;
}

inline w11complex<double> operator+(w11complex<float> x, w11complex<double> y)
{
	w11complex<double> z(x.real() + y.real(), x.imag() + y.imag());
	return z;
}

inline w11complex<double> operator-(w11complex<double> x, w11complex<float> y)
{
	w11complex<double> z(x.real() - y.real(), x.imag() - y.imag());
	return z;
}

inline w11complex<double> operator-(w11complex<float> x, w11complex<double> y)
{
	w11complex<double> z(x.real() - y.real(), x.imag() - y.imag());
	return z;
}


#endif