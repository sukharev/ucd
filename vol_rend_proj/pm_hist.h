/* Histogram.
 * Author: Petr Mikulik
 * Version: April 2004
 * License: GNU GPL
 */


/*
 * Use:
 * 1. set bins for number of bins
 * 2. set logbase=0 for equidistant linear range, otherwise to log base
 *    (e.g. 10) for equidistant logarithmic range
 * 3. set either autoscale (default) or zfrom, zto
 * 4. 
 */


struct t_pm_histogram {
    int bins;		// Number of bins.
    double zfrom, zto;	// Limit this range of data, or
    int autoscale;	// ...autoscale them.
    double logbase;	// Bins equidistant in linear or logarithmic space.
    t_pm_histogram () { // default constructor
	bins = 10;
	autoscale = 1;
	logbase = 0; // linear range
    };
} ;


/*
 * Input:   a ... array of data for histogramming
 *	    n ... number of data a[0] .. a[n-1]
 *	    x_explicit ... optional, ascending sequence of opts.bins bins centers
 *
 * Results: x ... centers of bins
 * 	    h ... occurencies
 * 	    return value of the function:
 *		0 for success
 *		1 error in input parameters
 *		2 error in output fields (h is NULL)
 *
 * x and h must have been properly allocated like
    double        *x = malloc(opts.bins * sizeof(double));
    unsigned long *h = malloc(opts.bins * sizeof(unsigned long));
 *
 *
 * Limitation: for h being unsigned int, the limitation is 4*1024^3 values
 * in a bin; thus, we use unsigned long instead -- that is enough even for
 * large volumic data.
 * 
 */
int
pm_histogram (
	double *x, unsigned long *h, 
	const double *a, int n, const t_pm_histogram& opts,
	const double *x_explicit = NULL );



bool run_histogram(	double* a,  					// data
				int numPts,						// number of data points
				int nbins,						// number of bins in the histogram
				/* out */ double *x,			// histogram values
				/* out */ unsigned long *h,     // histogram counters
				double logbase = 0);

bool pm_histogram_nonuniform (double *x,
							  unsigned long *h,
							  int ubin_count,
							  int nonubin_count,
							  double *nonu_x,
							  unsigned long *nonu_h);

// eof pm_hist.h
