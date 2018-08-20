#pragma once
#include "../stdafx.h"
#include "inRange.h"

#define MORPHOLOGICAL_KERNEL_MIN 0
#define MORPHOLOGICAL_KERNEL_MAX 9

array OldYaeAlgorithm(array blackMask) {
	//��鹷�� 1: threshold �մ�
	// array blackMask = inRange(subtitleFrame, 0, 0, 0, 20, 20, 20);
	//��� 2: ��Ѻ��
	array invertedMask = !blackMask;
	array resultMask;

	//��� 3: ����
	array cavityMask = regions(invertedMask.as(b8), AF_CONNECTIVITY_8);
	array cavityMaskBorder = join(0, flat(cavityMask.row(0)), cavityMask.col(0), cavityMask.col(end), flat(cavityMask.row(end)));
	cavityMaskBorder = setUnique(cavityMaskBorder).as(s32);
	int i, borderRegionCount = cavityMaskBorder.dims(0);
	int* borderRegions = cavityMaskBorder.host<int>();
	for (i = 0; i < borderRegionCount; i++) {
		cavityMask -= (cavityMask == borderRegions[i]) * borderRegions[i];
	}
	freeHost(borderRegions);
	//��鹷�� 4: �Ҿ�鹷���˭��Թ���Ҿ
	array largeObjectImage = erode(cavityMask, constant(1, MORPHOLOGICAL_KERNEL_MAX, MORPHOLOGICAL_KERNEL_MAX));
	//��鹷�� 5: ���ѵ�ط���˭��Թ�
	array largeObjectSet = setUnique(flat(largeObjectImage)).as(s32);
	int largeObjectCount = largeObjectSet.dims(0);
	int* largeObjectIds = largeObjectSet.host<int>();
	for (i = 0; i < largeObjectCount; i++) {
		cavityMask -= (cavityMask == largeObjectIds[i]) *largeObjectIds[i];
	}
	freeHost(largeObjectIds);
	if (MORPHOLOGICAL_KERNEL_MIN != 0) {
		//��鹷�� 6: ź��鹷������Թ��͡�ҡ�Ҿ
		array smallMask = constant(1, MORPHOLOGICAL_KERNEL_MIN, MORPHOLOGICAL_KERNEL_MIN);
		array denoisedImage = dilate(erode(cavityMask, smallMask), smallMask);
		//��� 7: �Ӿ�鹷�袹Ҵ����͡�ҡ cavityMask ���� connected component labeling ������������������ҡ
		array denoisedObjectSet = setUnique(flat(denoisedImage)).as(s32);
		resultMask = constant(0, cavityMask.dims(0), cavityMask.dims(1));
		int denoisedObjectCount = denoisedObjectSet.dims(0);
		int* denoiseObjectIds = denoisedObjectSet.host<int>();
		for (i = 0; i < denoisedObjectCount; i++) {
			resultMask += (cavityMask == denoiseObjectIds[i]) * denoiseObjectIds[i];
		}
		freeHost(denoiseObjectIds);
	}
	else {
		resultMask = cavityMask;
	}
	return resultMask > 0;
}