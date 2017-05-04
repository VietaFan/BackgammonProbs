#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "utils.h"
#include "conversion.h"
#define MAX_ROLLS 30
using namespace std;
int recPlayInnerDoubles(int *a, int d, int n, float *expval) {
	int current, ans;
	float best = 1e10;
	if (n == 0)
		return convert(a);
	if (a[d] > 0) {
		--a[d];
		++a[0];
		ans = recPlayInnerDoubles(a, d, n-1, expval);
		--a[0];
		++a[d];
		return ans;
	}
	bool high = false;
	for (int i=6; i>d; --i) {
		if (a[i]) {
			high = true;
			--a[i];
			++a[i-d];
			current = recPlayInnerDoubles(a, d, n-1, expval);
			--a[i-d];
			++a[i];
			if (expval[current] < best) {
				best = expval[current];
				ans = current;
			}
		}
	}
	if (!high) {
		ans = 0;
		for (int i=d-1; i>0; --i) {
			if (a[i] > 0) {
				if (a[i] >= n) {
					a[i] -= n;
					a[0] += n;
					ans = convert(a);
					a[i] += n;
					a[0] -= n;
				} else {
					int temp = a[i];
					a[i] = 0;
					a[0] += temp;
					ans = recPlayInnerDoubles(a, d, n-temp, expval);
					a[0] -= temp;
					a[i] = temp;
				}
				break;
			}
		}
	}
	return ans;
}			
	
int recPlayOuterDoubles(int *a, int d, int n, float *expval) {
	if (n == 0)
		return convert(a);	
	if (convert(a) < choose_dp[21][6]) // if it's actually an inner board-only state
		return recPlayInnerDoubles(a, d, n, expval);
	int conv, best_conv;
	float best = 1e10;
	// try all possible ways to play one without bearing off
	// at least one must exist since it's an outer board state
	for (int i=12; i>d; --i) {
		if (!a[i]) continue;
		--a[i];
		++a[i-d];
		conv = recPlayOuterDoubles(a, d, n-1, expval);
		if (expval[conv] < best) {
			best = expval[conv];
			best_conv = conv;
		}
		--a[i-d];
		++a[i];
	}
	return best_conv;
}
	
			
void calcInnerProbs(float *expval, float *ctprobs[]) {
	int inner = choose_dp[21][6], next_pos;
	int a[13];
	expval[0] = 0;
	ctprobs[0][0] = 1;
	bool opp1, opp2;
	int temp, conv, best_conv;
	float current, best;
	// for each board position
	for (int pos=1; pos<inner; ++pos) {
		unconvert(pos, a);
		expval[pos] = 1.0; // current roll, we'll add more later
		//  try all possible rolls. first, non-doubles
		for (int d1=1; d1<7; ++d1) {
			for (int d2=1; d2<d1; ++d2) {
				best = 1e10;
				for (int dir=0; dir<2; ++dir) {
					// if this is true, then we get to bear off the highest checker with d1
					opp1 = false;
					// try first die moves
					for (int p1=d1; p1<7; ++p1) {
						if (a[p1] > 0) {
							opp1 = true;
							--a[p1];
							++a[p1-d1];			
							// we might only need to move one die
							if (a[0] == 15) {
								best = 0.0;
								best_conv = 0;
								++a[p1];
								--a[p1-d1];	
								continue;
							}
							opp2 = false;
							// try second die moves
							for (int p2=d2; p2<7; ++p2) {
								if (a[p2] > 0) {
									opp2 = true;
									--a[p2];
									++a[p2-d2];
									conv = convert(a);
									current = expval[conv];
									if (current < best) {
										best = current;
										best_conv = conv;
									}
									--a[p2-d2];
									++a[p2];
								}
							}
							// all checkers on spaces < d2
							if (!opp2) {
								int p2;
								for (p2 = d2-1; p2 > 0 && a[p2] == 0; --p2);
								if (p2 > 0) {
									--a[p2];
									++a[0];
									conv = convert(a);
									current = expval[conv];
									if (current < best) {
										best = current;
										best_conv = conv;
									}
									--a[0];
									++a[p2];
								}
							}
							++a[p1];
							--a[p1-d1];
						}
					}
					// we have to bear off a checker at position < d1
					if (!opp1) {
						int p1;
						for (p1 = d1-1; p1 > 0 && a[p1] == 0; --p1);
						if (p1 > 0) {
							--a[p1];
							++a[0];						
							// we might only need to move one die
							if (a[0] == 15) {
								best = 0.0;
								best_conv = 0;
								++a[p1];
								--a[0];	
								temp = d1;
								d1 = d2;
								d2 = temp;	
								continue;
							}
							opp2 = false;
							// try second die moves
							for (int p2=d2; p2<7; ++p2) {
								if (a[p2]) {
									opp2 = true;
									--a[p2];
									++a[p2-d2];
									conv = convert(a);
									current = expval[conv];
									if (current < best) {
										best = current;
										best_conv = conv;
									}
									--a[p2-d2];
									++a[p2];
								}
							}
							// all checkers on spaces < d2
							if (!opp2) {
								int p2;
								for (p2 = d2-1; p2 > 0 && a[p2] == 0; --p2);
								if (p2 > 0) {
									--a[p2];
									++a[0];
									conv = convert(a);
									current = expval[conv];
									if (current < best) {
										best = current;
										best_conv = conv;
									}
									--a[0];
									++a[p2];
								}
							}
							++a[p1];
							--a[0];
						}
					}					
					temp = d1;
					d1 = d2;
					d2 = temp;	
				}
				// update expected value and roll count probabilities
				expval[pos] += expval[best_conv]/18;
				for (int i=0; i<MAX_ROLLS; ++i) 
					ctprobs[pos][i+1] += ctprobs[best_conv][i]/18;
			}
		}
		// the doubles case
		for (int d=1; d<7; ++d) {
			// play the doubles in the best way possible
			conv = recPlayInnerDoubles(a, d, 4, expval); 
			// update expected value and roll count probabilities
			expval[pos] += expval[conv]/36;
			for (int i=0; i<MAX_ROLLS; ++i) 
				ctprobs[pos][i+1] += ctprobs[conv][i]/36;
		}
	}
}

// assumes all inner board probabilities have already been calculated
void calcOuterProbs(float *expval, float **ctprobs, int highBound=-1) {
	int inner = choose_dp[21][6], outer = highBound;
	if (highBound < 0) 
		outer = choose_dp[27][12];
	int temp, conv, best_conv;
	float current, best;
	int a[13];
	for (int pos=inner; pos<outer; ++pos) {
		if (pos%25000 == 0) {
			cout << "Analyzing board state #" << pos << endl;
		}
		unconvert(pos, a);
		expval[pos] = 1.0; // current roll, we'll add more later
		//  try all possible rolls. first, non-doubles
		for (int d1=1; d1<7; ++d1) {
			for (int d2=1; d2<d1; ++d2) {
				best = 1e10;
				for (int dir=0; dir<2; ++dir) {
					for (int p1=d1+1; p1<13; ++p1) {
						if (!a[p1]) continue;
						--a[p1];
						++a[p1-d1];
						if (convert(a) < inner) {
							// all checkers are now in inner board
							// get best play of the second die, considering that we can now bear off with it
							conv = recPlayInnerDoubles(a, d2, 1, expval);
							if (expval[conv] < best) {
								best = expval[conv];
								best_conv = conv;
							}
						} else {
							// play second checker in outer board
							for (int p2=d2+1; p2<13; ++p2) {
								if (!a[p2]) continue;
								// try the move, record it if it's the best so far
								--a[p2];
								++a[p2-d2];
								conv = convert(a);
								if (expval[conv] < best) {
									best = expval[conv];
									best_conv = conv;
								}
								--a[p2-d2];
								++a[p2];
							}
						}
						--a[p1-d1];
						++a[p1];
					}
					temp = d1;
					d1 = d2;
					d2 = temp;	
				}
				// add best move to our expected value and probabilities
				expval[pos] += expval[conv]/18;
				for (int i=0; i<MAX_ROLLS; ++i) 
					ctprobs[pos][i+1] += ctprobs[conv][i]/18;
			}
		}
		// now try the doubles
		for (int d=1; d<7; ++d) {
			// play the doubles in the best way possible
			conv = recPlayOuterDoubles(a, d, 4, expval); 
			// update expected value and roll count probabilities
			expval[pos] += expval[conv]/36;
			for (int i=0; i<MAX_ROLLS; ++i) 
				ctprobs[pos][i+1] += ctprobs[conv][i]/36;
		}
	}
}
void outputStatePDFs(ostream &out, float **pdfs, int npdfs) {
	char buf[64];
	int ctr1, ctr2, k, x;
	float *p;
	out.write((char*)(&npdfs), 4);
	for (int pos=0; pos<npdfs; ++pos) {
		if (pos%20000 == 0)
			cout << "outputting PDF #" << pos << endl;
		for (int i=0; i<64; ++i)
			buf[i] = 0;
		p = pdfs[pos];
		x = 3;
		for (ctr1 = 0; p[ctr1]+0.0000005 < 0.000001; ++ctr1);
		buf[0] = ctr1;
		for (ctr2 = ctr1; p[ctr2]+0.0000005 < 0.000256; ++ctr2) {
			k = (p[ctr2]+0.0000005)*1000000;
			buf[++x] = k;
		}
		buf[1] = (ctr2-ctr1)<<4;
		for (ctr1 = ctr2; p[ctr1]+0.0000005 < 0.065536; ++ctr1) {
			k = (p[ctr1]+0.0000005)*1000000;
			buf[++x] = (k>>8);
			buf[++x] = k;
		}
		buf[1] |= ((ctr1-ctr2)&0xf);
		for (ctr2 = ctr1; p[ctr2]+0.0000005 >= 0.065536; ++ctr2) {
			k = (p[ctr2]+0.0000005)*1000000;
			buf[++x] = (k>>16);
			buf[++x] = (k>>8);
			buf[++x] = k;
		}
		buf[2] = (ctr2-ctr1)<<4;
		for (ctr1 = ctr2; p[ctr1]+0.0000005 >= 0.000256; ++ctr1) {
			k = (p[ctr1]+0.0000005)*1000000;
			buf[++x] = (k>>8);
			buf[++x] = k;
		}
		buf[2] |= ((ctr1-ctr2)&0xf);
		for (ctr2 = ctr1; p[ctr2]+0.0000005 >= 0.000001; ++ctr2) {
			k = (p[ctr2]+0.0000005)*1000000;
			buf[++x] = k;
		}			
		buf[3] = (ctr2-ctr1)<<4;
		out.write(buf, 64);
	}
}
int main() {
	clock_t t = clock();
	init_choosedp(30);
	init_combindiffs();
	int bound = choose_dp[22][7];//choose_dp[27][12];//17383860 > 54264
	float *expval = new float[bound];
	float *ctprobs = new float[bound*(2+MAX_ROLLS)];
	float **ctprobptrs = new float*[bound];
	for (int i=0; i<bound; ++i)
		ctprobptrs[i] = ctprobs+(2+MAX_ROLLS)*i;
	cout << "analyzing all inner board possibilities...\n";
	calcInnerProbs(expval, ctprobptrs);
	cout << "done\n";
	cout << "time = " << clock()-t << " ms\n";
	t = clock();
	cout << "analyzing outer board possibilities up to game state #" << bound << "...\n";
	calcOuterProbs(expval, ctprobptrs, bound);
	cout << "done\n";
	cout << "time = " << clock()-t << " ms\n";
	t = clock();
	cout << "outputting data to file\n";
	ofstream fout("bkgmdata.txt", ios::out | ios::binary);
	outputStatePDFs(fout, ctprobptrs, bound);
	fout.close();
	cout << "time = " << clock()-t << " ms\n";
	string s;
	int a[13];
	for (int i=0; i<13; i++)
		a[i] = 0;
	int conv, k;
	while (true) {
		cout << "Enter a checker distribution.\n";
		cin >> a[1] >> a[2] >> a[3] >> a[4] >> a[5] >> a[6] >> a[7];
		if (a[1] < 0)
			break;
		a[0] = 15-a[1]-a[2]-a[3]-a[4]-a[5]-a[6]-a[7];
		conv = convert(a);
		cout << "expected number of rolls: " << fixed << setprecision(6) << expval[conv] << endl;
		k = 0;
		while (ctprobptrs[conv][k] < 0.000001) 
			++k;
		while (ctprobptrs[conv][k] > 0.000001) {
			cout << "P(# rolls = " << k << ") = " << fixed << setprecision(6) << ctprobptrs[conv][k] << endl;
			++k;
		}
	}
	/*int pips, checkers, pct;
	float exp_val;
	float probs[32];
	int div;
	while (true) {
		cout << "How many pips left?";
		cin >> pips;
		cout << "How many checkers left?";
		cin >> checkers;
		div = 0;
		exp_val = 0;
		for (int i=0; i<MAX_ROLLS+2; ++i)
			probs[i] = 0;
		for (int i=0; i<54264; ++i) {
			unconvert(i, a);
			if (a[0] != 15-checkers)
				continue;
			pct = 0;
			for (int i=1; i<7; ++i)
				pct += i*a[i];
			if (pct != pips) continue;
			++div;
			exp_val += expval[i];
			for (int j=0; j<MAX_ROLLS; ++j) {
				probs[j] += ctprobptrs[i][j];
			}
		}
		exp_val /= div;
		for (int j=0; j<MAX_ROLLS; ++j)
			probs[j] /= div;
		cout << "expected number of rolls: " << fixed << setprecision(6) << exp_val << endl;
		k = 0;
		while (probs[k] < 0.000001) 
			++k;
		while (probs[k] > 0.000001) {
			cout << "P(# rolls = " << k << ") = " << fixed << setprecision(6) << probs[k] << endl;
			++k;
		}
	}*/
	delete[] expval;
	delete[] ctprobs;
	delete[] ctprobptrs;
	/*for (int i=0; i<54624; ++i)
		delete[] ctprobs[i];*/
	//delete[] ctprobs;
}
