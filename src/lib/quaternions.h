//////////////////////////////////////////
////         QUATERNION CLASS         ////
//////////////////////////////////////////
////          based upon the          ////
////         original code of         ////
////          Andrew Burbanks         ////
////          and expanded by         ////
////          Alessandro Rosa         ////
//////////////////////////////////////////
// Please, send comments and suggestions /
//////////////////////////////////////////
// Alessandro Rosa : zandor_zz@yahoo.it  /
//////////////////////////////////////////
#ifndef _QUATERNION_H
#define _QUATERNION_H

#include <stdlib.h>
#include <math.h>


#define TRUE 1
#define FALSE 0

/////////////////////////////////////////////////
#define PI	3.141592653589793
#define E	2.718282
/////////////////////////////////////////////////

class quaternion
{
  friend int operator==(const quaternion& a, const quaternion& b);
  friend int operator!=(const quaternion& a, const quaternion& b);
  friend quaternion operator-(quaternion& x);
  friend quaternion operator+(const quaternion& x, const quaternion& y);
  friend quaternion operator-(const quaternion& x, const quaternion& y);
  friend quaternion operator*(const quaternion& x, const double y);
  friend quaternion operator*(const quaternion& x, const quaternion& y);
  friend quaternion operator/(const quaternion& x, const quaternion& y);

public:
  double r;
  double i;
  double j;
  double k;

public:
  quaternion() { r = i = j = k = 0.0; }                   // uninitialized

  quaternion(double rr) : r(rr),i(0),j(0),k(0) {}

  quaternion( double, double, double, double );
  quaternion(const quaternion& b)
  {
	  r = b.r ,i = b.i , j = b.j , k = b.k ; // copy constructor
  }

  double GetR() const { return r ;}
  double GetI() const { return i ;}
  double GetJ() const { return j ;}
  double GetK() const { return k ;}

  double norm() const;
  double abs() const;
  friend quaternion q_abs( quaternion h )	{	
            h.r = ( h.r < 0.0 ) ? -h.r : h.r ;
            h.i = ( h.i < 0.0 ) ? -h.i : h.i ;
            h.j = ( h.j < 0.0 ) ? -h.j : h.j ;
            h.k = ( h.k < 0.0 ) ? -h.k : h.k ;
            
            return h ;
    }
  quaternion& operator=(const quaternion& b) {r=b.r; i=b.i; j=b.j; k=b.k; return *this;}

  quaternion conjugate();
  quaternion square();

  quaternion invert();

  quaternion& operator+=(const quaternion& b);
  quaternion& operator-=(const quaternion& b);
  quaternion& operator*=(const double b);
  quaternion& operator/=(const double b);
  quaternion& operator*=(const quaternion& b);
  quaternion& operator/=(const quaternion& b);
  
public:
	void q_plugin( double rr , double ii , double jj , double kk )		
	{	r = rr ;	i = ii ;	j = jj ;	k = kk ;	}

public:
	double distancefrom( quaternion qu_to )
	{
		return pow( pow( r - qu_to.r , 2.0 ) + pow( i - qu_to.i , 2.0 ) 
					+ pow( j - qu_to.j , 2.0 ) + pow( k - qu_to.k , 2.0 ) , 0.5 );
	}

public:
	friend quaternion q_power( quaternion , int );

public: // trigonometric
	friend quaternion q_sin( quaternion );
	friend quaternion q_cos( quaternion );
	friend quaternion q_tg( quaternion );
	friend quaternion q_ctg( quaternion );

public: // trascendental
	friend quaternion q_exp( quaternion );
	friend quaternion q_ln( quaternion );

public: // hyperbolic
	friend quaternion q_sinh( quaternion );
	friend quaternion q_cosh( quaternion );
	friend quaternion q_tgh( quaternion );
	friend quaternion q_ctgh( quaternion );

public: // inverse trigonometric
	friend quaternion q_arcsin( quaternion );
	friend quaternion q_arccos( quaternion );
	friend quaternion q_arctg( quaternion );
	friend quaternion q_arcctg( quaternion );
};
#include "quaternions.cpp"
#endif //_QUATERNION_H

