// fpGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include "..\..\source\Float128.h"
#include "..\..\source\rand.h"


int main()
{
	Float128 a, b, c, o, p;
	float af, bf, of;
	double ad, bd, od, pd;
	unsigned int *ap, *bp, *op;
	unsigned __int64 *ap64, *bp64, *op64;
	int nn;
	int addsub;
	RTFClasses::Random rnd;
	char rec[300];
	std::ofstream ofs("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpFMA128_tv.txt");
	std::ofstream ofsdiv("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpDiv_tvq.txt");
	std::ofstream ofsaddq("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpAddsub_tvq.txt");
	std::ofstream ofsadds("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpAddsub_tvs.txt");
	std::ofstream odsadds("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpAddsub_tvd.txt");
	std::ofstream o52sadds("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpAddsub_tv52.txt");
	std::ofstream p52muls("d:\\cores5\\Gambit\\v5\\software\\tools\\fpGen\\fpMul_tv52.txt");

	rnd.srand(1);
  std::cout << "Hello World!\n"; 
	for (nn = 0; nn < 24000; nn++) {
		a.sign = rnd.rand(2);
		a.exp = rnd.rand(65536) - 32767;
		a.man[0] = rnd.rand(0xffffffff);
		a.man[1] = rnd.rand(0xffffffff);
		a.man[2] = rnd.rand(0xffffffff);
		a.man[3] = rnd.rand(0xffffffff);
		b.sign = rnd.rand(2);
		b.exp = rnd.rand(65536) - 32767;
		b.man[0] = rnd.rand(0xffffffff);
		b.man[1] = rnd.rand(0xffffffff);
		b.man[2] = rnd.rand(0xffffffff);
		b.man[3] = rnd.rand(0xffffffff);
		c.sign = rnd.rand(2);
		c.exp = rnd.rand(65536) - 32767;
		c.man[0] = rnd.rand(0xffffffff);
		c.man[1] = rnd.rand(0xffffffff);
		c.man[2] = rnd.rand(0xffffffff);
		c.man[3] = rnd.rand(0xffffffff);
		Float128::Assign(&c, Float128::Zero());
		Float128::Mul(&p, &a, &b);
		Float128::Add(&o, &p, &c);
		strcpy_s(rec, 300, "0");
		strcat_s(rec, 300, o.ToHexString());
		strcat_s(rec, 300, c.ToHexString());
		strcat_s(rec, 300, b.ToHexString());
		strcat_s(rec, 300, a.ToHexString());
		strcat_s(rec, 300, "\n");
		ofs << rec;
		Float128::Div(&p, &a, &b);
		strcpy_s(rec, "");
		strcat_s(rec, 300, p.ToHexString());
		strcat_s(rec, 300, b.ToHexString());
		strcat_s(rec, 300, a.ToHexString());
		strcat_s(rec, 300, "\n");
		ofsdiv << rec;
		addsub = rnd.rand(2);
		if (addsub)
			Float128::Sub(&p, &a, &b);
		else
			Float128::Add(&p, &a, &b);
		strcpy_s(rec, "0");
		strcat_s(rec, addsub ? "1" : "0");
		strcat_s(rec, 300, p.ToHexString());
		strcat_s(rec, 300, b.ToHexString());
		strcat_s(rec, 300, a.ToHexString());
		strcat_s(rec, 300, "\n");
		ofsaddq << rec;
		addsub = rnd.rand(2);
		ap = (unsigned int *)&af;
		bp = (unsigned int *)&bf;
		op = (unsigned int *)&of;
		*ap = rnd.rand(0xffffffff);
		*bp = rnd.rand(0xffffffff);
		of = addsub ? af - bf : af + bf;
		strcpy_s(rec, "0");
		strcat_s(rec, addsub ? "1" : "0");
		sprintf_s(rec, 300, "%c0%08x%08x%08x\n", addsub ? '1' : '0', *op, *bp, *ap);
		ofsadds << rec;
		ap64 = (unsigned __int64 *)&ad;
		bp64 = (unsigned __int64 *)&bd;
		*ap64 = ((unsigned __int64)rnd.rand(0xffffffff) << 32L) | (unsigned __int64)rnd.rand(0xffffffff);
		*bp64 = ((unsigned __int64)rnd.rand(0xffffffff) << 32L) | (unsigned __int64)rnd.rand(0xffffffff);
		op64 = (unsigned __int64 *)&od;
		od = addsub ? ad - bd : ad + bd;
		pd = ad * bd;
		ap = (unsigned int *)ap64;
		bp = (unsigned int *)bp64;
		op = (unsigned int *)op64;
		strcpy_s(rec, "0");
		strcat_s(rec, addsub ? "1" : "0");
		sprintf_s(rec, 300, "%c0%08x%08x%08x%08x%08x%08x\n", addsub ? '1' : '0', op[1], op[0], bp[1],bp[0], ap[1],ap[0]);
		odsadds << rec;
		sprintf_s(rec, 300, "%c0%08x%05x%08x%05x%08x%05x\n", addsub ? '1' : '0', op[1], (op[0] >> 12) & 0xfffff, bp[1], (bp[0] >> 12) & 0xfffff, ap[1], (ap[0] >> 12) & 0xfffff);
		o52sadds << rec;
		op64 = (unsigned __int64 *)&pd;
		op = (unsigned int *)op64;
		sprintf_s(rec, 300, "%08x%05x%08x%05x%08x%05x\n", op[1], (op[0] >> 12) & 0xfffff, bp[1], (bp[0] >> 12) & 0xfffff, ap[1], (ap[0] >> 12) & 0xfffff);
		p52muls << rec;
	}
	p52muls.close();
	odsadds.close();
	ofsadds.close();
	ofsaddq.close();
	ofsdiv.close();
	ofs.close();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
