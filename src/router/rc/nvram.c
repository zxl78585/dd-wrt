
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <utils.h>

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#elif HAVE_NVRAM_128
#define NVRAMSPACE 0x20000
#elif HAVE_MVEBU
#define NVRAMSPACE 0x10000
#elif HAVE_WDR4900
#define NVRAMSPACE 0x20000
#elif HAVE_ALPINE
#define NVRAMSPACE 0x20000
#elif HAVE_IPQ806X
#define NVRAMSPACE 0x10000
#else
#define NVRAMSPACE NVRAM_SPACE
#endif

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
#pragma message (VAR_NAME_VALUE(NVRAMSPACE))

#if 0
static int *nvram_hash;

static INLINE uint hash(const char *s, int mul)
{
	uint hash = 0;

	while (*s)
		hash = mul * hash + *s++;

	return hash;
}

void main(int argc, char *argv[])
{
	int i;
	char buf[NVRAMSPACE], *name;
	int mul, max;
	int lowest = 255;
	int lowesti = 0;
	int mod = atoi(argv[1]);
	nvram_hash = malloc(mod * 4);
	for (mul = 0; mul < mod; mul++) {
		memset(buf, 0, NVRAMSPACE);
		memset(nvram_hash, 0, mod * 4);
		nvram_getall(buf, sizeof(buf));
		int len;
		for (name = buf; *name; name += len + 1) {
			len = strlen(name);
			for (i = 0; i < len; i++)
				if (name[i] == '=')
					name[i] = 0;
			nvram_hash[hash(name, mul) % mod]++;
		}

		max = 0;
		for (i = 0; i < mod; i++) {
			if (nvram_hash[i] > max)
				max = nvram_hash[i];
			//fprintf(stderr, "%d ",nvram_hash[i]);
		}
		if (max < lowest) {
			lowest = max;
			lowesti = mul;
		}
		fprintf(stderr, "\n%d = %d\n", mul, max);
	}

	fprintf(stderr, "\n%d = %d\n", lowesti, lowest);
}
#endif

/* 
 * NVRAM utility 
 */
static int nvram_main(int argc, char **argv)
{
	char *name, *value, buf[NVRAMSPACE];
	int size;

	/* 
	 * Skip program name 
	 */
	--argc;
	++argv;

	if (!*argv) {
		fprintf(stderr, "usage: nvram [get name] [set name=value] [unset name] [show] [backup filename] [restore filename]\n");
		exit(0);
	}

	/* 
	 * Process the remaining arguments. 
	 */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		} else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				strncpy(value = buf, *argv, sizeof(buf));
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		} else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		} else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		} else if (!strncmp(*argv, "show", 4)
			   || !strncmp(*argv, "getall", 6)) {
			nvram_getall(buf, sizeof(buf));
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
			size = sizeof(struct nvram_header) + (long)name - (long)buf;
			int space = NVRAMSPACE;
			if (nvram_get("nvram_space"))
				space = nvram_geti("nvram_space");
			fprintf(stderr, "size: %d bytes (%d left)\n", size, space - size);
		} else if (!strncmp(*argv, "backup", 6)) {
			if (*++argv) {
				int ret = nvram_backup(*argv);
				if (ret < 0) {
					fprintf(stderr, "can't write %s\n", *argv);
					return 1;
				}
			}
		} else if (!strncmp(*argv, "restore", 7)) {
			if (*++argv) {
				int ret = nvram_restore(*argv);
				if (ret == -1) {
					fprintf(stderr, "can't write %s\n", *argv);
					return 1;
				}
				if (ret == -2) {
					fprintf(stderr, "file %s broken\n", *argv);
					return 1;
				}
			}
		}
		if (!*argv)
			break;
	}

	return 0;
}
