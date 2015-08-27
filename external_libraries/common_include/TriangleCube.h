#include <math.h>

/* this version of SIGN3 shows some numerical instability, and is improved
 * by using the uncommented macro that follows, and a different test with it */
#ifdef OLD_TEST
	#define SIGN3( A ) (((A).x<0)?4:0 | ((A).y<0)?2:0 | ((A).z<0)?1:0)
#else
	#define EPS 10e-5
	#define SIGN3( A ) \
	  (((A).x < EPS) ? 4 : 0 | ((A).x > -EPS) ? 32 : 0 | \
	   ((A).y < EPS) ? 2 : 0 | ((A).y > -EPS) ? 16 : 0 | \
	   ((A).z < EPS) ? 1 : 0 | ((A).z > -EPS) ? 8 : 0)
#endif

#define CROSS( A, B, C ) { \
  (C).x =  (A).y * (B).z - (A).z * (B).y; \
  (C).y = -(A).x * (B).z + (A).z * (B).x; \
  (C).z =  (A).x * (B).y - (A).y * (B).x; \
   }
#define SUB( A, B, C ) { \
  (C).x =  (A).x - (B).x; \
  (C).y =  (A).y - (B).y; \
  (C).z =  (A).z - (B).z; \
   }
#define LERP( A, B, C) ((B)+(A)*((C)-(B)))
#define MIN3(a,b,c) ((((a)<(b))&&((a)<(c))) ? (a) : (((b)<(c)) ? (b) : (c)))
#define MAX3(a,b,c) ((((a)>(b))&&((a)>(c))) ? (a) : (((b)>(c)) ? (b) : (c)))
#define INSIDE 0
#define OUTSIDE 1

typedef struct {
      float           x;
      float           y;
      float           z;
      } Point3;

typedef struct{
   Point3 v1;                 /* Vertex1 */
   Point3 v2;                 /* Vertex2 */
   Point3 v3;                 /* Vertex3 */
   } Triangle3; 

long t_c_intersection(Triangle3 t);