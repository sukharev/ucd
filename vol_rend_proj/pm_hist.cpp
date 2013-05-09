/* Histogram.
 * Author: Petr Mikulik
 * Version: April 2004
 * License: GNU GPL
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "pm_hist.h"


#define HIST_DEBUG 0


/*
 * Use:
 * 1. set bins for number of bins
 * 2. set logbase=0 for equidistant linear range, otherwise to log base
 *    (e.g. 10) for equidistant logarithmic range
 * 3. set either autoscale (default) or zfrom, zto
 * 4.
 */

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
 * Limitation: for h being unsigned int, the limitation is 4*1024^3.
 * values in a bin; thus, we use unsigned long instead -- that is enough
 * even for large volumic data.
 *
 */
int
pm_histogram (
	double *x, unsigned long *h,
	const double *a, int n, const t_pm_histogram& opts,
	const double *x_explicit /* = NULL */ )
{
    // Local variables for out convenience:
    int bins = opts.bins;
    int bins_1 = bins - 1;
    // Minimum and maximum values of the histogramming range:
    double xfrom, xto;
    // Calculated step in x (lin or log space) if x_explicit not used
    double xstep;
    // This is used only if x the routine is called with x == NULL
    double *xtmp = NULL;
    // Local variables:
    int j, k;

    // Try to be foolproof
    if (bins <= 0 || n <= 0)
	return 1;
    if (!h)
	return 2;

    if (x_explicit) { // x_explicit, if given, must be ascending
	for (j = 1; j < bins; j++)
	    if (x_explicit[j-1] >= x_explicit[j])
		return 1;
    }

    // Set the bin range [xfrom..xto]:
    if (x_explicit) {
	// Explicitly given bins
	xfrom = x_explicit[0];
	xto   = x_explicit[bins_1];
    } else {
	if (!opts.autoscale) {
	    // Use range values from options
	    xfrom = (opts.zfrom < opts.zto) ? opts.zfrom : opts.zto;
	    xto   = (opts.zfrom > opts.zto) ? opts.zfrom : opts.zto;
	} else {
	    // Calculated range from autoscaling
	    xfrom = DBL_MAX;
	    xto = -DBL_MAX;
	    for (j = 0; j < n; j++) {
		if (opts.logbase && a[j] <= 0) continue;
		if (a[j] < xfrom) xfrom = a[j];
		if (a[j] > xto) xto = a[j];
	    }
	}
    }

    // Empty the histogram
    for (j = 0; j < bins; )
	h[j++] = 0;

    // Try to be foolproof -- return nonsense centers
    if (opts.logbase < 0 || (opts.logbase > 0 && (xfrom <= 0 || xto <= 0))) {
	if (x)
	    for (j = 0; j < bins; j++)
		x[j] = -100000 + j;
	return 1;
    }

    // Routine called with x == NULL => use xtmp instead
    if (!x) {
	xtmp = (double*)malloc(bins * sizeof(double));
	x = xtmp;
    }

    // Calculate right edges of bins -- there are only (bins-1) of them
    if (x_explicit) {
	// Calculate these edges from x_explicit
	for (j = bins_1-1; j >= 0; j--)
	    x[j] = (x_explicit[j] + x_explicit[j+1]) * 0.5;
    } else {
	// Calculate these edges according to xfrom, xto and logbase
	if (opts.logbase == 0) {
	    // Equidistant bin positions
	    xstep = (xto - xfrom) / bins_1;
	    for (j = bins_1; j >= 0; j--)
		x[j] = xfrom + (j + 0.5) * xstep;
	} else {
	    // Logarithmic bin positions
	    xstep = pow(opts.logbase,  log(xto/xfrom) / (log(opts.logbase) * bins_1));
	    x[0] = xfrom * sqrt(xstep);
	    for (j = 1; j <= bins_1; j++) {
		x[j] = x[j-1] * xstep;
	    }
	}
    }
#ifdef HIST_DEBUG
    printf("xstep=%g\txfrom=%g\txto=%g\n", (x_explicit ? 0 : xstep), xfrom, xto);
    for (j = 0; j < bins_1; j++)
	printf("\tedges  j=%i\tx[j]=%g\n", j, x[j]);
#endif


    /* Calculate the histogram -- go over all input values in array a.
       There are different algorithms to achieve this.
       Which algorithm to use? Equidistant 0, bisection 1.
    */
    int alg = (x_explicit || (opts.logbase != 0));

    if (alg == 0) {

    /* Algorithm for bins equidistant in x.
    */
    double coef = bins_1 / (xto - xfrom);
    for (j = 0; j < n; j++) {
	if (a[j] < x[0]) {
	    h[0]++;
	    continue;
	}
	if (a[j] >= x[bins_1-1]) {
	    h[bins_1]++;
	    continue;
	}
//	k = (int)((a[j] - xfrom) * coef + 0.5);
	k = (int)((a[j] - xfrom) * coef + 0.5);
#ifdef HIST_DEBUG
	printf("Alg equi: %i: a=%g\tk=%i\n", j, a[j], k);
#endif
	h[k]++;
    }


    } else { /* alg == 1 */


#if 0
    /* The most stupid algorithm, starts always from the first bin.
       Works for any ascending bin positions x.
       For debugging purposes only.
    */
    for (j = 0; j < n; j++) {
	for (k = 0; ; k++) {
	    if (k == bins_1) { // value belongs to the last bin
		h[bins_1]++;
		break;
	    }
	    if (a[j] < x[k]) {
		h[k]++;
		break;
	    }
	}
    }


#else


    /* The most powerful algorithm: bisection of intervals.
       Number of loops scales as log_2(bins)
    */
    for (j = 0; j < n; j++) {
	// eliminate boundary cases
	if (a[j] < x[0]) {
	    h[0]++;
	    continue;
	}
	if (a[j] >= x[bins_1]) {
	    h[bins_1]++;
	    continue;
	}
	// find k such that x[k-1] <= a[j] < x[k]
	int k_low = 0;
	int k_up = bins_1;
	while (1) {
	    k = (k_up + k_low) / 2;
#ifdef HIST_DEBUG
	    printf("Alg bisect: j=%i %g\tk=%i in %i %i \t: %g %g\n", j, a[j], k, k_low, k_up, x[k_low], x[k_up]);
#endif
	    if (k == k_low) break;
	    if (a[j] < x[k])
		k_up = k;
	    else
		k_low = k;
	}
	h[k_up]++;
    }


#endif // bisect algorithm


    } // alg == 1


    // Finally calculate centers of bins
    if (xtmp) // x was NULL => no interest in outputting bin centers
	free(xtmp);
    else {
	if (x_explicit) {
	    // bin centers passed in x_explicit
	    for (j = 0; j < bins; j++)
		x[j] = x_explicit[j];
	} else {
	    // bin centers from autoscaled or from opts.xfrom..opts.xto
	    x[0] = xfrom;
	    x[bins_1] = xto;
	    if (opts.logbase == 0) {
		for (j = 1; j < bins; j++)
		    x[j] = (opts.logbase) ? x[j-1] * xstep : xfrom + j * xstep;
	    }
	}
    }

    return 0; // success
}


bool run_histogram(double* a,  					// data
				int numPts,						// number of data points
				int nbins,						// number of bins in the histogram
				/* out */ double *x,			// histogram values
				/* out */ unsigned long *h,     // histogram counters
				double logbase)
{

	int n = numPts;

    //double *a = (double*)malloc(n * sizeof(double));
    t_pm_histogram opts;


    // Print generated numbers
#ifdef VERBOSE
    printf("List of generated data:\n");
    for (i=0; i<n; i++) {
	printf("\t%g", a[i]);
#if 1
	printf("%s", ((i % 5 == 4) ? "\n" : "\t"));
#else
	printf("\n");
#endif
    }
    printf("\nEnd of list of generated data.\n");
#endif // VERBOSE


    // Set histograming options
    opts.autoscale = 1;
    opts.zfrom = 0;
    opts.zto = n-1;
    opts.bins = nbins;
    opts.logbase = logbase;

    printf("*** Histogram properties:  from=%g  to=%g  bins=%i  logbase=%g\n", opts.zfrom, opts.zto, opts.bins, opts.logbase);
    // Run the histogram
    printf("*** Starting the histogram:\n");
    //double *x = (double*)malloc(opts.bins * sizeof(double));
    //unsigned long *h = (unsigned long*)malloc(opts.bins * sizeof(unsigned long));
#if 1
    int res = pm_histogram(x, h, a, n, opts);
#else
    for (i=0; i<opts.bins; i++) x[i] = i;
    	int res = pm_histogram(NULL, h, a, n, opts, x);
#endif
    if (res) {
		printf("Call to histogram failed with this reason: %i\n", res);
		exit(res);
    }

    // Print it to screen
#ifdef VERBOSE
    printf("*** The histogram:\n");
    for (i=0; i<opts.bins; i++) {
		if (opts.logbase)
			printf("\t%.3g\t%lu\n", x[i], h[i]);
		else
			printf("\t%.3f\t%lu\n", x[i], h[i]);
    }
#endif

    fprintf(stderr, "\nDone.\n");
    return true;
}


bool pm_histogram_nonuniform (double *x,
							  unsigned long *h,
							  int ubin_count,
							  int nonubin_count,
							  double *nonu_x,
							  unsigned long *nonu_h)
{
	int total_count = 0;
	for(int i=0; i<ubin_count;i++){
		total_count += h[i];
	}
	float avg_count = total_count/nonubin_count;
	int t_nonubin_count = nonubin_count;

	int step_bin_count = 0;
	int j = 0; // nonuniform hist counter
	unsigned long max_val_count = h[0];
	nonu_x[0] = x[0];
	for(int i=0; i<ubin_count; i++){
		step_bin_count += h[i];
		if(step_bin_count >= avg_count){
			nonu_h[j] = step_bin_count - h[i];
			t_nonubin_count--;
			total_count -= nonu_h[j];
			if(t_nonubin_count == 0 && total_count > 0){
				if(max_val_count < h[i]){
					max_val_count = h[i];
					nonu_x[j] = x[i];
				}
				break;
			}
			
			avg_count = total_count/t_nonubin_count;

			j++;
			step_bin_count = h[i];
			max_val_count = h[i];
			nonu_x[j] = x[i];
		}
		if(max_val_count < h[i]){
			max_val_count = h[i];
			nonu_x[j] = x[i];
		}
	}
	return true;
}

// eof pm_hist.h
