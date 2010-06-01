#ifndef UTILS_H_
#define UTILS_H_

// function returns the amount of virtual memory used by this process in kb
inline uint VmSize()
{
	uint mem = 0;
#if 0
	char fname[40];
	sprintf(fname, "/proc/self/status");
	ifstream fin(fname);
	if (fin.is_open())
	{
		string txt;
		do
		{
			fin >> txt;
		} while (txt != "VmSize:");
		fin >> mem;
		fin.close();
	}
#endif
	return mem;
}

#endif /* UTILS_H_ */
