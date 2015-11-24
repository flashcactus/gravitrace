//////////////////////////////////////////
////                 QUATERNION CLASS                 ////
//////////////////////////////////////////
////                    based upon the                    ////
////                 original code of                 ////
////                    Andrew Burbanks                 ////
////                    and expanded by                 ////
////                    Alessandro Rosa                 ////
//////////////////////////////////////////
// Please, send comments and suggestions /
//////////////////////////////////////////
// Alessandro Rosa : zandor_zz@yahoo.it    /
//////////////////////////////////////////

#include "quaternions.h"

///////////////////////////////////////////////////////////////
static int factorial(int p)
{
	if ( p == 0 || p == 1 ) return 1;
	else if ( p < 0 ) return 0;

	int f = 1 ;

	while( p > 1 )
	{
		f *= p ;
		p-- ;
	}

	return f;
}
///////////////////////////////////////////////////////////////

quaternion::quaternion( double rr, double ii, double jj, double kk ): r(rr),i(ii),j(jj),k(kk)
{}

double quaternion::norm() const
{
    return r*r+i*i+j*j+k*k;
}

double quaternion::abs() const
{
    return sqrt(norm());
}

quaternion operator+(const quaternion& x, const quaternion& y)
{
    quaternion z = x ;
    return z += y; // derived operator
}

quaternion operator-(const quaternion& x, const quaternion& y)
{
    quaternion z = x ;
    return z -= y; // derived operator
}

quaternion operator*(const quaternion& x, const double y)
{
    quaternion z = x ;
    return z *= y; // derived operator
}

quaternion operator*(const quaternion& x, const quaternion& y)
{
    quaternion z = x ;
    return z *= y; // derived operator
}

quaternion operator/(const quaternion& x, const quaternion& y)
{
    quaternion z = x ;
    return z /= y; // derived operator
}

quaternion quaternion::conjugate()
{
    return quaternion(r,-i,-j,-k);
}

quaternion quaternion::square()
{
    double temp = 2*r ;
    double new_r = r * r - ( i * i + j * j + k * k );
    quaternion sq(new_r,
    i * temp,
    j * temp, 
    k * temp 
    );
    return sq;
}

quaternion quaternion::invert()
{
    double temp = norm();
    quaternion inv( 
    r / temp,
    i / -temp,
    j / -temp,
    k / -temp
    );
    return inv;
}

quaternion& quaternion::operator+=(const quaternion& b)
{
    r+=b.r; i+=b.i; j+=b.j; k+=b.k; return *this;
}

quaternion& quaternion::operator-=(const quaternion& b)
{
    r-=b.r; i-=b.i; j-=b.j; k-=b.k; return *this;
}

quaternion& quaternion::operator*=(const double b)
{
    r*=b; i*=b; j*=b; k*=b; return *this;
}

quaternion& quaternion::operator/=(const double b)
{
    r/=b; i/=b; j/=b; k/=b; return *this;
}

quaternion& quaternion::operator*=(const quaternion& qu )
{
    double tmp_r = r * qu.r - i * qu.i - j * qu.j - k * qu.k ;
    double tmp_i = r * qu.i + i * qu.r + j * qu.k - k * qu.j ;
    double tmp_j = r * qu.j - i * qu.k + j * qu.r + k * qu.i ;
    double tmp_k = r * qu.k + i * qu.j - j * qu.i + k * qu.r ;
    
    r = tmp_r , i = tmp_i , j = tmp_j , k = tmp_k ;

    return *this ;
}

quaternion& quaternion::operator/=(const quaternion& b)
{
    quaternion temp = b;
    return (*this)*=(temp.invert());            // Ugh!
}

int operator==(const quaternion& a, const quaternion& b)
{
    return (a.r==b.r && a.i==b.i && a.j==b.j && a.k==b.k);
}

int operator!=(const quaternion& a, const quaternion& b)
{
    return !(a==b); // derived operator
}

