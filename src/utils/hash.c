unsigned int hash_32_fnv1a(char *str) {
    unsigned int hash = 2166136261u;
    while (*str != '\0') {
        hash ^= *str++;
        hash *= 16777619u;
    }
    return hash;
}
