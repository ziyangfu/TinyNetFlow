#include "mex.h"
#include "../Mediator.h"

/* The computational routine */
void arrayMedian(int windowsize, double *y, double *z, mwSize n)
{
    mwSize i;
    Mediator<double> mediator(windowsize);
    for (i=0; i<n; i++) {
        mediator.insert(y[i]);
        z[i] = mediator.getMedian();
    }
}

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
    int windowsize;              /* input scalar */
    double *inMatrix;               /* 1xN input matrix */
    size_t nrows;                   /* size of matrix */
    double *outMatrix;              /* output matrix */

    /* check for proper number of arguments */
    if(nrhs!=2) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs","Two inputs required.");
    }
    /*if(nlhs!=1) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs","One output required.");
    }*/
    /* make sure the first input argument is scalar */
    if( !mxIsDouble(prhs[0]) || 
         mxIsComplex(prhs[0]) ||
         mxGetNumberOfElements(prhs[0])!=1 ) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notScalar","Input multiplier must be a scalar.");
    }
    
    /* make sure the second input argument is type double */
    if( !mxIsDouble(prhs[1]) || 
         mxIsComplex(prhs[1])) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notDouble","Input matrix must be type double.");
    }
    
    /* check that number of rows in second input argument is 1 */
    if(mxGetN(prhs[1])!=1) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:notColVector","Input must be a column vector.");
    }
    
    /* get the value of the scalar input  */
    windowsize = (int) mxGetScalar(prhs[0]);

    /* create a pointer to the real data in the input matrix  */
    inMatrix = mxGetPr(prhs[1]);

    /* get dimensions of the input matrix */
    nrows = mxGetM(prhs[1]);

    /* create the output matrix */
    plhs[0] = mxCreateDoubleMatrix((mwSize)nrows,1,mxREAL);

    /* get a pointer to the real data in the output matrix */
    outMatrix = mxGetPr(plhs[0]);

    /* call the computational routine */
    arrayMedian(windowsize,inMatrix,outMatrix,(mwSize)nrows);
}
