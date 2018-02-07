#include <stdio.h>
#include <string>
#include "bignum.h"
#include <array>
#include <vector>

static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
static CBigNum bnProofOfStakeLimit(~uint256(0) >> 24);
static CBigNum bnProofOfStakeHardLimit(~uint256(0) >> 30);

using namespace::std;


struct Block {
    int array[4];
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

Block getBlock(int index, int blocks[][4]) {
	for (int i = 0; i < sizeof(blocks); i++) {
		if (blocks[i][0] == index) {
			Block b;
			b.array[0] = blocks[i][0]; // Block index
			b.array[1] = blocks[i][1]; // Timestamp
			b.array[2] = blocks[i][2]; // Previous 'bits' value
			b.array[3] = blocks[i][3]; // pos/pow flag (1 == pos)
			return b;
		}
	}
	// Null block
	Block nb;
	return nb;
}

const int GetLastBlockIndex(int index, int blocks[][4], bool fProofOfStake) {
	int pindex = index;
	Block b = getBlock(pindex, blocks);
    while (pindex > 0 && (pindex - 1) > 0 && ((b.array[3] == 1) != fProofOfStake))
        pindex -= 1;
    	b = getBlock(pindex, blocks);
    return pindex;
}

bool isNullBlock(Block block) {
	if (block.array[0] == 0 && block.array[1] == 0 && block.array[2] == 0 && block.array[3] == 0) {
		return true;
	} else {
		return false;
	}
}

void getTarget(int index, int blocks[][4]) {

	// Get the block for the current index
	Block block = getBlock(index, blocks);

	if (isNullBlock(block)) {
		// Ignore null blocks
		return;
	}

	// Previous bits for current block
	unsigned int prevBits = block.array[2];

	// Set pos/pow and target based on type
	bool fProofOfStake = false;
	CBigNum maxTarget = bnProofOfWorkLimit;
	if (block.array[3] == 1) {
		fProofOfStake = true;
		if(index + 1 > 15000)
			maxTarget = bnProofOfStakeLimit;
		else if(index + 1 > 14060)
			maxTarget = bnProofOfStakeHardLimit;
	}

	// Get indexes with GetLastBlockIndex()
	int pBlockHeight = GetLastBlockIndex(index, blocks, fProofOfStake);
	int ppBlockHeight = GetLastBlockIndex(pBlockHeight - 1, blocks, fProofOfStake); // Point to previous block with -1

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

void getTargets(int startIndex, int arrLen, int blocks[][4]) {

	for (int i = 0, j = startIndex; i < arrLen; i++, j++) {
		cout << blocks[i][0] << " :: " << blocks[i][1] << " :: " << blocks[i][2] << " :: " << blocks[i][3] << "\n";
		getTarget(j, blocks);
		cout << endl;
	}
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

	int blocks1to8[8][4] = {
			{1, 1390009623, 504365055, 0},
			{2, 1390009701, 504365055, 0},
			{3, 1390009747, 504365055, 0},
			{4, 1390009868, 504365055, 0},
			{5, 1390009984, 504365055, 0},
			{6, 1390010147, 504365055, 0},
			{7, 1390010322, 504365055, 0},
			{8, 1390010337, 504365055, 0}
	};

	int blocks88532to88535[4][4] = {
			{88532, 1392635955, 486651228, 0},
			{88533, 1392635968, 486651584, 0},
			{88534, 1392635990, 486651519, 0},
			{88535, 1392636015, 486651243, 1}
	};

	/* Get multiple block targets from a multi-dim array of block data */

	getTargets(1, 8, blocks1to8); // Upto block 9
	getTargets(88532, 4, blocks88532to88535); // 88532 - 88535
}
