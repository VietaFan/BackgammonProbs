#ifndef UTILS_H
#define UTILS_H
extern int choose_dp[50][50];
extern int combin_diffs[20][20][20];
void init_choosedp(int maxn);
void init_combindiffs(int mmax=12, int nmax=15);
#endif
