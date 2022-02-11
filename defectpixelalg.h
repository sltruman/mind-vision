#ifdef WIN32
#include <Windows.h>
#undef min
#endif

#include <stdint.h>
#include <vector>
#include <algorithm>

struct MvPoint16
{
	MvPoint16() : x(0), y(0) {}
	MvPoint16(uint16_t _x, uint16_t _y) : x(_x), y(_y) {}
	bool operator<(MvPoint16 const& r) const { return pack() < r.pack(); }
	bool operator==(MvPoint16 const& r) const { return pack() == r.pack(); }
	uint32_t pack() const { return (y << 16) + x; }

	uint16_t x;
	uint16_t y;
};

static VOID AnalyzeDefectPixelFromImage(std::vector<MvPoint16>& Points, BYTE *pImage, UCHAR nLevel, UINT width, UINT height)
{
    int nCol,nRow;
	int nLastCol,nLastRow;
	UCHAR *pCurLine,*pPrev2Line,*pNext2Line;
	USHORT ref;
	UCHAR a,b,c,d;
	UCHAR max,min;
    UINT g_nStatisticsHSize;
    UINT g_nStatisticsVSize;
    UINT uiCurImageYMean;

	Points.clear();
    
    int i,j;
    uiCurImageYMean = 0;
    j = width*height;
    i = 0;
    
    while(i++ < j)
    {
        uiCurImageYMean += *(pImage + i);
    }
    uiCurImageYMean /= j;
    uiCurImageYMean = std::min(20u,uiCurImageYMean);

    g_nStatisticsHSize = width;
    g_nStatisticsVSize = height;
    
	nLastCol = g_nStatisticsHSize - 2;
	nLastRow = g_nStatisticsVSize - 2;

    for (nRow=2; nRow<nLastRow; nRow++)
	{
		pCurLine = &pImage[nRow*g_nStatisticsHSize];
		pNext2Line = &pImage[(nRow+2)*g_nStatisticsHSize];
		pPrev2Line = &pImage[(nRow-2)*g_nStatisticsHSize];

        for (nCol=2; nCol<nLastCol; nCol++)
		{
			// 检查周围点
			
			//if (pCurLine[nCol] > 10*nLevel)
			if (pCurLine[nCol] > uiCurImageYMean)
			{
				//m_pDefectStatistics[nCol+(nRow*g_nStatisticsHSize)]++;
				//continue;
				/*
				Ref = pCurLine[nCol-2];

				if (pCurLine[nCol+2] > Ref)
					Ref = pCurLine[nCol+2];

				if (pNext2Line[nCol] > Ref)
					Ref = pNext2Line[nCol];

				if (pPrev2Line[nCol] > Ref)
					Ref = pNext2Line[nCol];
				*/

				a = pCurLine[nCol-2];
				b = pCurLine[nCol+2];
				c = pPrev2Line[nCol];
				d = pNext2Line[nCol];

				max = a;
				if (max < b)
					max = b;
				if (max < c)
					max = c;
				if (max < d)
					max = d;


				min = a;
				if (min > b)
					min = b;
				if (min > c)
					min = c;
				if (min > d)
					min = d;

				//ref = a+b+c+d - max - min;
				ref = (a+b+c+d - max - min)/2;
				if (ref < 10)
				{
					ref = 10;
				}
				/*
				if (nLevel == 0)
				{
					ref = ref << 1; // 2倍
				}
				else if (nLevel == 1)
				{
									// 1
				}	
				else
				{
					ref = (ref>>1) + (ref>>(nLevel-1)); 
				}
				*/

				// 大于一定阀值
				if (pCurLine[nCol] > (nLevel*ref/10))
				{
					Points.push_back(MvPoint16(nCol, nRow));
				}
			}
		}
	}
}

static void AnalyzeBrightPixelFromImage(std::vector<MvPoint16>& Points,
	BYTE *pImage, UCHAR nLevel, UINT width, UINT height)
{
	Points.clear();

	BYTE* p = pImage;
	for (int y = 0; y < height; ++y)
	{
		for (int i = 0; i < 2; ++i)
		{
			BYTE prev = p[i];
			for (int x = i + 2; x < width; x += 2)
			{
				int change = p[x] - prev;
				if (abs(change) > (nLevel + nLevel / 2) )
				{
					Points.push_back(MvPoint16(x, y));
				}
				else
				{
					prev = p[x];
				}
			}
		}

		p += width;
	}
}
