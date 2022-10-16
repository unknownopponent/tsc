
#ifdef _WIN32
#include <windows.h> // QueryPerformanceFrequency QueryPerformanceCounter Sleep
#include <intrin.h>
#else
#include <x86intrin.h>
#include <time.h> // clock_gettime
#include <unistd.h> // usleep
#include <cpuid.h> // __get_cpuid_count
#endif

#include <stdint.h>
#include <stdio.h>

char sleep_milli(uint64_t milli)
{
#ifdef _WIN32

	Sleep((DWORD)milli);
	return 0;

#else

	return usleep(milli * 1000);

#endif
}

void cpuid_serialize()
{
#ifdef _WIN32

	uint32_t result[4];
	__cpuidex(result, 0, 0);

#else

	uint32_t result[4];
	__get_cpuid_count(0, 0, result, result+1, result+2, result+3);

#endif
}

typedef struct HighResClock
{
#ifdef _WIN32
	
	LARGE_INTEGER frequency;

	LARGE_INTEGER start;
	LARGE_INTEGER end;

#else

	struct timespec start;
	struct timespec end;

#endif
} HighResClock;

char get_high_res_clock_frequency(HighResClock* clock)
{
#ifdef _WIN32

	return !QueryPerformanceFrequency(&clock->frequency);

#else

	return 0;

#endif
}

char get_high_res_clock_start(HighResClock* clock)
{
#ifdef _WIN32

	return !QueryPerformanceCounter(&clock->start);

#else

	return clock_gettime(CLOCK_MONOTONIC, &clock->start);

#endif
}

char get_high_res_clock_end(HighResClock* clock)
{
#ifdef _WIN32

	return !QueryPerformanceCounter(&clock->end);

#else

	return clock_gettime(CLOCK_MONOTONIC, &clock->end);

#endif
}

uint64_t get_high_res_clock_delay_ns(HighResClock* clock)
{
#ifdef _WIN32

	return ((clock->end.QuadPart - clock->start.QuadPart) * 1000000000 ) / clock->frequency.QuadPart;

#else

	struct timespec diff;

	diff.tv_sec = clock->end.tv_sec - clock->start.tv_sec;
	if (clock->end.tv_nsec < clock->start.tv_nsec)
	{
		diff.tv_nsec = 1000000000 + clock->end.tv_nsec - clock->start.tv_nsec;
		diff.tv_sec -= 1;
	}
	else
	{
		diff.tv_nsec = clock->end.tv_nsec - clock->start.tv_nsec;
	}

	return diff.tv_nsec + (diff.tv_sec & 0xffffffff) * 1000000000;

#endif
}

int main(void)
{
	HighResClock clock = { 0 };
	if (get_high_res_clock_frequency(&clock))
	{
		return 1;
	}
	if (get_high_res_clock_start(&clock))
	{
		return 1;
	}

	cpuid_serialize();
	uint64_t tsc1 = __rdtsc();

	sleep_milli(200);

	cpuid_serialize();
	uint64_t tsc2 = __rdtsc();

	if (get_high_res_clock_end(&clock))
	{
		return 1;
	}

	/* may overflow */
	uint64_t frequency = ((tsc2 - tsc1) * 1000000000) / get_high_res_clock_delay_ns(&clock);
	
	printf("%llu Hz\n", frequency);
	printf("%llu MHz\n", frequency / 1000000);

	return 0;
}