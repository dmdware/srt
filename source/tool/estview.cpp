

#include "estview.h"
#include "../texture.h"
#include "procvid.h"
#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec3d.h"
#include "../math/3dmath.h"
#include "../window.h"


//iph 6 p
//horiz 42.5 
//vert 29

float g_xfov = 42.5;
float g_yfov = 29;

/*
x width: 800
y height: 600
x FOV: 45.00 deg
y FOV: 33.75 deg

camera_x_fov 63.54
camera_y_fov 35.7972
*/

#if 0
Vec3f OnNearPersp(int x, int y, int width, int height, Vec3f posvec, Vec3f sidevec, Vec3f upvec, Vec3f viewdir, float fov, float mind)
{
	viewdir = Normalize(viewdir);

	float halfWidth = (float)width / 2.0f;
	float halfHeight = (float)height / 2.0f;

	float ratiox = (x - halfWidth) / halfWidth;
	float ratioy = -(y - halfHeight) / halfHeight;

	float aspect = fabsf((float)width / (float)height);
	float Hnear = 2 * tan( DEGTORAD(fov) / 2) * mind;
	float Wnear = Hnear * aspect;

	//return ( c->m_pos + c->m_strafe * ratioX * Wnear + c->up2() * ratioY * Hnear );
	Vec3f result = viewdir * mind + posvec + sidevec * ratiox * Wnear + upvec * ratioy * Hnear;

	//g_applog<<"--------------"<<std::endl;
	//g_applog<<"onnear x,y="<<x<<","<<y<<" w,h="<<width<<","<<height<<" halfw,h="<<halfWidth<<","<<halfHeight<<" ratiox,y="<<ratiox<<","<<ratioy<<" aspect="<<aspect<<" W,Hnear="<<Wnear<<","<<Hnear<<std::endl;
	//g_applog<<"onnear resultx,y,z="<<result.x<<","<<result.y<<","<<result.z<<std::endl;
	//g_applog<<"--------------"<<std::endl;

	return result;
}
#endif

#if 0
void EstRotTl(LoadedTex* comptex, LoadedTex* colortex, LoadedTex* colortex2, int fin)
{
	//estimated in all the pixel comparisons
	float pitch = 0;
	float yaw = 0;
	float roll = 0;
	Vec3f translation;
	float count = 0;

	//vector from observer O to observer O'
	Vec3f OO2;
	float dOO2;
	
	//vector from observer O to point P
	Vec3f* PO = new Vec3f[ comptex->sizex * comptex->sizey ];
	float* dPO = new float[ comptex->sizex * comptex->sizey ];


	//list of pixel candidates for quick comparison
	struct PixOff
	{
		int pixx;
		int pixy;
		signed char offx;
		signed char offy;
	};

	std::list<PixOff> candidates;

	g_yfov = g_xfov * (float)comptex->sizey / (float)comptex->sizex;
	
	float aspect = fabsf((float)colortex->sizex / (float)colortex->sizey);
	float Hnear = 2 * tan( DEGTORAD(g_xfov) / 2) * MIN_DISTANCE;
	float Wnear = 2 * tan( DEGTORAD(g_yfov) / 2) * MIN_DISTANCE;

	//angle of of pixel x coordinate in reference frame 1

	char outfile[32];
	sprintf(outfile, "out/frame%06d.txt", fin);
	FILE* fp = fopen(outfile, "w");

	Camera cam, cam2;
	
	Camera prevcam2;
	Camera prevprevcam2;

	float score;
	float prevscore = -1;
	float prevprevscore = -1;

	float* pixofferrs = new float [ comptex->sizex * comptex->sizey ];
	float* depths = new float [ comptex->sizex * comptex->sizey ];

	bool done = false;

	for(int pixx=0; pixx<comptex->sizex; pixx++)
	{
		for(int pixy=0; pixy<comptex->sizey; pixy++)
		{
			unsigned char* shift = &comptex->data[ (pixx + pixy * comptex->sizex) * comptex->channels ];
			
			if( !shift[0] || !shift[1])
				continue;

			if( shift[0] == 127 && shift[1] == 127 )
				continue;

			int pixx2 = pixx + shift[0] - 127;
			int pixy2 = pixy + shift[1] - 127;

			//degrees in reference frame 1 along side and local up vectors of x and y pixel coords
			float xPa;
			float yPa;

			//figure out the above
			float ratiox = (pixx - comptex->sizex/2) / (float)(comptex->sizex/2);
			float ratioy = ((comptex->sizey - pixy) - comptex->sizey/2) / (float)(comptex->sizey/2);
			
			float nearx = ratiox * Wnear;
			float neary = ratioy * Hnear;

			//tan = o / a = nearx / MIN_DISTANCE

			// pixx 0.000000 pixy 0.000000 :: ratiox -1.000000 ratioy 1.000000 :: nearx -0.517235 neary 0.777757 :: xPOSa -27.349596 yPOUa 37.874260
			
			//float xOPa = g_xfov/2 + RADTODEG( atan(nearx / MIN_DISTANCE) );
			//float yOPa = g_yfov/2 + RADTODEG( atan(neary / MIN_DISTANCE) );
			
			xPa = g_xfov/2 + RADTODEG( atan(nearx / MIN_DISTANCE) );
			yPa = g_yfov/2 + RADTODEG( atan(neary / MIN_DISTANCE) );

#if 0
			//eq 1.
			float xPOSa = (g_xfov - xPa) + (90 - g_xfov/2);	//P - O - local (right) Side angle
			float yPOUa = (g_yfov - yPa) + (90 - g_yfov/2);	//P - O - local Up angle
			
			float* pix_dPO = &dPO[ pixx + pixy * comptex->sizex ];

			if(count == 0)
			{
				*pix_dPO = 100;	//100 cm = 1 meter
				//this estimate would be better with a depth sensor even at least up to 2.5 m
			}

			//eq 2.
			float xdOSP = sin(xPOSa) * *pix_dPO;
			float ydOUP = sin(yPOUa) * *pix_dPO;

			//eq 3.
			if(count == 0)
			{
				dOO2 = 0.5f;	//0.5 centimeters
			}

			//eq 4. estimate angle between side vec of O and O to O', about O up2 vector and about O view vector () 

#if 1
			fprintf(fp, "pixx %f pixy %f :: ratiox %f ratioy %f :: nearx %f neary %f :: xPa %f yPa %f :: xPOSa %f yPOUa %f \r\n",
				xPa, yPa,
				(float)pixx, (float)pixy,
				ratiox, ratioy,
				nearx, neary,
				xPOSa, yPOUa);
#endif
#else
			//attempt 2 - trying to find side and view vec to point for O and O'


#endif

			Vec3f colorray[2];
			Vec3f colorvec;

			colorvec = ScreenPerspRay(pixx, pixy, comptex->sizex, comptex->sizey, cam.m_pos, cam.m_strafe, cam.up2(), Normalize(cam.m_view - g_cam.m_pos), g_xfov);
			colorray[0] = OnNearPersp(pixx, pixy, comptex->sizex, comptex->sizey, cam.m_pos, cam.m_strafe, cam.up2(), Normalize(cam.m_view - g_cam.m_pos), g_xfov, MIN_DISTANCE);
			colorray[1] = colorray[0] + colorvec;

			done = true;
			break;
		}

		if(done) break;
	}
			
	fclose(fp);
	
	delete [] PO;
	delete [] dPO;
	delete [] pixofferrs;
	delete [] depths;
}
#endif

/*
==========================================
eq5-7

returns point on ray at t=stn
==========================================
*/

Vec3d eq5_sr1(double c2x, double c2y, double c2z, 
	double sr1a, double sr1b, double sr1c,
	double st1)
{
	return Vec3d(
		c2x + sr1a * st1,
		c2y + sr1b * st1,
		c2z + sr1c * st1
		);
}

Vec3d eq6_sr2(double c2x, double c2y, double c2z,
	double sr2a, double sr2b, double sr2c,
	double st2)
{
	return Vec3d(
		c2x + sr2a * st2,
		c2y + sr2b * st2,
		c2z + sr2c * st2
		);
}

Vec3d eq7_sr3(double c2x, double c2y, double c2z,
	double sr3a, double sr3b, double sr3c,
	double st3)
{
	return Vec3d(
		c2x + sr3a * st3,
		c2y + sr3b * st3,
		c2z + sr3c * st3
		);
}


/*
==========================================
eq8-10

returns point on ray at t=stn
==========================================
*/

Vec3d eq8_sr1(Vec3d c2,
	Vec3d c2right, Vec3d c2up, Vec3d c2view,
	double sr1r, double sr1u, double sr1v,
	double st1)
{
	return c2 + (c2right * sr1r + c2up * sr1u + c2view * sr1v) * st1;
}

Vec3d eq9_sr2(Vec3d c2,
	Vec3d c2right, Vec3d c2up, Vec3d c2view,
	double sr2r, double sr2u, double sr2v,
	double st2)
{
	return c2 + (c2right * sr2r + c2up * sr2u + c2view * sr2v) * st2;
}

Vec3d eq10_sr3(Vec3d c2,
	Vec3d c2right, Vec3d c2up, Vec3d c2view,
	double sr3r, double sr3u, double sr3v,
	double st3)
{
	return c2 + (c2right * sr3r + c2up * sr3u + c2view * sr3v) * st3;
}


/*
==========================================
eq11-13

returns point on ray at t=it
==========================================
*/

Vec3d eq11_r1(Vec3d sr1)
{
	return sr1;
}

Vec3d eq12_r2(Vec3d sr2)
{
	return sr2;
}

Vec3d eq13_r3(Vec3d sr3)
{
	return sr3;
}

Vec3d eq11_sr1(Vec3d r1)
{
	return r1;
}

Vec3d eq12_sr2(Vec3d r2)
{
	return r2;
}

Vec3d eq13_sr3(Vec3d r3)
{
	return r3;
}


/*
==========================================
eq17-19

cross of right and view of c2
==========================================
*/

double eq17_c2upx(double c2righty, double c2viewz, double c2rightz, double c2viewy)
{
	return ((c2righty*c2viewz) - (c2rightz*c2viewy));
}

double eq18_c2upy(double c2rightz, double c2viewx, double c2rightx, double c2viewz)
{
	return ((c2rightz*c2viewx) - (c2rightx*c2viewz));
}

double eq19_c2upz(double c2rightx, double c2viewy, double c2righty, double c2viewx)
{
	return ((c2rightx*c2viewy) - (c2righty*c2viewx));
}


/*
==========================================
eq23-25

get c2 up,right,view x,y,z component 
note: square root makes sign uncertain
==========================================
*/

double eq23_c2upx(double c2upy, double c2upz)
{
	return sqrt(1.0 - c2upy*c2upy - c2upz*c2upz);
}
double eq23_c2upy(double c2upx, double c2upz)
{
	return sqrt(1.0 - c2upx*c2upx - c2upz*c2upz);
}
double eq23_c2upz(double c2upx, double c2upy)
{
	return sqrt(1.0 - c2upx*c2upx - c2upy*c2upy);
}

double eq24_c2rightx(double c2righty, double c2rightz)
{
	return sqrt(1.0 - c2righty*c2righty - c2rightz*c2rightz);
}
double eq24_c2righty(double c2rightx, double c2rightz)
{
	return sqrt(1.0 - c2rightx*c2rightx - c2rightz*c2rightz);
}
double eq24_c2rightz(double c2rightx, double c2righty)
{
	return sqrt(1.0 - c2rightx*c2rightx - c2righty*c2righty);
}

double eq25_c2viewx(double c2viewy, double c2viewz)
{
	return sqrt(1.0 - c2viewy*c2viewy - c2viewz*c2viewz);
}
double eq25_c2viewy(double c2viewx, double c2viewz)
{
	return sqrt(1.0 - c2viewx*c2viewx - c2viewz*c2viewz);
}
double eq25_c2viewz(double c2viewx, double c2viewy)
{
	return sqrt(1.0 - c2viewx*c2viewx - c2viewy*c2viewy);
}


/*
==========================================
eq23-25

get c2 up,right,view x,y,z component
note: square root makes sign uncertain
==========================================
*/










Camera g_prevcam;

void EstRotTl(LoadedTex* comptex, LoadedTex* colortex, LoadedTex* colortex2, int fin)
{
	Vec3d rotaxis;
	double rotrad;
	Vec3d translation;
	double count = 0;

	double aspect = fabs((double)colortex->sizex / (double)colortex->sizey);
	double Hnear = 2 * tan(DEGTORAD(g_xfov) / 2) * MIN_DISTANCE;
	double Wnear = 2 * tan(DEGTORAD(g_yfov) / 2) * MIN_DISTANCE;

	Camera cam2;
	//cam2 = g_prevcam;

	class ColourRay
	{
	public:
		Vec3f camorig;
		Vec3f offdir;
		Vec3f somept;
		Vec3f distoff;
	};

	ColourRay* origrays = new ColourRay[comptex->sizex * comptex->sizey];
	ColourRay* offrays = new ColourRay[comptex->sizex * comptex->sizey];
	float* ctobs = new float[comptex->sizex * comptex->sizey];	//c-to-b's
	float* as = new float[comptex->sizex * comptex->sizey];	//a's

	Vec3f origup = Normalize(g_prevcam.up2());
	Vec3f origright = Normalize(g_prevcam.m_strafe);
	Vec3f origview = Normalize(g_prevcam.m_view - g_prevcam.m_pos);

	Vec3f offup = Normalize(cam2.up2());
	Vec3f offright = Normalize(cam2.m_strafe);
	Vec3f offview = Normalize(cam2.m_view - cam2.m_pos);

	const float fardist = 100 * 100;	//100 meters

	for (int pixx = 0; pixx<comptex->sizex; ++pixx)
	{
		for (int pixy = 0; pixy<comptex->sizey; ++pixy)
		{
			unsigned char* shift = &comptex->data[(pixx + pixy * comptex->sizex) * comptex->channels];

			if (!shift[0] || !shift[1])
				continue;	//no data

			if (shift[0] == 127 && shift[1] == 127)
				continue;	//no data

			ColourRay* origray = &origrays[pixx + pixy * comptex->sizex];
			ColourRay* offray = &offrays[pixx + pixy * comptex->sizex];
			float* ctob = &ctobs[pixx + pixy * comptex->sizex];
			float* a = &as[pixx + pixy * comptex->sizex];

			origray->camorig = g_prevcam.m_pos +
				origview * MIN_DISTANCE +
				origright * (-Wnear / 2.0f * (pixx - comptex->sizex / 2) / (float)(comptex->sizex / 2)) +
				origup * (-Hnear / 2.0f * (pixy - comptex->sizey / 2) / (float)(comptex->sizey / 2));

			origray->offdir = Normalize(
				origright * (-Wnear / 2.0f * (pixx - comptex->sizex / 2) / (float)(comptex->sizex / 2)) +
				origup * (-Hnear / 2.0f * (pixy - comptex->sizey / 2) / (float)(comptex->sizey / 2)) 
				);

			int shiftx = (int)shift[0] - 127;
			int shifty = (int)shift[1] - 127;

			//offray is relative to cam2, whatever that happens to be. edit:not
#if 1
#if 0
			offray->somept = origray->camorig +
				origview * MIN_DISTANCE +
				origright * (-Wnear / 2.0f * ((pixx - shiftx) - comptex->sizex / 2) / (float)(comptex->sizex / 2)) +
				origup * (-Hnear / 2.0f * ((pixy - shifty) - comptex->sizey / 2) / (float)(comptex->sizey / 2));
#endif

			//offray->offdir is relative to cam2, whatever that happens to be
			offray->offdir = Normalize(
				offright * (-Wnear / 2.0f * ((pixx-shiftx) - comptex->sizex / 2) / (float)(comptex->sizex / 2)) +
				offup * (-Hnear / 2.0f * ((pixy-shifty) - comptex->sizey / 2) / (float)(comptex->sizey / 2))
				);
#endif
#if 0
			//calc angle offset of shift between frames
			Vec3f Aaxis;
			float Aangle;
			float Aanglex2, Aanglex1, Aangley2, Aangley1;

			float Aanglex = Aanglex2 - Aanglex1;
			float Aangley = Aangley2 - Aangley1;
#endif
		}
	}

	//get 3 shift points at right angles or forming close to an equilateral triangle
	//using origrays and relative offrays of those points and shifted points, get cam2 orientation

	/*

	ray = (A + at, B + bt, C + ct) for t >= 0 

	r1 = (r1A + r1a t1, r1B + r1b t1, r1C + r1c t1) for t1>=0
	r2 = (r2A + r2a t2, r2B + r2b t2, r2C + r2c t2) for t2>=0
	r3 = (r3A + r3a t3, r3B + r3b t3, r3C + r3c t3) for t3>=0

	sr1 = (sr1A + sr1a st1, sr1B + sr1b st1, sr1C + sr1c st1) for st1>=0
	sr2 = (sr2A + sr2a st2, sr2B + sr2b st2, sr2C + sr2c st2) for st2>=0
	sr3 = (sr3A + sr3a st3, sr3B + sr3b st3, sr3C + sr3c st3) for st3>=0

	c1 = original camera pos
	c2 = shift camera pos

	r1 = r2 = r3 = c1 for a t1<=0, t2<=0, t3<=0
	sr1 = sr2 = sr3 = c2 for a st1<=0, st2<=0, st3<=0

	r1 = sr1 for a t1>=0, st1>=0
	r2 = sr2 for a t2>=0, st2>=0
	r3 = sr3 for a t3>=0, st3>=0

	relative to c2,

	c2up = c2 y axis normalized
	c2right = c2 x axis normalized
	c2view = c2 z axis normalized

	snr,snu,snv = right,up,view multiplier ratios
	snr,snu from pixel shift and snv from c2view

	sr1 = (c2) + (c2right sr1r + c2up sr1u + c2view sr1v) (st1 + MIN_DISTANCE)
	sr2 = (c2) + (c2right sr2r + c2up sr2u + c2view sr2v) (st2 + MIN_DISTANCE)
	sr3 = (c2) + (c2right sr3r + c2up sr3u + c2view sr3v) (st3 + MIN_DISTANCE)

	given: all of r1,r2,r3,c1
	sr1r,sr1u,sr1v
	sr2r,sr2u,sr2v
	sr3r,sr3u,sr3v

	find: c2,sr1,sr2,sr3,c2right,c2up,c2view

	sr1 = (c2.x + ( c2right.x sr1r + c2up.x sr1u + c2view.x sr1v ) (st1 + MIN_DISTANCE), 
		c2.y + ( c2right.y sr1r + c2up.y sr1u + c2view.y sr1v ) (st1 + MIN_DISTANCE),
		c2.z + ( c2right.z sr1r + c2up.z sr1u + c2view.z sr1v ) (st1 + MIN_DISTANCE) )
		= (sr1A + sr1a st1, sr1B + sr1b st1, sr1C + sr1c st1)

	sr2 = (c2.x + ( c2right.x sr2r + c2up.x sr2u + c2view.x sr2v ) (st2 + MIN_DISTANCE), 
		c2.y + ( c2right.y sr2r + c2up.y sr2u + c2view.y sr2v ) (st2 + MIN_DISTANCE),
		c2.z + ( c2right.z sr2r + c2up.z sr2u + c2view.z sr2v ) (st2 + MIN_DISTANCE) )
		= (sr2A + sr2a st2, sr2B + sr2b st2, sr2C + sr2c st2)

	sr3 = (c2.x + ( c2right.x sr3r + c2up.x sr3u + c2view.x sr3v ) (st3 + MIN_DISTANCE), 
		c2.y + ( c2right.y sr3r + c2up.y sr3u + c2view.y sr3v ) (st3 + MIN_DISTANCE),
		c2.z + ( c2right.z sr3r + c2up.z sr3u + c2view.z sr3v ) (st3 + MIN_DISTANCE) )
		= (sr3A + sr3a st3, sr3B + sr3b st3, sr3C + sr3c st3)


		r1 = sr1 for a t1>=0, st1>=0 where t1=it1 and st1=ist1
		r2 = sr2 for a t2>=0, st2>=0 where t2=it2 and st2=ist2
		r3 = sr3 for a t3>=0, st3>=0 where t3=it3 and st3=ist3

		(r1A + r1a it1, r1B + r1b it1, r1C + r1c it1)
		= (c2.x + ( c2right.x sr1r + c2up.x sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE),
			c2.y + ( c2right.y sr1r + c2up.y sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE),
			c2.z + ( c2right.z sr1r + c2up.z sr1u + c2view.z sr1v ) (ist1 + MIN_DISTANCE) )

	r1A + r1a it1 = c2.x + ( c2right.x sr1r + c2up.x sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE)
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + c2up.y sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE)
	r1C + r1c it1 = c2.z + ( c2right.z sr1r + c2up.z sr1u + c2view.z sr1v ) (ist1 + MIN_DISTANCE)

	c2up = cross(c2right, c2view)

	c2up.x = ((c2right.y * c2view.z) - (c2right.z * c2view.y))
	c2up.y = ((c2right.z * c2view.x) - (c2right.x * c2view.z))
	c2up.z = ((c2right.x * c2view.y) - (c2right.y * c2view.x))


	r1A + r1a it1 = c2.x + ( c2right.x sr1r + ((c2right.y * c2view.z) - (c2right.z * c2view.y)) sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE)
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + ((c2right.z * c2view.x) - (c2right.x * c2view.z)) sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE)
	r1C + r1c it1 = c2.z + ( c2right.z sr1r + ((c2right.x * c2view.y) - (c2right.y * c2view.x)) sr1u + c2view.z sr1v ) (ist1 + MIN_DISTANCE)

	c2right = cross(c2view, c2up)

	c2right.x = ((c2view.y * ((c2right.x * c2view.y) - (c2right.y * c2view.x)) ) - (c2view.z * ((c2right.z * c2view.x) - (c2right.x * c2view.z)) ))
	c2right.y = ((c2view.z * ((c2right.y * c2view.z) - (c2right.z * c2view.y)) ) - (c2view.x * ((c2right.x * c2view.y) - (c2right.y * c2view.x)) ))
	c2right.z = ((c2view.x * ((c2right.z * c2view.x) - (c2right.x * c2view.z)) ) - (c2view.y * ((c2right.y * c2view.z) - (c2right.z * c2view.y)) ))


	r1A + r1a it1 = c2.x + ( c2right.x sr1r + ((c2right.y * c2view.z) - (c2right.z * c2view.y)) sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE)
	r1A + r1a it1 = c2.x + ( c2right.x sr1r + (c2right.y * c2view.z) sr1u - (c2right.z * c2view.y) sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE)
	r1A + r1a it1 = c2.x + ( c2right.x sr1r + c2right.y c2view.z sr1u - c2right.z c2view.y sr1u + c2view.x sr1v ) (ist1 + MIN_DISTANCE)
	r1A + r1a it1 = c2.x + ( c2right.x sr1r + c2right.y c2view.z sr1u - c2right.z c2view.y sr1u + c2view.x sr1v ) ist1 +
		( c2right.x sr1r + c2right.y c2view.z sr1u - c2right.z c2view.y sr1u + c2view.x sr1v ) MIN_DISTANCE

	r1A + r1a it1 = c2.x + c2right.x sr1r ist1 + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
		c2right.x sr1r MIN_DISTANCE + c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE

	c2right.x sr1r ist1 + c2right.x sr1r MIN_DISTANCE = -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
		c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE
	
	c2right.x sr1 ( ist1 + MIN_DISTANCE ) = -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
		c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE

	c2right.x = ( -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
		c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE ) 
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )

	
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + ((c2right.z * c2view.x) - (c2right.x * c2view.z)) sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE)
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + (c2right.z * c2view.x) sr1u - (c2right.x * c2view.z) sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE)
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + c2right.z c2view.x sr1u - c2right.x c2view.z sr1u + c2view.y sr1v ) (ist1 + MIN_DISTANCE)
	r1B + r1b it1 = c2.y + ( c2right.y sr1r + c2right.z c2view.x sr1u - c2right.x c2view.z sr1u + c2view.y sr1v ) ist1 
		+ ( c2right.y sr1r + c2right.z c2view.x sr1u - c2right.x c2view.z sr1u + c2view.y sr1v ) MIN_DISTANCE

	r1B + r1b it1 = c2.y + c2right.y sr1r ist1 + c2right.z c2view.x sr1u ist1 - c2right.x c2view.z sr1u ist1 + c2view.y sr1v ist1 
		+ c2right.y sr1r MIN_DISTANCE + c2right.z c2view.x sr1u MIN_DISTANCE - c2right.x c2view.z sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE

	c2right.x c2view.z sr1u ist1 + c2right.x c2view.z sr1u MIN_DISTANCE = -r1B - r1b it1 + c2.y + c2right.y sr1r ist1 + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1 
		+ c2right.y sr1r MIN_DISTANCE + c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE

	c2right.x c2view.z sr1u ( ist1 + MIN_DISTANCE ) = -r1B - r1b it1 + c2.y + c2right.y sr1r ist1 + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1 
		+ c2right.y sr1r MIN_DISTANCE + c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE

	c2right.x = ( -r1B - r1b it1 + c2.y + c2right.y sr1r ist1 + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1 
		+ c2right.y sr1r MIN_DISTANCE + c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )


	( -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
		c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )
		= ( -r1B - r1b it1 + c2.y + c2right.y sr1r ist1 + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
		+ c2right.y sr1r MIN_DISTANCE + c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )

	( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
		 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )
		+ ( c2right.y c2view.z sr1u ist1 + c2right.y c2view.z sr1u MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) )
		= 
	( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
		+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
		+ ( c2right.y sr1r ist1 + c2right.y sr1r MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )

	( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
		 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )
		+ c2right.y c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) )
		= 
	( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
		+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
		+ c2right.y sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )

	c2right.y c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) )
	- c2right.y sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
		= 
	( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
		+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
	- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
		 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )

	c2right.y ( c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) ) - sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) ) )
		= 
	( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
		+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
		/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
	- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
		 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
		/ ( sr1 ( ist1 + MIN_DISTANCE ) )

	c2right.y
		= 
	( 
		( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
			+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
			/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
		- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
			 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
			/ ( sr1 ( ist1 + MIN_DISTANCE ) )
	)
	/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) ) - sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) ) )


	r1C + r1c it1 = c2.z + ( c2right.z sr1r + ((c2right.x * c2view.y) - (c2right.y * c2view.x)) sr1u + c2view.z sr1v ) (ist1 + MIN_DISTANCE)

	^^prev:
	c2right.x = ( -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
	c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
	/ ( sr1 ( ist1 + MIN_DISTANCE ) )

	r1C + r1c it1 = c2.z + 
		( c2right.z sr1r + 
			(
				(
					(
						( -r1A - r1a it1 + c2.x + c2right.y c2view.z sr1u ist1 - c2right.z c2view.y sr1u + c2view.x sr1v ist1 +
							c2right.y c2view.z sr1u MIN_DISTANCE - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
						/ ( sr1 ( ist1 + MIN_DISTANCE ) )
					)
					* c2view.y
				) 
				- (c2right.y * c2view.x)
			) sr1u + 
		c2view.z sr1v ) (ist1 + MIN_DISTANCE)

		1 = sqrt( c2right.x * c2right.x + c2right.y * c2right.y + c2right.z * c2right.z )
		1 = sqrt( c2up.x * c2up.x + c2up.y * c2up.y + c2up.z * c2up.z )
		1 = sqrt( c2view.x * c2view.x + c2view.y * c2view.y + c2view.z * c2view.z )

		1 = c2right.x * c2right.x + c2right.y * c2right.y + c2right.z * c2right.z
		1 = c2up.x * c2up.x + c2up.y * c2up.y + c2up.z * c2up.z
		1 = c2view.x * c2view.x + c2view.y * c2view.y + c2view.z * c2view.z

	r1C + r1c it1 = c2.z + 
		( c2right.z sr1r + 
			(
				(
					(
						( -r1A - r1a it1 + c2.x 
							+ (
								( 
									( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
										+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
										/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
									- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
										 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
										/ ( sr1 ( ist1 + MIN_DISTANCE ) )
								)
								/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) ) - sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) ) )
							) c2view.z sr1u ist1 
							- c2right.z c2view.y sr1u + c2view.x sr1v ist1
							+ (
								( 
									( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
										+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
										/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
									- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
										 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
										/ ( sr1 ( ist1 + MIN_DISTANCE ) )
								)
								/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) ) - sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) ) )
							) c2view.z sr1u MIN_DISTANCE 
							- c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
						/ ( sr1 ( ist1 + MIN_DISTANCE ) )
					)
					* c2view.y
				) 
				- (
					(
						( 
							( -r1B - r1b it1 + c2.y + c2right.z c2view.x sr1u ist1 + c2view.y sr1v ist1
								+ c2right.z c2view.x sr1u MIN_DISTANCE + c2view.y sr1v MIN_DISTANCE )
								/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) )
							- ( -r1A - r1a it1 + c2.x - c2right.z c2view.y sr1u + c2view.x sr1v ist1 
								 - c2right.z c2view.y sr1u MIN_DISTANCE + c2view.x sr1v MIN_DISTANCE )
								/ ( sr1 ( ist1 + MIN_DISTANCE ) )
						)
						/ ( c2view.z sr1u ( ist1 + MIN_DISTANCE) / ( sr1 ( ist1 + MIN_DISTANCE ) ) - sr1r ( ist1 + MIN_DISTANCE ) / ( c2view.z sr1u ( ist1 + MIN_DISTANCE ) ) )
					) * c2view.x
				)
			) sr1u + 
		c2view.z sr1v ) (ist1 + MIN_DISTANCE)

	//better use doubles




	r2A + r2a it2 = c2.x + ( c2right.x sr2r + ((c2right.y * c2view.z) - (c2right.z * c2view.y)) sr2u + c2view.x sr2v ) (ist2 + MIN_DISTANCE)
	r2B + r2b it2 = c2.y + ( c2right.y sr2r + ((c2right.z * c2view.x) - (c2right.x * c2view.z)) sr2u + c2view.y sr2v ) (ist2 + MIN_DISTANCE)
	r2C + r2c it2 = c2.z + ( c2right.z sr2r + ((c2right.x * c2view.y) - (c2right.y * c2view.x)) sr2u + c2view.z sr2v ) (ist2 + MIN_DISTANCE)


	r3A + r3a it3 = c2.x + ( c2right.x sr3r + ((c2right.y * c2view.z) - (c2right.z * c2view.y)) sr3u + c2view.x sr3v ) (ist3 + MIN_DISTANCE)
	r3B + r3b it3 = c2.y + ( c2right.y sr3r + ((c2right.z * c2view.x) - (c2right.x * c2view.z)) sr3u + c2view.y sr3v ) (ist3 + MIN_DISTANCE)
	r3C + r3c it3 = c2.z + ( c2right.z sr3r + ((c2right.x * c2view.y) - (c2right.y * c2view.x)) sr3u + c2view.z sr3v ) (ist3 + MIN_DISTANCE)

	*/

	delete[] ctobs;
	delete[] as;
	delete[] origrays;
	delete[] offrays;
}