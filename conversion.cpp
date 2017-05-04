#include "conversion.h"
#include "utils.h"
// convert backgammon half-position into unique integer from 0 to 27C12 - 1
// a is sequence of checkers on pips (including off), m is number of spaces, n is number of checkers
int convert(int *a, int m, int n) {
	int ans = 0;
	while (m > 0) {
		ans += combin_diffs[n][m][a[m]]; // this equals (choose_dp[n+m][m]-choose_dp[n+m-a[m]][m]);
		n -= a[m];
		--m;
	}
	return ans;
}
// convert integer from 0 to 27C12 - 1 to backgammon position
void unconvert(int k, int *a, int m, int n) {
	int x,i;
	while (m > 0) {
		x = choose_dp[n+m][m]-k;
		i = 1;
		while (choose_dp[n+m-i][m] >= x)
			++i;
		a[m] = i-1;
		k -= combin_diffs[n][m][i-1];// this equals (choose_dp[n+m][m]-choose_dp[n+m-i+1][m]);
		n -= a[m];
		--m;
	}
	a[0] = n;
}

