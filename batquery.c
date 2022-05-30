#include <getopt.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* pad(size_t pad_len)
{
	char* pad_str = (char*) malloc(sizeof(char)*(pad_len+1));
	if (pad_str == NULL)
		return pad_str;
	pad_str[pad_len] = '\0';
	for (int i = 0; i < pad_len; i++)
		pad_str[i] = ' ';
	return pad_str;
}

int usage(const char* prog_name)
{
	size_t pad_len = strlen("usage: ") + strlen(prog_name) + 1;
	char* pad_str = pad(pad_len);
	fprintf(stderr, "usage: %s [-i] [-p | -t] <battery_path>\n", prog_name);
	fprintf(stderr, "%s -i: show battery status icon\n", pad_str);
	fprintf(stderr, "%s -p: show battery percent\n", pad_str);
	fprintf(stderr, "%s -t: show time remaining\n", pad_str);
	free(pad_str);
	exit(EXIT_FAILURE);
}

int error(const char* error_scope, const char* error_msg)
{
	fprintf(stderr, "%s: %s\n", error_scope, error_msg);
	exit(EXIT_FAILURE);
}

int get_battery_percent(const char* battery_path)
{
	char battery_capacity_path[PATH_MAX];
	int n;
	if (battery_path[strlen(battery_path)-1] != '/') {
		n = snprintf(battery_capacity_path, PATH_MAX, "%s/capacity", battery_path);
	} else {
		n = snprintf(battery_capacity_path, PATH_MAX, "%scapacity", battery_path);
	}
	if (n < 0)
		return -1;
	return 0;
}

int main(int argc, char** argv)
{
	bool show_icon = false, show_percent = true, show_time = false;
	int opt;
	while ((opt = getopt(argc, argv, "ipt")) != -1) {
		switch (opt) {
			case 'i':
				show_icon = true;
				break;
			case 'p':
				show_percent = true;
				if (show_time)
					show_time = false;
				break;
			case 't':
				show_time = true;
				if (show_percent)
					show_percent = false;
				break;
			default:
				usage(argv[0]);
		}
	}

	if (optind >= argc)
		error("args", "battery_path not specified");
	const char* battery_path = argv[optind];
	if (battery_path == NULL || strlen(battery_path) == 0)
		error("args", "battery_path specified was empty");

	exit(EXIT_SUCCESS);
}
