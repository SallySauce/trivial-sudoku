#pragma once

struct EntireFile {
    size_t size;
    char   *data;
};

EntireFile read_entire_file(const char* filename) {

    EntireFile file = {};

    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        fprintf(stderr, "cannot open file %s\n", filename);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    auto length = ftell(fp);
    assert(length != -1);
    rewind(fp);

    file.data = (char *)malloc(length+1);

    if (!file.data) {
        fprintf(stderr, "not enough memory\n");
        exit(1);
    }

    if (fread(file.data, 1, length, fp) == 0) {
        fprintf(stderr, "unable to read file\n");
        exit(1);
    }

    // convenient terminator for string processnig
    file.data[length] = '\0';
    file.size         = length;

    fclose(fp);

    return file;
}

