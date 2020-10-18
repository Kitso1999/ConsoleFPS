#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>

#include <Windows.h>

//Console Dimensions
int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	std::wstring map;

	map = L"################"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"#..............#"
		  L"################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	//Game Loop
	while (true)
	{
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		//Controls
		//Rotation
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= 0.8f * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += 0.8f * fElapsedTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += cosf(fPlayerA) * 5.0 * fElapsedTime;
			fPlayerY += sinf(fPlayerA) * 5.0 * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == L'#')
			{
				fPlayerX -= cosf(fPlayerA) * 5.0 * fElapsedTime;
				fPlayerY -= sinf(fPlayerA) * 5.0 * fElapsedTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= cosf(fPlayerA) * 5.0 * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * 5.0 * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == L'#')
			{
				fPlayerX += cosf(fPlayerA) * 5.0 * fElapsedTime;
				fPlayerY += sinf(fPlayerA) * 5.0 * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; ++x)
		{
			// For Each Column, Calculate The Projected View Angle Into World Space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = cosf(fRayAngle); //Unit Vector For Ray In Player Space
			float fEyeY = sinf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += 0.1f;


				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//If Ray Is Out Of Bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;	//Set Distance To Maximum Depth
				}
				else
				{
					//Ray Is In Bound So Check For Collision
					if (map[nTestY * nMapWidth + nTestX] == L'#')
					{
						bHitWall = true;

						std::vector<std::pair<float, float>> p;	//(Distance, Dot Product)
						for(int tx = 0; tx < 2; ++tx)
							for (int ty = 0; ty < 2; ++ty)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = fEyeX * vx / d + fEyeY * vy / d;
								p.push_back(std::make_pair(d, dot));
							}
							//Sort Pairs From Closest To Farthest
							std::sort(p.begin(), p.end(), [](const std::pair<float, float> &l, const std::pair<float, float> &r) {return l.first < r.first; });

							float fBound = 0.01f;
							if (std::acos(p.at(0).second) < fBound) bBoundary = true;
							if (std::acos(p.at(1).second) < fBound) bBoundary = true;
							
					}
				}
			}
			//Calculate Distance To Ceiling And Floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / (float)fDistanceToWall;
			int nFloor = nScreenHeight - nCeiling;

			unsigned short nShade = ' ';

			if (fDistanceToWall < fDepth / 4.0f)			nShade = 0x2588; //Very Close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = L' ';
			
			if (bBoundary) nShade = L' ';


			for (int y = 0; y < nScreenHeight; ++y)
			{
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = L' ';
				else if(y >= nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
				{
					//Shade floor
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = L'#';
					else if (b < 0.5)	nShade = L'x';
					else if (b < 0.75)	nShade = L'.';
					else if (b < 0.9)	nShade = L'_';
					else				nShade = L' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
			
		}

		//Display Stats
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Display Map
		for(int x = 0; x < nMapWidth; ++x)
			for (int y = 0; y < nMapHeight; ++y)
			{
				screen[(y + 1) * nScreenWidth + x] = map[y * nMapWidth + x];
			}
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)fPlayerX] = L'P';
		screen[nScreenWidth * nScreenHeight - 1] = L'\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}