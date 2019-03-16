#ifndef __DDUTIL_H__
#define __DDUTIL_H__


#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>

#include <stdarg.h>

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>    // sort


using namespace std;

namespace DD {

#ifdef WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif


#if __cplusplus < 201103L
#error This file requires compiler and library support for the \
ISO C++ 2011 standard. This support is currently experimental, and must be \
enabled with the -std=c++11 or -std=gnu++11 compiler options.
#endif

// http://stackoverflow.com/a/28827188
// cross-platform sleep function
inline static
void sleep_ms(int milliseconds)
{
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}


#ifdef WIN32
    #include <conio.h> //provides non standard getch() function
#else
    #include <unistd.h>   //_getch*/
    #include <termios.h>  //_getch*/

    inline static
    char getch(){           // http://godofjava.tistory.com/14
        struct termios oldt,
        newt;
        int ch;
        tcgetattr( STDIN_FILENO, &oldt );
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newt );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
        return ch;

        /*
        char buf=0;
        struct termios old={0};
        fflush(stdout);
        if(tcgetattr(0, &old)<0)
            perror("tcsetattr()");
        old.c_lflag&=~ICANON;
        old.c_lflag&=~ECHO;
        old.c_cc[VMIN]=1;
        old.c_cc[VTIME]=0;
        if(tcsetattr(0, TCSANOW, &old)<0)
            perror("tcsetattr ICANON");
        if(read(0,&buf,1)<0)
            perror("read()");
        old.c_lflag|=ICANON;
        old.c_lflag|=ECHO;
        if(tcsetattr(0, TCSADRAIN, &old)<0)
            perror ("tcsetattr ~ICANON");
        printf("%c\n",buf);
        return buf;
        */
     }
#endif

////////////////////////////////////////////////////////////////////////////
// String

inline static
bool is_equal_strings(const std::string& a, const std::string& b)
{
	unsigned int sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

#include <sstream>
// http://stackoverflow.com/a/22775236
// example : std::string str1 = to_string(5) ;
template <typename T>
inline static std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}


// https://wiki.kldp.org/HOWTO/html/C++Programming-HOWTO/standard-string.html
inline static
void tokenize(const string& str,
					  std::vector<std::string>& tokens,
					  const string& delimiters)
{
	// 맨 첫 글자가 구분자인 경우 무시
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // 구분자가 아닌 첫 글자를 찾는다
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // token을 찾았으니 vector에 추가한다
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // 구분자를 뛰어넘는다.  "not_of"에 주의하라
        lastPos = str.find_first_not_of(delimiters, pos);
        // 다음 구분자가 아닌 글자를 찾는다
        pos = str.find_first_of(delimiters, lastPos);
    }
}


// http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
// need to compile with c++11 flag
// compile option : http://stackoverflow.com/a/18078167 and https://cmake.org/Wiki/CMake/Tutorials/C%2B%2B11Flags
// g++ -std=c++11
#ifdef _WINDOWS
#pragma warning(disable : 4996)
#endif

#include <string.h>
inline std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::string str;
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while(1) {
	formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
	strcpy(&formatted[0], fmt_str.c_str());
	va_start(ap, fmt_str);
	final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
	va_end(ap);
	if (final_n < 0 || final_n >= n)
		n += abs(final_n - n + 1);
	else
		break;
	}
	return std::string(formatted.get());
}


// Returns a list of files in a directory (except the ones that begin with a dot)
// http://stackoverflow.com/a/1932861

inline void GetFilesInDirectory(std::vector<string> &out, const std::string &directory)
{

	std::cout << directory.c_str() << std::endl;

#ifdef WINDOWS
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */

	do {
		const string file_name = file_data.cFileName;
		const string full_file_name = directory + "/" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		out.push_back(full_file_name);
	} while (FindNextFile(dir, &file_data));

	FindClose(dir);
#else

	DIR *dir;
	class dirent *ent;
	class stat st;

	dir = opendir(directory.c_str());


	while ((ent = readdir(dir)) != NULL) {
		const string file_name = ent->d_name;
		const string full_file_name = directory + "/" + file_name;

		if (file_name[0] == '.')
		    continue;

		if (stat(full_file_name.c_str(), &st) == -1)
		    continue;

		const bool is_directory = (st.st_mode & S_IFDIR) != 0;

		if (is_directory)
		    continue;

		out.push_back(full_file_name);
	}

    closedir(dir);

    std::sort(out.begin(), out.end());
#endif
} // GetFilesInDirectory



inline static
int GetPicturesInDirectory(std::vector<std::string> &out, const string &directory)
{
	std::vector<std::string> fns;
	DD::GetFilesInDirectory(fns, directory);
	std::vector<std::string> fnphoto;
	for(std::vector<string>::iterator it= fns.begin(); it!= fns.end(); ++it)
	{
		vector<string> tokens;
		tokenize(*it, tokens, ".");
		string ext = tokens[tokens.size()-1];

		if(is_equal_strings(ext, "jpg") || is_equal_strings(ext, "png") || is_equal_strings(ext, "bmp") || is_equal_strings(ext, "tiff"))
		{
			;//cout << "found a(n) " << ext << endl;
			fnphoto.push_back(*it);

		}
	}
	out = fnphoto;

    std::sort(out.begin(), out.end());

	return fnphoto.size();
}


inline static
int GetFilesofExtInDirectory(std::vector<std::string> &out, const string &directory, const string &_extension)
{
	std::vector<std::string> fns;
	DD::GetFilesInDirectory(fns, directory);
	std::vector<std::string> fn1;
	for(std::vector<string>::iterator it= fns.begin(); it!= fns.end(); ++it)
	{
		vector<string> tokens;
		tokenize(*it, tokens, ".");
		string ext = tokens[tokens.size()-1];

		if(is_equal_strings(ext, _extension) )
		{
			;//cout << "found a(n) " << ext << endl;
			fn1.push_back(*it);

		}
	}
	out = fn1;

    std::sort(out.begin(), out.end());

	return fn1.size();
}



inline std::string ExtractFilename(string fn)
{

	string::size_type pos = fn.find_last_of("\\/");
	return fn.substr(pos+1, fn.length());
}

inline string EF(string fn)
{
	return ExtractFilename(fn);
}

#ifdef _WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif

inline static std::string getTimeString()
{

    string str1;
    // only for Windows
    #ifdef _WINDOWS
        SYSTEMTIME st;
        GetSystemTime(&st);
        str1 = string_format("%d%02d%02d_%02d%02d%02d_%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    #else
        time_t t = time(0);   // get time now
        struct tm * now = localtime( & t );
        struct timeval ts;
        gettimeofday(&ts,0);
        double tu = ts.tv_usec;

        str1 = string_format("%d%02d%02d_%02d%02d%02d_%.f",    now->tm_year + 1900,
                                                                now->tm_mon + 1,
                                                                now->tm_mday,
                                                                now->tm_hour,
                                                                now->tm_min,
                                                                now->tm_sec,
                                                                tu);

    #endif





    return str1;

}


inline static std::string makeFileNameWithTime(const string ext)
{
    string ret = getTimeString()+ "."+ext;
    return ret;
}



// http://stackoverflow.com/a/9526483
/// @return the nth element in the pair. n must be 0 or 1.
template <class T>
const T& pair_at(const std::pair<T, T>& p, unsigned int n)
{
    //assert(n == 0 || n == 1 && "Pair index must be 0 or 1!");
    // or use -fpermissive flag
    return n == 0 ? p.first : p.second;
}

/// @return the nth element in the pair. n must be 0 or 1.
template <class T>
T& pair_at(std::pair<T, T>& p, unsigned int index)
{
    //assert(index == 0 || index == 1 && "Pair index must be 0 or 1!");
    // or use -fpermissive flag
    return index == 0 ? p.first : p.second;
}



}




template <typename T>
inline static void DbgVectorString( vector<T> &v, int ncnt=-1)
{

    if(ncnt == -1)
    {
        ncnt = v.size();
    }
    typename vector<T>::const_iterator it =  v.begin();
    for(int cnt = 0; it!= v.end(), cnt<ncnt; ++it, ++cnt)
    {

        cout << *it <<endl;

    }

}

template <typename T>
inline static void constvectorDbg(const vector<T> &v, int ncnt=-1)
{
    int cnt = 0;
    if(ncnt == -1)
    {
        ncnt = v.size();
    }
    typename vector<T>::const_iterator it =  v.begin();
    for(; it!= v.end(), cnt<ncnt; ++it)
    {

        cout << "("<< it->x << ", " << it->y <<") "  <<endl;
        cnt++;
    }

}


#endif
