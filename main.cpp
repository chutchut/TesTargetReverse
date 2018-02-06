#include <stdio.h>
#include <string>
#include "bignum.h"
#include <array>

static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
static CBigNum bnProofOfStakeLimit(~uint256(0) >> 24);
static CBigNum bnProofOfStakeHardLimit(~uint256(0) >> 30);

using namespace::std;


struct Block {
    int array[5];
};

string padString(string in) {
	int targetLen = 64;
	int strLen = in.length();
	int padNum = targetLen - strLen;

	string outStr = "0x";
	for (int i = 0; i < padNum; i++) {
		outStr += "0";
	}
	outStr += in;

	return outStr;
}

Block getBlock(int index, int blocks[][5]) {
	for (int i = 0; i < sizeof(blocks); i++) {
		if (blocks[i][0] == index) {
			Block b;
			b.array[0] = blocks[i][0];
			b.array[1] = blocks[i][1];
			b.array[2] = blocks[i][2];
			b.array[3] = blocks[i][3];
			b.array[4] = blocks[i][4];
			return b;
		}
	}
	// Null block
	Block nb;
	return nb;
}

const int GetLastBlockIndex(int index, int blocks[][5], bool fProofOfStake) {
	int pindex = index;
	Block b = getBlock(pindex, blocks);
    while (pindex > 0 && (pindex - 1) > 0 && ((b.array[4] == 1) != fProofOfStake))
        pindex -= 1;
    	b = getBlock(pindex, blocks);
    return pindex;
}

void getTarget(int index, int blocks[][5]) {

	// Get the block for the current index
	Block block = getBlock(index, blocks);

	// Previous bits for current block
	unsigned int prevBits = block.array[3];

	// Set pos/pow and target based on type
	bool fProofOfStake = false;
	CBigNum maxTarget = bnProofOfWorkLimit;
	if (block.array[4] == 1) {
		fProofOfStake = true;
		if(index + 1 > 15000)
			maxTarget = bnProofOfStakeLimit;
		else if(index + 1 > 14060)
			maxTarget = bnProofOfStakeHardLimit;
	}

	// Get indexes with GetLastBlockIndex()
	int pBlockHeight = GetLastBlockIndex(index, blocks, fProofOfStake);
	int ppBlockHeight = GetLastBlockIndex(pBlockHeight - 1, blocks, fProofOfStake);

	// Get blocks for prev and prev-prev indexes
	Block pblock = getBlock(pBlockHeight, blocks);
	Block ppblock = getBlock(ppBlockHeight, blocks);

	// Set timestamps from above blocks
	int64 pBlockTime = pblock.array[1];
	int64 ppBlockTime = ppblock.array[1];

	// Reverse the algorithm
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

	cout << "Inputs:\n";
	cout << "pBlockHeight: " << pBlockHeight
			<< "\nppBlockHeight: " << ppBlockHeight
			<< "\nprevBits: " << prevBits
			<< "\npBlockTime: " << pBlockTime
			<< "\nppBlockTime: " << ppBlockTime << "\n";

	cout << "Calculations:\n";
	int64 actualSpacing = pBlockTime - ppBlockTime;
	cout << "actualSpacing: " << actualSpacing << "\n";
	int64 targetSpacing = fProofOfStake ? nStakeTargetSpacing : min(nTargetSpacingWorkMax, (int64) nStakeTargetSpacing * (1 + index - pBlockHeight));
	cout << "targetSpacing: " << targetSpacing << "\n";
	int64 interval = nTargetTimespan / targetSpacing;
	cout << "interval: " << interval << "\n";

	// Do the work to get the new target
	CBigNum bnNew;
	cout << "Setting bits in bignum\n";
	bnNew.SetCompact(prevBits);

	// Right op 1
	int64 rOp = ((interval - 1) * targetSpacing + actualSpacing + actualSpacing);
	bnNew *= rOp;
	cout << "Val after first op: " << bnNew.GetCompact() << " (" << padString(bnNew.GetHex()) << "). Right operand: " << rOp << "\n";

	// Right op 2
	rOp = ((interval + 1) * targetSpacing);
	bnNew /= rOp;

	// New target
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

	int blocks1to8[8][5] = {
	   {1, 1390009623, 1389962782, 504365055, 0},
	   {2, 1390009701, 1390009623, 504365055, 0},
	   {3, 1390009747, 1390009701, 504365055, 0},
	   {4, 1390009868, 1390009747, 504365055, 0},
	   {5, 1390009984, 1390009868, 504365055, 0},
	   {6, 1390010147, 1390009984, 504365055, 0},
	   {7, 1390010322, 1390010147, 504365055, 0},
	   {8, 1390010337, 1390010322, 504365055, 0}
	};

	for (int i = 0, j = 1; i < 8; i++, j++) {
		cout << blocks1to8[i][0] << " :: " << blocks1to8[i][1] << " :: " << blocks1to8[i][2] << " :: " << blocks1to8[i][3] << " :: " << blocks1to8[i][4] << "\n";
		getTarget(j, blocks1to8);
	}

	// Block 9
	//getTarget(8, 1390010337, 1390010322, 504365055, 0);

	// Block 88535
	//getTarget(88535, 1392636015, 1392635990, 486651519, 1);

}
