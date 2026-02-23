void strcpy(char *dst, char *src) {
    while (*src != '\0') {
        *dst++ = *src++;
    }
    *dst = '\0';
}

int strlen(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {}
    return i;
}

int strcmp(char *s1, char *s2) {
    int i;

    if (strlen(s1) != strlen(s2)) {
        return 0;
    }

    for (i = 0; s1[i] != '\0'; i++) {
        if (s1[i] != s2[i]) {
            return 0;
        }
    }

    return 1;
}

void ascii_sum(char *a, char *b, char *result) {
    int len_a = strlen(a) - 1;
    int len_b = strlen(b) - 1;
    int carry = 0;
    int length;
    char ascii_sum[64];
    int i;

    for (i = 0; len_a >= 0 || len_b >= 0; i++, len_a--, len_b--) {
        int da = len_a >= 0 ? a[len_a] - '0' : 0;
        int db = len_b >= 0 ? b[len_b] - '0' : 0;

        int sum = da + db + carry;
        carry = sum / 10;

        ascii_sum[i] = (sum % 10) + '0';
    }

    if (carry != 0) {
        ascii_sum[i++] = carry + '0';
    }
    ascii_sum[i] = '\0';

    length = i;
    for (i = 0; i < length; i++) {
        result[i] = ascii_sum[length - i - 1];
    }
    result[i] = '\0';
}

void ascii_sub(char *a, char *b, char *result)
{
    int len_a = strlen(a) - 1;
    int len_b = strlen(b) - 1;
    int borrow = 0;
    int length;
    char ascii_sub[64];
    int i, j;

    for (i = 0; len_a >= 0 || len_b >= 0; i++, len_a--, len_b--)
    {
        int da = len_a >= 0 ? a[len_a] - '0' : 0;
        int db = len_b >= 0 ? b[len_b] - '0' : 0;
        int diff = da - db - borrow;

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        ascii_sub[i] = diff + '0';
    }

    ascii_sub[i] = '\0';

    // Remove leading zeros in reversed result
    while (i > 1 && ascii_sub[i - 1] == '0') {
        i--;
    }

    // Reverse into final result
    length = i;
    for (j = 0; j < length; j++) {
        result[j] = ascii_sub[length - j - 1];
    }
    result[length] = '\0';
}

int ascii_lower(char *str1, char *str2) {
    return strlen(str1) <= strlen(str2) && strcmp(str1, str2) < 0;
}

int ascii_higher(char *str1, char *str2) {
    return strlen(str1) >= strlen(str2) && strcmp(str1, str2) > 0;
}

int is_in_range(char *range_start, char *range_end, char *value) {
    int is_higher = ascii_higher(value, range_start);
    int is_lower = ascii_lower(value, range_end);
    return is_higher == 1 && is_lower == 1;
}

void itoa(int num, char *dst) {
    int index = 0;
    int start = 0;
    int i, j;

    if (num == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return;
    }

    if (num < 0) {
        dst[index++] = '-';
        num = -num;
        start = 1;
    }

    while (num != 0) {
        dst[index++] = (num % 10) + '0';
        num /= 10;
    }

    dst[index] = '\0';

    for (i = start, j = index - 1; i < j; i++, j--) {
        char tmp = dst[i];
        dst[i] = dst[j];
        dst[j] = tmp;
    }
}

int array_contains_int(int *array, int len, int value) {
    int i;
    for (i = 0; i < len; i++) {
        if (array[i] == value) {
            return 1;
        }
    }
    return 0;
}

void load_ingredients_data(int fd, char *ranges_start, char *ranges_end, char *ingredient_ids, int *ranges_len, int *ingredients_len) {
    char buffer[32], number[32];
    int i, input_part_ranges = 0, number_idx = 0;
    int bytes_read = 0;

    do {
        bytes_read = read(fd, buffer, 32);
        for (i = 0; i < bytes_read; i++) {
            if (buffer[i] >= '0' && buffer[i] <= '9') {
                number[number_idx++] = buffer[i];
            } else if (buffer[i] == '-') {
                number[number_idx] = '\0';
                number_idx = 0;
                strcpy(&ranges_start[*ranges_len * 32], number);
            } else if (buffer[i] == '\n') {
                number[number_idx] = '\0';
                number_idx = 0;
                if (strlen(number) == 0) {
                    input_part_ranges = 1;
                } else {
                    if (input_part_ranges == 0) {
                        strcpy(&ranges_end[*ranges_len * 32], number);
                        *ranges_len = *ranges_len + 1;
                    } else {
                        strcpy(&ingredient_ids[*ingredients_len * 32], number);
                        *ingredients_len = *ingredients_len + 1;
                    }
                }
            }
        }
    } while (bytes_read > 0);
}

void sort_ranges(char *ranges_start, char *ranges_end, int ranges_len) {
    int i, j;
    for (i = 0; i < ranges_len; i++) {
        for (j = 0; j < ranges_len - i - 1; j++) {
            if (ascii_lower(&ranges_start[(j + 1) * 32], &ranges_start[j * 32]) == 1) {
                char tmp[32];

                strcpy(tmp, &ranges_start[(j + 1) * 32]);
                strcpy(&ranges_start[(j + 1) * 32], &ranges_start[j * 32]);
                strcpy(&ranges_start[j * 32], tmp);

                strcpy(tmp, &ranges_end[(j + 1) * 32]);
                strcpy(&ranges_end[(j + 1) * 32], &ranges_end[j * 32]);
                strcpy(&ranges_end[j * 32], tmp);
            }
        }
    }
}

void reformat_ingredients_part2(char *ranges_start, char *ranges_end, int ranges_len, char *ranges_start_pt2, char *ranges_end_pt2, int *ranges_len_pt2) {
    int ignored_list[1000];
    int ignored_list_len = 0;

    int i, j;
    for (i = 0; i < ranges_len; i++) {
        char *org_r_s, *org_r_e;
        if (array_contains_int(ignored_list, ignored_list_len, i) == 1) {
            continue;
        }

        org_r_s = &ranges_start[i * 32];
        org_r_e = &ranges_end[i * 32];

        for (j = i + 1; j < ranges_len; j++) {
            char *trg_r_s, *trg_r_e;
            int is_original_in_range, is_target_in_range;

            if (array_contains_int(ignored_list, ignored_list_len, j) == 1) {
                continue;
            }

            trg_r_s = &ranges_start[j * 32];
            trg_r_e = &ranges_end[j * 32];

            is_original_in_range = is_in_range(trg_r_s, trg_r_e, org_r_s) == 1 || is_in_range(trg_r_s, trg_r_e, org_r_e) == 1;
            is_target_in_range = is_in_range(org_r_s, org_r_e, trg_r_s) == 1 || is_in_range(org_r_s, org_r_e, trg_r_e) == 1;

            if (is_original_in_range == 1 || is_target_in_range == 1) {
                ignored_list[ignored_list_len++] = j;
                if (ascii_lower(trg_r_s, org_r_s) == 1) {
                    org_r_s = trg_r_s;
                }
                if (ascii_higher(trg_r_e, org_r_e) == 1) {
                    org_r_e = trg_r_e;
                }
            }
        }

        strcpy(&ranges_start_pt2[*ranges_len_pt2 * 32], org_r_s);
        strcpy(&ranges_end_pt2[*ranges_len_pt2 * 32], org_r_e);
        *ranges_len_pt2 = *ranges_len_pt2 + 1;
    }
}

int is_fresh_ingredient(char *ranges_start, char *ranges_end, int ranges_len, char *ingredient_id) {
    int i;
    for (i = 0; i < ranges_len; i++) {
        char *range_start_addr = &ranges_start[i * 32];
        char *range_end_addr = &ranges_end[i * 32];

        if (strlen(ingredient_id) < strlen(range_start_addr) || strlen(ingredient_id) > strlen(range_end_addr)) {
            continue;
        }

        if (strcmp(ingredient_id, range_start_addr) >= 0 && strcmp(ingredient_id, range_end_addr) <= 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    char buffer[32], ingredients[32768], ranges_start[10240], ranges_end[10240];
    int ingredients_len = 0, ranges_len = 0;

    int result_part1 = 0;
    char result_part2[64] = {'0', '\0'};

    char ranges_start_part2[10240], ranges_end_part2[10240];
    int ranges_len_part2 = 0;

    int fd, i;

    fd = open("day5.txt", O_RDONLY, 00700);
    if (fd < 0) {
        write(STDOUT_FILENO, "Error opening file\n", 20);
        return 1;
    }

    load_ingredients_data(fd, ranges_start, ranges_end, ingredients, &ranges_len, &ingredients_len);
    for (i = 0; i < ingredients_len; i++) {
        char *input_addr = &ingredients[i * 32];
        if (is_fresh_ingredient(ranges_start, ranges_end, ranges_len, input_addr)) {
            result_part1++;
        }
    }

    sort_ranges(ranges_start, ranges_end, ranges_len);
    reformat_ingredients_part2(ranges_start, ranges_end, ranges_len, ranges_start_part2, ranges_end_part2, &ranges_len_part2);

    for (i = 0; i < ranges_len_part2; i++) {
        char *range_start_addr = &ranges_start_part2[i * 32];
        char *range_end_addr = &ranges_end_part2[i * 32];
        char result[32];
        ascii_sub(range_end_addr, range_start_addr, result);
        ascii_sum(result_part2, result, result_part2);
        ascii_sum(result_part2, "1", result_part2);
    }

    itoa(result_part1, buffer);
    write(STDOUT_FILENO, "Puzzle result (part 1): ", 24);
    write(STDOUT_FILENO, buffer, strlen(buffer));
    write(STDOUT_FILENO, "\n", 1);

    write(STDOUT_FILENO, "Puzzle result (part 2): ", 24);
    write(STDOUT_FILENO, result_part2, strlen(result_part2));
    write(STDOUT_FILENO, "\n", 1);

    close(fd);
    return ranges_len;
}
