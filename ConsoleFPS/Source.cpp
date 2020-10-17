#include <iostream>
#include <chrono>

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
		  L"#....##........#"
		  L"#.....#........#"
		  L"#..............#"
		  L"#..............#"
		  L"#..........#...#"
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
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= cosf(fPlayerA) * 5.0 * fElapsedTime;
			fPlayerY -= sinf(fPlayerA) * 5.0 * fElapsedTime;
		}

		for (int x = 0; x < nScreenWidth; ++x)
		{
			// For Each Column, Calculate The Projected View Angle Into World Space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;

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
			else										nShade = ' ';


			for (int y = 0; y < nScreenHeight; ++y)
			{
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = L' ';
				else if(y >= nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
					screen[y * nScreenWidth + x] = L' ';
			}
			
		}
		screen[nScreenWidth * nScreenHeight - 1] = L'\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}