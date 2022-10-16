
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#include <cpuid.h>
#endif

#include <stdint.h>
#include <stdio.h>

void cpuid(uint32_t result[4], uint32_t eax, uint32_t ecx)
{
#ifdef _WIN32
	__cpuidex(result, eax, ecx);
#else
	__get_cpuid_count(eax, ecx, result, result+1, result+2, result+3);
#endif
}

int main(void)
{
	uint32_t cpuid_result[4] = { 0 };
	cpuid(cpuid_result, 0, 0);

	uint32_t highest_eax = cpuid_result[0];

	printf("highest eax : %u\n", highest_eax);

	char tsc = 0;
	char tsc_deadline = 0;

	if (highest_eax >= 1)
	{
		cpuid(cpuid_result, 1, 0);

		tsc = !!(cpuid_result[3] & 16);

		if (!tsc)
		{
			printf("tsc not supported\n");
			return 0;
		}

		printf("tsc supported\n");
	
		tsc_deadline = !!(cpuid_result[2] & 0x1000000);

		if (tsc_deadline)
			printf("tsc deadline supported\n");
		else
			printf("tsc deadline not supported\n");
	}

	char rdpid = 0;

	if (highest_eax >= 7)
	{
		cpuid(cpuid_result, 7, 0);

		rdpid = !!(cpuid_result[2] & 0x400000);

		if (rdpid)
			printf("Read Processor ID and IA32_TSC_AUX supported\n");
		else
			printf("Read Processor ID and IA32_TSC_AUX not supported\n");
	}


	cpuid(cpuid_result, 0x80000000, 0);

	uint32_t highest_extended_eax = cpuid_result[0];

	printf("highest extended eax : 0x%x\n", highest_extended_eax);

	char rdtscp = 0;
	char perftsc = 0;

	if (highest_extended_eax >= 0x80000001)
	{
		cpuid(cpuid_result, 0x80000001, 0);

		rdtscp = !!(cpuid_result[3] & 0x8000000);
		if (rdtscp)
			printf("rdtscp supported\n");
		else
			printf("rdtscp not supported\n");

		perftsc = !!(cpuid_result[2] & 0x8000000);
		if (perftsc)
			printf("perf tsc supported\n");
		else
			printf("perf tsc not supported\n");
	}

	char invariant_tsc = 0;

	if (highest_extended_eax >= 0x80000007)
	{
		cpuid(cpuid_result, 0x80000007, 0);

		invariant_tsc = !!(cpuid_result[3] & 0x100);
		if (invariant_tsc)
			printf("tsc is invariant\n");
		else
			printf("tsc is not invariant\n");
	}

	return 0;
}