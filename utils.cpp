#include "utils.h"
int choose_dp[50][50];
int combin_diffs[20][20][20];
void init_choosedp(int maxn) {
	for (int i=0; i<maxn; ++i) {
		choose_dp[0][i] = 0;
		choose_dp[i][0] = 1;
	}
	for (int i=1; i<maxn; ++i) {
		for (int j=1; j<=i; ++j)
			choose_dp[i][j] = choose_dp[i-1][j]+choose_dp[i-1][j-1];
		choose_dp[i][i+1] = 0;
	}
}
void init_combindiffs(int mmax, int nmax) {
	for (int n=0; n<=nmax; ++n)
		for (int m=0; m<=mmax; ++m)
			for (int a=0; a<=n; ++a)
				combin_diffs[n][m][a] = choose_dp[n+m][m]-choose_dp[n+m-a][m];
}
