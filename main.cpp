#include <stdio.h>
#include <string>
#include "bignum.h"

static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
static CBigNum bnProofOfStakeLimit(~uint256(0) >> 24);
static CBigNum bnProofOfStakeHardLimit(~uint256(0) >> 30);

using namespace::std;


std::string padString(std::string in) {
	int targetLen = 64;
	int strLen = in.length();
	int padNum = targetLen - strLen;

	std:string outStr = "0x";
	for (int i = 0; i < padNum; i++) {
		outStr += "0";
	}
	outStr += in;

	return outStr;
}

void getTarget(int index, int64 pBlockTime, int64 ppBlockTime, unsigned int prevBits, int isPos) {

	CBigNum maxTarget = bnProofOfWorkLimit; // Only changes if: (nHeight + 1 > 14060) or (nHeight + 1 > 15000)

	// Reverse the algorithm
	bool fProofOfStake = false;
	if (isPos == 1) {
		fProofOfStake = true;
	}
	unsigned int nStakeMinAge = 60 * 60 * 24; // minimum age for coin age, changed to 8 hours (24 hr realistic min) network will auto balance
	unsigned int nStakeMaxAge = 60 * 60 * 24 * 90; // stake age of full weight
	unsigned int nStakeTargetSpacing = 1 * 30; // 1-minute block spacing    --- actually is 30 second
	static const int64 nTargetTimespan = 0.16 * 24 * 60 * 60;  // 0.16 of a day
	static const int64 nTargetSpacingWorkMax = 12 * nStakeTargetSpacing; // 12 minutes
	string type;
	if (fProofOfStake) {
		type = "pos";
	} else {
		type = "pow";
	}
	cout << "Get target algo (" << type << ") constants\n";
	cout << "nStakeMinAge: " << nStakeMinAge
			<< "\nnStakeMaxAge: " << nStakeMaxAge
			<< "\nnStakeTargetSpacing: " << nStakeTargetSpacing
			<< "\nnTargetTimespan: " << nTargetTimespan
			<< "\nnTargetSpacingWorkMax: " << nTargetSpacingWorkMax << "\n";

	int pBlockHeight = index;
	int ppBlockHeight = index;

	/*
	int64 pBlockTime = 1390010424;
	int64 ppBlockTime = 1390010337;
	unsigned int prevBits = 504365055;
	*/

	cout << "Inputs:\n";
	cout << "pBlockHeight: " << pBlockHeight
			<< "\nppBlockHeight: " << ppBlockHeight
			<< "\nprevBits: " << prevBits
			<< "\npBlockTime: " << pBlockTime
			<< "\nppBlockTime: " << ppBlockTime << "\n";

	cout << "Calculations:\n";
	int64 actualSpacing = pBlockTime - ppBlockTime;
	cout << "actualSpacing: " << actualSpacing << "\n";
	int64 targetSpacing = fProofOfStake ? nStakeTargetSpacing : min(nTargetSpacingWorkMax, (int64) nStakeTargetSpacing * (1 + pBlockHeight - ppBlockHeight));
	cout << "targetSpacing: " << targetSpacing << "\n";
	int64 interval = nTargetTimespan / targetSpacing;
	cout << "interval: " << interval << "\n";

	// Do the work to get the new target
	CBigNum bnNew;
	cout << "Setting bits in bignum\n";
	bnNew.SetCompact(prevBits);
	bnNew *= ((interval - 1) * targetSpacing + actualSpacing + actualSpacing);
	int64 rOp = ((interval - 1) * targetSpacing + actualSpacing + actualSpacing);
	cout << "Val after first op: " << bnNew.GetCompact() << " (" << padString(bnNew.GetHex()) << "). Right operand: " << rOp << "\n";
	bnNew /= ((interval + 1) * targetSpacing);
	rOp = ((interval + 1) * targetSpacing);
	if (bnNew > maxTarget) {
		cout << "New target (" << bnNew.GetCompact() << ") greater than max target! Setting to max target: " << maxTarget.GetCompact() << "\n";
		bnNew = maxTarget;
	}
	cout << "New target is: " << bnNew.GetCompact() << " (" << padString(bnNew.GetHex()) << "). Right operand: " << rOp << "\n";
}

int main(int argc, const char* argv[]) {

	cout << "Max targets:\n";
	cout << padString(bnProofOfWorkLimit.GetHex()) << "\n";
	cout << padString(bnProofOfStakeLimit.GetHex()) << "\n";
	cout << padString(bnProofOfStakeHardLimit.GetHex()) << "\n";

	CBigNum maxTarget = bnProofOfWorkLimit; // Only changes if: (nHeight + 1 > 14060) or (nHeight + 1 > 15000)

	cout << "Compacts:\n";

	cout << "Max target (" << padString(maxTarget.GetHex()) << ") as compact: " << maxTarget.GetCompact() << "\n";

	CBigNum newNum;
	newNum.SetCompact(maxTarget.GetCompact());
	cout << "GetCompact(" << newNum.GetCompact() << "): " << newNum.GetCompact() << "\n";

	string cmpHex = newNum.GetHex();
	cout << "GetCompact(" << newNum.GetCompact() << ") as hex: " << padString(cmpHex) << "\n";

	CBigNum hexNum;
	hexNum.SetHex(cmpHex);
	cout << "GetCompact() from hex: " << padString(cmpHex) << ": " << hexNum.GetCompact() << "\n";

	/*
	int blocks[8][5] = {
	   {1, 1390009623, 1389962782, 504365055, 0},
	   {2, 1390009701, 1390009623, 504365055, 0},
	   {3, 1390009747, 1390009701, 504365055, 0},
	   {4, 1390009868, 1390009747, 504365055, 0},
	   {5, 1390009984, 1390009868, 504365055, 0},
	   {6, 1390010147, 1390009984, 504365055, 0},
	   {7, 1390010322, 1390010147, 504365055, 0},
	   {8, 1390010337, 1390010322, 504365055, 0}
	};

	for (int i = 0; i < 8; i++) {
		cout << blocks[i][0] << " :: " << blocks[i][1] << " :: " << blocks[i][2] << " :: " << blocks[i][3] << " :: " << blocks[i][4] << "\n";
		getTarget(blocks[i][0], blocks[i][1], blocks[i][2], blocks[i][3], blocks[i][4]);
	}
	*/

	// Block 9
	getTarget(8, 1390010337, 1390010322, 504365055, 0);

}
