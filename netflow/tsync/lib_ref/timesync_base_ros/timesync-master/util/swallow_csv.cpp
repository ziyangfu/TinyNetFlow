//
// Efficient CSV Reader for MATLAB
//
// Copyright (C) 2012, Stanislaw Adaszewski
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <mex.h>
#include <vector>
#include <string>

using namespace std;

static double zero = 0.0;
static double NaN = zero / 0;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1 || nrhs > 4 || nlhs != 2) {
        mexErrMsgTxt("Usage: [numbers, text] = swallow_csv(filename, [quote, [sep, [escape]]])");
    }
    
    for (int i = 0; i < nrhs; i++) {
        if (!mxIsChar(prhs[i]) || mxIsEmpty(prhs[i])) {
            mexErrMsgTxt("Usage: [numbers, text] = swallow_csv(filename, [quote, [sep, [escape]]])");
        }
    }
    
    char quote = '"';
    char sep = ';';
    char escape = '\\';
    
    if (nrhs > 1) {
        quote = mxGetChars(prhs[1])[0] & 0xff;
        if (nrhs > 2) {
            sep = mxGetChars(prhs[2])[0] & 0xff;
            if (nrhs > 3) {
                escape = mxGetChars(prhs[3])[0] & 0xff;
            }
        }
    }
    
    char filename[1024];
    mxGetString(prhs[0], filename, 1024);
    FILE *f = fopen(filename, "rb");
    if (!f) {
        mexErrMsgTxt("Couldn't open specified file");
    }
    char buf[4096];
    int state = 0;
    vector<vector<string> > data;
    string cell;
    vector<string> row;
    size_t max_columns = 0;
    
    while (true) {
        size_t n = fread(buf, 1, 4096, f);
        if (n == 0) {
            break;
        }
        for (size_t i = 0; i < n; i++) {
            switch(state) {
                case 0: // virgin state
                    if (buf[i] == quote) {
                        state = 1; // quoted string
                    } else if (buf[i] == sep) {
                        row.push_back(cell);
                        cell = "";
                    } else if (buf[i] == '\r') {
                        // do nothing
                    } else if (buf[i] == '\n') {
                        row.push_back(cell);
                        cell = "";
                        if (row.size() > max_columns) {
                            max_columns = row.size();
                        }
                        data.push_back(row);
                        row.clear();
                    } else {
                        cell += buf[i];
                    }
                    break;
                case 1: // quoted string
                    if (buf[i] == escape) {
                        state = 2; // escaped state
                    } else if (buf[i] == quote) {
                        state = 3; // potential double quote
                    } else {
                        cell += buf[i];
                    }
                    break;
                case 2: // escaped
                    if (buf[i] == 'n') {
                        cell += '\n';
                    } else if (buf[i] == 't') {
                        cell += '\t';
                    } else if (buf[i] == 'r') {
                        cell += '\r';
                    } else if (buf[i] == quote) {
                        cell += quote;
                    } else {
                        mexErrMsgTxt("Encountered unknown escape sequence.");
                    }
                    state = 1;
                    break;
                case 3: // potential double quote
                    if (buf[i] == quote) {
                        cell += quote;
                        state = 1; // quoted string continues
                    } else {
                        state = 0; // parse it in the regular way
                        i--;
                    }
                    break;
            }
        }
    }
    
    fclose(f);
    
    mwSize dims[] = {(mwSize) data.size(), (mwSize) max_columns};
    plhs[0] = mxCreateNumericArray(2, dims, mxDOUBLE_CLASS, mxREAL);
    plhs[1] = mxCreateCellArray(2, dims);
    
    double *p = mxGetPr(plhs[0]);
    
    for (int i = 0; i < data.size(); i++) {
        vector<string> &row(data[i]);
        for (int j = 0; j < row.size(); j++) {
            char *nptr = (char*) row[j].c_str();
            char *endptr;
            double d = strtod(nptr, &endptr);
            if (row[j].size() == 0 || endptr != nptr + row[j].size()) {
                p[j * data.size() + i] = NaN;
                mxArray *tmp = mxCreateString(nptr);
                mxSetCell(plhs[1], (int)(j * data.size() + i), tmp);
            } else {
                p[j * data.size() + i] = d;
            }
        }
		for (int j = row.size(); j < max_columns; j++) {
			p[j * data.size() + i] = NaN;
		}
    }
}