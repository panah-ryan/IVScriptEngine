typedef unsigned char BYTE;
typedef unsigned char byte;
typedef unsigned long DWORD;
typedef int BOOL;
typedef void* PVOID;
typedef DWORD* PDWORD;
typedef unsigned short UINT16, *PUINT16;

#define FALSE 0
#define TRUE 1

struct opd_s
{
	DWORD sub;
	DWORD toc;
};

void PS3MSG(char *msg)
{
	Dialog::msgdialog_mode = Dialog::MODE_STRING_OK;
	Dialog::Show(msg);
}

void PS3MSGLOADING(char *msg, char *smallMSG, int seconds)
{
	Dialog::msgdialog_mode = Dialog::MODE_CHECK_PROGRESSBAR_1;
	Dialog::Showprogress(msg, smallMSG, seconds);
}

int write_process(void* destination, const void* source, size_t size)
{
#ifdef _DEX
	system_call_4(905, (uint64_t)sys_process_getpid(), (uint64_t)destination, size, (uint64_t)source);
#elif _CEX
	system_call_4(201, (uint64_t)sys_process_getpid(), (uint64_t)destination, size, (uint16_t)source);
#else
	system_call_6(8, 0x7777, 0x0032, (uint64_t)sys_process_getpid(), (uint64_t)destination, (uint64_t)((uint32_t)source), (uint64_t)size);
#endif
	return_to_user_prog(int);
}

void* malloc(uint32_t a_uiSize)
{
	sys_addr_t l_uiAddr;
	a_uiSize = ((a_uiSize + 65536) / 65536) * 65536;
	sys_memory_allocate(a_uiSize, SYS_MEMORY_PAGE_SIZE_64K, &l_uiAddr);
	return (void*)l_uiAddr;
}

void free(void* a_Addr)
{
	sys_memory_free((sys_addr_t)a_Addr);
}

void* operator new (std::size_t sz)
{
	void *p;
	if (sz == 0)
		sz = 1;
	p = malloc(sz);
	return p;
}

void operator delete (void *ptr)
{
	free(ptr);
}

void sleep(usecond_t time)
{
	sys_timer_usleep(time * 1000);
}

char* va(char* text, ...)
{
	char buffer[2048];
	va_list VA_ARGS;
	va_start(VA_ARGS, text);
	vsprintf(buffer, text, VA_ARGS);
	return buffer;
}

void sys_ppu_thread_start(sys_ppu_thread_t thread_id)
{
	system_call_1(53, thread_id);
}

void sys_ppu_thread_stop(sys_ppu_thread_t thread_id)
{
	system_call_1(50, thread_id);
}