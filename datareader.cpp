#include <fstream>
#include <iostream>
#include <iomanip>
#include "conversion.h"
#include "utils.h"
#define MAX_ROLLS 30
using namespace std;

void readInputData(ifstream &fin, char *arr) {
	fin.read(arr, 4);
	int nlines = *((int*)arr);
	for (int i=0; i<nlines; ++i) {
		fin.read(arr+64*i, 64);
		/*for (int j=0; j<16; ++j) {
			cout << hex << (((int)arr[64*i+j])&0xff) << ' ';
		}
		cout << endl;*/
	}
	/*char buf[4];
	in.read(buf, 4);
	int nlines = *((int*)buf);
	cout << nlines << endl;
	for (int i=0; i<nlines; ++i) {
		in.read(arr+64*i, 64);
		if (i == 123) {
	for (int i=0; i<64; ++i)
		cout << hex << (((int)arr[i])&0xff) << ' ';
	cout << endl;
}
	}
	cout << endl;*/
}

void getdist(int conv, char *arr, float *dist) {
	cout << conv << endl;
	for (int i=64*conv; i<64*(conv+1); ++i) {
		cout << hex << ((int)(arr[i])&0xff) << ',';
	}
	cout << endl;
	char* buf = arr+64*conv;
	int k = 0;
	unsigned int a, b, c, d, e;
	a = buf[1] >> 4;
	b = buf[1] & 0xf;
	c = buf[2] >> 4;
	d = buf[2] & 0xf;
	e = buf[3] >> 4;
	for (int i=0; i<buf[0]; ++i)
		dist[k++] = 0.0;
	int x=3;
	for (int i=0; i<a; ++i)
		dist[k++] = 0.000001*(0xff&buf[++x]);
	for (int i=0; i<b; ++i)
		dist[k++] = 0.000256*(0xff&buf[++x])+0.000001*(0xff&buf[++x]);
	for (int i=0; i<c; ++i)
		dist[k++] = 0.065536*(0xff&buf[++x])+0.000256*(0xff&buf[++x])+0.000001*(0xff&buf[++x]);
	for (int i=0; i<d; ++i)
		dist[k++] = 0.000256*(0xff&buf[++x])+0.000001*(0xff&buf[++x]);
	for (int i=0; i<e; ++i)
		dist[k++] = 0.000001*(0xff&buf[++x]);
	for (; k<MAX_ROLLS; ++k)
		dist[k] = 0;
}
		
int main() {
	ifstream fin("bkgmdata.txt", ios::in | ios::binary);
	init_choosedp(30);
	init_combindiffs();
/*	char arr[16];
	fin.read(arr, 4);
	for (int i=0; i<50; ++i) {
		fin.read(arr, 16);
		for (int j=0; j<16; ++j) {
			cout << hex << (((int)arr[j])&0xff) << ' ';
		}
		cout << endl;
	}*/
	char* arr = new char[20000000];
	readInputData(fin, arr);
	fin.close();
	//char buf[64];
	//f/in.read(buf, 4);
	//int nlines = *((int*)buf);
	//cout << "number of lines = " << nlines << endl;
	//fin.read(arr, 64);
//	fin.close();
	string s;
	int a[13];
	for (int i=0; i<13; i++)
		a[i] = 0;
	int conv, k;
	float dist[32];
	while (true) {
		cout << "Enter a checker distribution.\n";
		cin >> a[1] >> a[2] >> a[3] >> a[4] >> a[5] >> a[6] >> a[7];
		if (a[1] < 0)
			break;
		a[0] = 15-a[1]-a[2]-a[3]-a[4]-a[5]-a[6]-a[7];
		conv = convert(a);
		getdist(conv, arr, dist);
	//	for (int i=0; i<30; ++i)
	//		cout << dist[i] << ' ';
	//	cout << endl;
		k = 0;
		while (dist[k] < 0.000001) 
			++k;
		while (dist[k] > 0.000001) {
			cout << "P(# rolls = " << k << ") = " << fixed << setprecision(6) << dist[k] << endl;
			++k;
		}
	}
	delete[] arr;
	return 0;
}
