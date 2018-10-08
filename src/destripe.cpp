#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>

#include <complex>
#include <vector>

using namespace std;

// this declaration used the "vector" type, so must go after "using"
typedef complex<double> CD;

class MeanStd {  // for computing mean and standard deviation
  public:
     MeanStd(){sum = sum2 = 0.0; n=0;}
     void Reset(){sum = sum2 = 0.0; n=0;};
     void Element(double a){sum += a; sum2 += a*a; n++;}
     void Stats(double &avg, double &std){ avg = sum/n; std = sum2/n-avg*avg < 0 ? 0.0 : sqrt(sum2/n - avg*avg);}  // could be off by rounding
     double Mean(){return sum/n;}
     double Sum(){return sum;}
     double Sum2(){return sum2;}
     double Std() {double avg = sum/n; return sum2/n-avg*avg < 0 ? 0.0 : sqrt(sum2/n - avg*avg);}  // could be off by rounding
     double Var() {double avg = sum/n; return sum2/n-avg*avg < 0 ? 0.0 : sum2/n - avg*avg;}        // could be off by rounding
     int HowMany(){return n;}
  private:
     double sum, sum2;
     long int n;  // could average more the 2B elements, so need long
     };
typedef unsigned char uint8;

// Reads an 8 bit png file.
uint8* read_8bit_png_file(const char *file_name, int &w, int &h)
{
FILE *fp = fopen(file_name, "rb");
if (!fp) {
    return NULL;
    }
const int number=8;  // read this many bytes
png_byte header[number];
fread(header, 1, number, fp);
bool is_png = !png_sig_cmp(header, 0, number);
if (!is_png) {
    return NULL;  // not a .png file
    }

// create the structures
png_structp png_ptr = png_create_read_struct
        (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); // use default error handlers
if (!png_ptr)
    return NULL;
png_infop info_ptr = png_create_info_struct(png_ptr);
if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    return NULL;
    }

png_init_io(png_ptr, fp);
png_set_sig_bytes(png_ptr, number);

png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ENDIAN, NULL);
fclose(fp);

// now get some info about the 
int bit_depth, color_type;
png_uint_32 width, height;
png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
printf("width %d, height %d, bit depth %d, color_type %d\n", width, height, bit_depth, color_type);

// get the pointers to each row.
uint8* rslt = (uint8 *)malloc(height*width*sizeof(uint8));
if (bit_depth == 8){
    png_byte **row_ptrs = (png_byte**)png_get_rows(png_ptr, info_ptr);
    int n=0;
    for(int y=0; y<height; y++)
	for(int x=0; x<width; x++)
	    rslt[n++] = row_ptrs[y][x];
    }
else {
    printf("Trying 8 bit read; Cannot read .png file '%s' with bit depth %d\n", file_name, bit_depth);
    free(rslt);
    rslt = NULL;
    }
png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  // we've made a copy, so free original
w = width;   // returned values.  Just to make types 'int' for convenience
h = height;
return rslt;
}

// Writes an in-memory raster as an 8 bit .png file
void write_8bit_png_file(const char* file_name, unsigned char *raster, int width, int  height)
{
/* create file */
FILE *fp = fopen(file_name, "w");
if (fp == NULL) {
    printf("Open for write of 8-bit '%s' failed\n", file_name);
    exit(-1);
    }

/* initialize stuff */
png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
png_infop  info_ptr = png_create_info_struct(png_ptr);
png_init_io(png_ptr, fp);

png_byte bit_depth = 8;
png_byte color_type = PNG_COLOR_TYPE_GRAY;
png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE,
 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
png_write_info(png_ptr, info_ptr);

// create the row pointers
vector<png_byte *> row_pointers(height);
for(int i=0; i<height; i++)
    row_pointers[i] = (png_byte *)(raster+i*width);
png_write_image(png_ptr, &row_pointers[0]);

png_write_end(png_ptr, NULL);
fclose(fp);
}

struct Correction {
  public:
    double left, right;
  Correction(){ left = 0.0; right = 0.0;}
  };

int ROUND(double x) {return x >= 0 ? int(x+0.5) : int (x-0.5);}

int main(int argc, char **argv) {

vector<char *> noa;    // all non-option arguments
bool debug = false;  // product debug images
for(int i=1; i<argc; i++) {
    if (argv[i][0] != '-')
	noa.push_back(argv[i]);
    else if (strcasecmp(argv[i], "-d") == 0)
	debug = true;
    else {
	printf("Unknown option %s\n", argv[i]);
	return 42;
	}
    }
if (noa.size() < 2) {
    printf("Usage:  Destripe [-d] <input file> <output file>\n");
    return 42;
    }
int w, h;
uint8 *image = read_8bit_png_file(noa[0], w, h);
printf("opened '%s', w=%d h=%d\n", noa[0], w, h);

int YC = h/1000;
printf("%d measurement points in Y\n", YC);
vector<int> ys(YC+2);
ys[0] = -1;
for(int i=1; i<=YC; i++)
    ys[i] = int((double(i)-0.5)/YC * h);
ys[YC+1] = h;
for(int i=0; i<ys.size(); i++)
    printf("ys[%2d] = %5d\n", i, ys[i]);

// here are the X values of the seams.  Also two at ends.
int seam[] = {-1, 2066,4684,7574,10385,13222,15937,18531,21198,23826,26506,29175,31772, w};
int NS = sizeof(seam)/sizeof(int);
printf("%d seams\n", NS);

// look at normalizing through one section.   First make plot
vector<double> means_by_col(w);
FILE *fp = fopen("pl", "w");
if (fp == NULL) {
    printf("Could not open 'pl'\n");
    return 42;
    }
for(int x=0; x<w; x++) {
    MeanStd m;
    for(int y=0; y<h; y++)
	m.Element(image[x+y*w]);
    fprintf(fp, "%d %.2f %.2f\n", x, m.Mean(), m.Std() );
    means_by_col[x] = m.Mean();
    }
fclose(fp);
printf("Wrote file 'fp'\n");
for(int i=0; i<NS-1; i++) {
    int xmin = max(seam[i]+1, 0);
    int xmax = min(seam[i+1]-1, w-1);
    MeanStd overall;
    for(int x=xmin; x <= xmax; x++)
	overall.Element(means_by_col[x]);
    printf("slab mean is %.2f\n", overall.Mean());
    for(int x=xmin; x <= xmax; x++) {
	int delta = ROUND(overall.Mean() - means_by_col[x]);
        for(int y=0; y<h; y++) {
	    int pix = image[x+y*w] + delta;
	    pix = min(pix, 255);
	    pix = max(pix,   0);
	    image[x+y*w] = pix;
	    }
	}
    }

// Create an array of corrections.  It's unevenly spaced in X and Y, but with a constant number of points in each row/column.
// Two extra points in each direction; one at 0 and one at the far edge.  All measured points are interior.

int XSIZE = NS;
int YSIZE = YC+2;
vector<vector<Correction>  >corr(XSIZE, vector<Correction>(YSIZE));
printf("Correction vector has %d entries, %d by %d\n", corr.size(), XSIZE, YSIZE);

for(int i=1; i<NS-1; i++) {
    int xmid = seam[i];
    const int DX = 10;
    const int SX = 100;
    for(int iy=1; iy < YSIZE-1; iy++) {
        int y0 = ys[iy];
        MeanStd left, right;
	for(int y=y0-500; y<y0+500 && y < h; y++) {
	    for(int x=DX; x<=SX; x++) {
		left .Element(image[xmid-x + y*w]);
		right.Element(image[xmid+x + y*w]);
		}
	    }
	printf("At y=%5d and xmid = %d, left %.2f %.3f, right %.2f %.3f\n", y0, xmid, left.Mean(), left.Std(), right.Mean(), right.Std() );
        // If both values look plausible, set corrections at this location.
        // Plausible means values between 100-200, std between 15 and 50.
        bool PlausLeft   = left.Mean() > 100 &&  left.Mean() < 220 &&  left.Std() > 15 &&  left.Std() < 60;
        bool PlausRight = right.Mean() > 100 && right.Mean() < 220 && right.Std() > 15 && right.Std() < 60;
        if (PlausLeft && PlausRight) {
	    double mid = (left.Mean() + right.Mean())/2.0;
            corr[i][iy].left = mid - left.Mean();  // amount to add
            corr[i][iy].right= mid - right.Mean();
	    }
	}
    }
for(int y=0; y<YSIZE; y++) {
    for(int x=0; x<XSIZE; x++)
	printf("%5.1f:%5.1f ", corr[x][y].left, corr[x][y].right);
    printf("\n");
    }
// Find the corrections.  Put the X loop on the outside, even though that's bad for the cache, since the X computation is more expensive.
vector<uint8> fake(w*h, 127);
double yslot = double(h)/YC;
for(int x=0; x<w; x++) {
    // find the x bin.  May not exist since exact seams don't count
    int ix;
    for(ix = 0; ix < XSIZE-1; ix++)
	if (x > seam[ix] && x < seam[ix+1])
	    break;
    if (ix >= XSIZE-1) {// found nothing.  Just copy this column.  Fix the case where it's black, since Shinya prefers white.
	printf("No X at %d\n", x);
        for(int y=0; y<h; y++) {
            int pix = image[x+y*w];
            if (pix == 0 && (image[x-1+y*w] != 0 || image[x+1+y*w] != 0)) // if not in a completely black area
		pix = 225;  // Set to a very white value
	    fake[x+y*w] = pix;
	    }
	continue;
	}
    double alpha = (double(x) - seam[ix])/(seam[ix+1] - seam[ix]);
    //printf("alpha %f\n", alpha);
    for(int y=0; y<h; y++) {
	int slot = int(double(y)/yslot + 0.5);
        // check
        if (y < ys[slot] || y > ys[slot+1])
	    printf("Oops.  yslot %f, slot %d, ys %d %d\n", yslot, slot, ys[slot], ys[slot+1]);
	// OK, interpolate
	double beta = (double(y) - ys[slot])/(ys[slot+1] - ys[slot]);
        double incr = (1-alpha)*(1-beta)*corr[ix  ][slot  ].right +
		      (alpha  )*(1-beta)*corr[ix+1][slot  ].left  + 
		      (1-alpha)*(beta  )*corr[ix  ][slot+1].right +
                      (alpha  )*(beta  )*corr[ix+1][slot+1].left  ;
        int delta = incr >= 0 ? int(incr + 0.5) : int(incr - 0.5);  // round to integer
        if (debug && x >= 13220 && x <= 13224 && y >= 20880 && y <= 20883) {
            printf("x %d y %d alpha %f beta %f incr %f delta %d ix %d slot %d \n", x, y, alpha, beta, incr, delta, ix, slot );
            printf(" %5.1f:%5.1f*, +x %5.1f*, %5.1f, +y %5.1f, %5.1f*, +xy %5.1f*, %5.1f   :: %.2f %.2f %.2f %.2f\n", 
	     corr[ix  ][slot  ].left, corr[ix  ][slot  ].right,
	     corr[ix+1][slot  ].left, corr[ix+1][slot  ].right,
	     corr[ix  ][slot+1].left, corr[ix  ][slot+1].right,
	     corr[ix+1][slot+1].left, corr[ix+1][slot+1].right,
	     (1-alpha)*(1-beta)*corr[ix  ][slot  ].right,
	     (alpha  )*(1-beta)*corr[ix+1][slot  ].left ,
             (1-alpha)*(beta  )*corr[ix  ][slot+1].right,
             (alpha  )*(beta  )*corr[ix+1][slot+1].left );
	    }
        int pix = image[x+y*w] + delta;
        //pix = 127 + delta;  // just testing
        pix = min(pix, 255);
	pix = max(pix,   0);
        fake[x+y*w] = pix;
	}
    }
printf("Output file is '%s', %d x %d\n", noa[1], w, h);
write_8bit_png_file(noa[1], &fake[0], w, h);

return 0;
}
