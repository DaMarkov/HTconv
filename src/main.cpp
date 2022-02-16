#include <stdio.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <functional>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "file.h"
#include "blob.h"
#include "png.h"
#include "hts.h"
#include "htc.h"


using namespace std::string_literals;



unsigned int hex2int(const std::string& hexstring)
{
	std::stringstream ss;
	ss << std::hex << hexstring;
	unsigned int value;
	ss >> value;
	return value;
}



std::string int2hex(unsigned int value)
{
	std::stringstream stream;
	stream << std::hex << value;
	return stream.str();
}



void Export(HT* hts, const std::string& Folder)
{
	//Iterate through storage
	auto size = hts->GetStorage().size();
	size_t count = 0;
	for (auto& [key, value] : hts->GetStorage())
	{
		HTS::TexInfo info;
		hts->get(key, info, true);

		float percent = ((count++) * 100.0f) / (float)size;

		if (info.data)
		{
			std::string filename = Folder + "\\#"s + int2hex(key) + "#.png"s;

			printf("[%.01f%%] Texture (%d x %d) -> %s\n", percent, info.width, info.height, filename.c_str());

			if (info.width > 0 && info.height > 0)
				PNG::SavePNG(filename.c_str(), info);
		}

		delete[] info.data;
	}
}



bool Dump(HT* hts)
{
	FILE* dump;
	fopen_s(&dump, "dump.txt", "w");
	if (!dump)
		return false;

	//Iterate through storage
	for (auto& [key, value] : hts->GetStorage())
	{
		HTS::TexInfo info;
		hts->get(key, info, false);

		fprintf(dump, "%d x %d\n", info.width, info.height);

		if (info.data)
		{
			//std::string filename = "export\\test" + std::to_string(key) + ".png";
			//PNG::SavePNG(filename.c_str(), info);

			//auto file = File(filename);
			//Blob data(file);

			//fprintf(dump, filename.c_str());
			//fprintf(dump, " - ");
			fprintf(dump, "Key: %lld\n", key);
		}

		delete[] info.data;
	}

	fclose(dump);
	return true;
}



bool Insert(HT* hts, uint64_t Key, std::string& Filename)
{
	PNG image(Filename);

	if (!image)
		return false;

	hts->Append(Key, image);
	return true;
}



int64_t ToInt(const std::string& Number)
{
	char* pEnd;
	return strtoll(Number.c_str(), &pEnd, 10);	
}



bool IsDirectory(std::string& Filename)
{
#ifdef _WIN32
	WIN32_FIND_DATAA Find;
	HANDLE Handle = FindFirstFileA(Filename.c_str(), &Find);

	bool ret = Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;	

	FindClose(Handle);
	return ret;
#else
	return false;//MIA
#endif
}



bool Indexer(const std::string& FolderName, std::function<bool(const std::string&)> onFile, bool Recursive = true)
{
#ifdef _WIN32
	std::string SearchString = FolderName + "\\*.*";

	WIN32_FIND_DATAA Find;
	HANDLE Handle = FindFirstFileA(SearchString.c_str(), &Find);

	while (FindNextFileA(Handle, &Find))
	{
		if (Find.cFileName[0] != '.')
		{
			if (strlen(Find.cFileName) >= 1 && !(Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string Filename;

				if (FolderName[0] != '.')
					Filename = FolderName + "\\" + Find.cFileName;
				else
					Filename = Find.cFileName;

				if (!onFile(Filename))
				{
					FindClose(Handle);
					return false;
				}
			}

			else if (Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && Recursive)
				Indexer(FolderName + "\\" + Find.cFileName, onFile, Recursive);
		}
	}

	FindClose(Handle);
#else
	DIR* dp;
	dirent* dirp;

	dp = opendir(FolderName.c_str());

	if (!dp) return false;

	while ((dirp = readdir(dp)) != NULL)
	{
		if (dirp->d_name[0] == '.')
			continue;

		if (dirp->d_type == DT_REG)
		{
			std::string Filename;

			if (FolderName[0] != '.')
				Filename = FolderName + "/" + dirp->d_name;
			else
				Filename = dirp->d_name;

			if (!onFile(Filename))
			{
				closedir(dp);
				return false;
			}
		}

		else if (dirp->d_type == DT_DIR && Recru)
			Indexer(FolderName + "/" + dirp->d_name, onFile);
	}

	closedir(dp);
#endif
	return true;
}



void ExecuteCmds(std::vector<std::string> cmds)
{
	HT* HTfile = nullptr;;
	auto size = cmds.size();

	for (size_t i = 0; i < size; i++)
	{
		std::string cmd = cmds[i];

		if (cmd == "-open" && i+1 < size)
		{
			std::string filename = cmds[++i];
			if (!filename.empty())
			{
				if (HTfile)
				{
					delete HTfile;
					HTfile = nullptr;
				}

				printf("Opening %s\n", filename.c_str());

				if (filename[filename.length()-1] == 'c' || filename[filename.length()-1] == 'C')
					HTfile = new HTC(filename);
				else
					HTfile = new HTS(filename);

				if (!HTfile || !*HTfile)
					printf("\nCould not open file!!\n\n");
				else
					printf("File contains %d textures\n", HTfile->GetStorage().size());
			}
		}
		else if (cmd == "-export" && i+1 < size)
		{
			std::string foldername = cmds[++i];
			Export(HTfile, foldername);
		}

		else if (cmd == "-dump")
			Dump(HTfile);

		else if (cmd == "-import" && i+2 < size)
		{
			std::string key   = cmds[++i];
			std::string value = cmds[++i];

			Insert(HTfile, ToInt(key), value);
		}

		else if (cmd == "-save" && i+1 < size)
		{
			std::string filename = cmds[++i];
			if (HTfile)
			{
				HTS exported(*HTfile);
				exported.SaveTo(filename);
			}
		}

		else if (cmd == "-replace" && i+1 < size)
		{
			std::string filename = cmds[++i];

			if (IsDirectory(filename))
			{
				Indexer(filename, [HTfile](auto& file) {
					printf("Replacing %s\n", file.c_str());

					const char* name = file.c_str();
					std::string hex_string;
					bool copy = false;
					for (int j = 0; name[j]; ++j)
					{
						char c = name[j];

						if (c == '#')
						{
							if (hex_string.length() >= 1)
								break;
							copy = true;
						}
						else if (copy)
							hex_string += c;
					}

					auto hash = hex2int(hex_string);

					return HTfile->Replace(hash, file);
				});
			}
			else
			{
				if (i+1 < size)
				{
					std::string key = cmds[++i];
					HTfile->Replace(ToInt(key), filename);
				}
				else
					printf("No Key!!\n");
			}
		}

		else if (cmd == "-insert" && i + 1 < size)
		{
			std::string filename = cmds[++i];

			if (IsDirectory(filename))
			{
			}
			else
			{
				if (i+1 < size)
				{
					std::string key = cmds[++i];
					if (!HTfile->Append(ToInt(key), filename))
						printf("Could not insert!!\n");
				}
				else
					printf("No Key!!\n");
			}
		}
	}
}



void ShowUsage()
{
	printf("HTconv - .htc, .hts and .png file converter\n");
	printf("Programmed by DaMarkov for Open Ocarina Team\n\n");

	printf("htconv <coomand1> <coomand2> <coomand3> ...\n\n");
	printf("Commands:\n");
	printf("-open <filename>\n");
	printf("Opens an .htc or .hts file\n\n");

	printf("-export <directory>\n");
	printf("Exports the .htc/.hts file to a lot of .png files\n\n");

	printf("-save <filename>\n");
	printf("Exports the .htc/.hts file to an .hts file\n\n");

	printf("-replace <directory>\n");
	printf("Takes all #hex#.png files in the directory and put them in the opened file.\n");
	printf("Filename must be the hash in hex form. For example a1b2d3e4.png\n\n");

	printf("-replace <filename> <hash>\n");
	printf("Take a file and puts it in the opened file.\n");
	printf("<hash> should be a 64bit number. For example 4516864.\n\n");
}



int main(int argc, char** argv)
{
	/*HTS hts("cache\\org_THE LEGEND OF ZELDA_HIRESTEXTURES.hts");

	if (!hts)
	{
		printf("File not found!");
		return -1;
	}

	if (!hts.SaveTo("cache\\THE LEGEND OF ZELDA_HIRESTEXTURES.hts"))
		printf("\n\nSaving failed!!!\n\n");*/

	std::vector<std::string> cmds;
	for (int i = 1; i < argc; i++)
		cmds.push_back(argv[i]);

	//cmds.push_back("-open");
	//cmds.push_back("zelda.hts");
	//cmds.push_back("-export");
	//cmds.push_back("export");

	ExecuteCmds(cmds);

	if (cmds.size() <= 1)
		ShowUsage();

	return 0;
}